#include "engine/env/lights/Spotlight.h"
#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/Frustum.h"
#include "engine/rendering/GLUtils.h"
#include "LightProcessor.h"
#include <gl/glew.h>

#define SHADOWMAP_ATLAS_RESOLUTION 4096
#define HIGHPRIO_SHADOWMAP_SIZE 2048
#define MEDIUMPRIO_SHADOWMAP_SIZE 1024
#define LOWPRIO_SHADOWMAP_SIZE 512

void LightProcessor::prioritizeLights(const std::vector<Light*>& lights, RawBuffer<Light*>& outputBuffer, const std::array<unsigned int, LightPriority::Count>& maxShadowMapsPerPriority, const RenderContext& context) {
    struct ScoredLight { Light* valPtr; float score; };
    
    const Frustum cameraFrustum(context.viewProjection);

    // Calculate scores for lights
    glm::vec3 cameraPosition = context.viewportTransform.getPosition();
    std::vector<ScoredLight> scoredLights;
    for (Light* light : lights) {
        if (!cameraFrustum.isSphereInside(light->getGlobalTransform().getPosition(), light->getRange())) {
            // Skip lights outside the cameras view frustum. TODO: isSphereInside() method might be inaccurate for directional lights
            continue;
        }
        float distance = glm::distance(cameraPosition, light->getGlobalTransform().getPosition());
        float score = (1.0f / (distance + 1.0f)) * light->getRange();
        scoredLights.push_back({ light, score });
    }

    // Sort lights by score in descending order
    std::sort(scoredLights.begin(), scoredLights.end(), [](const ScoredLight& a, const ScoredLight& b) {
        return a.score > b.score;
    });

    // Assign priorities and shadow atlas indices
    std::array<unsigned int, LightPriority::Count> currentShadowMapCounts = {0, 0, 0};

    outputBuffer.clear();
    for (const auto& sLight : scoredLights) {
        for (int prio = LightPriority::High; prio <= LightPriority::Low; prio++) {
            if (currentShadowMapCounts[prio] < maxShadowMapsPerPriority[prio]) {
                sLight.valPtr->setPriotity(static_cast<LightPriority>(prio));
                sLight.valPtr->setShadowAtlasIndex(currentShadowMapCounts[prio]++);
                outputBuffer.push_back(sLight.valPtr);
                break;
            }
        }
    }
}

void LightProcessor::initialize() {
    if (!isInitialized) {
        // Create buffers for shadowmapping
        m_shadowMapAtlases[LightPriority::High] = std::make_unique<FrameBuffer>();
        m_shadowMapAtlases[LightPriority::High]->attachTexture(std::make_shared<Texture>(TextureType::Depth, SHADOWMAP_ATLAS_RESOLUTION, SHADOWMAP_ATLAS_RESOLUTION, 1, nullptr, TextureFilter::Nearest, TextureWrap::ClampToEdge));
        m_shadowMapSizes[LightPriority::High] = HIGHPRIO_SHADOWMAP_SIZE;
        m_shadowMapAtlases[LightPriority::Medium] = std::make_unique<FrameBuffer>();
        m_shadowMapAtlases[LightPriority::Medium]->attachTexture(std::make_shared<Texture>(TextureType::Depth, SHADOWMAP_ATLAS_RESOLUTION, SHADOWMAP_ATLAS_RESOLUTION, 1, nullptr, TextureFilter::Nearest, TextureWrap::ClampToEdge));
        m_shadowMapSizes[LightPriority::Medium] = MEDIUMPRIO_SHADOWMAP_SIZE;
        m_shadowMapAtlases[LightPriority::Low] = std::make_unique<FrameBuffer>();
        m_shadowMapAtlases[LightPriority::Low]->attachTexture(std::make_shared<Texture>(TextureType::Depth, SHADOWMAP_ATLAS_RESOLUTION, SHADOWMAP_ATLAS_RESOLUTION, 1, nullptr, TextureFilter::Nearest, TextureWrap::ClampToEdge));
        m_shadowMapSizes[LightPriority::Low] = LOWPRIO_SHADOWMAP_SIZE;

        m_totalSupportedLights = 0;
        for (int i = 0; i < LightPriority::Count; i++) {
            m_maxShadowMapsPerPriority[i] = m_shadowMapAtlases[i]->getAttachedDepthTexture()->width() / m_shadowMapSizes[i];
            m_maxShadowMapsPerPriority[i] *= m_maxShadowMapsPerPriority[i];
            m_totalSupportedLights += m_maxShadowMapsPerPriority[i];
        }

        m_lightViewProjectionBuffer = RawBuffer<glm::mat4>(m_totalSupportedLights);
        m_lightUniformBuffer = std::make_shared<UniformBuffer>(nullptr, m_totalSupportedLights * sizeof(ShaderLightStruct));
        
        m_lightBuffer = RawBuffer<ShaderLightStruct>(m_totalSupportedLights);
        m_lightViewProjectionUniformBuffer = std::make_shared<UniformBuffer>(nullptr, m_totalSupportedLights * sizeof(glm::mat4));

        isInitialized = true;
    }
}

void LightProcessor::clearShadowMaps() {
    for (int i = 0; i < LightPriority::Count; i++) {
		m_shadowMapAtlases[i]->bind();
		GLCALL(glClear(GL_DEPTH_BUFFER_BIT));
	}
}

void LightProcessor::prepareShadowPass(const Light* light) {
    if (isInitialized) {
        LightPriority prio = light->getPriotity();
        int atlasBufferSize = m_shadowMapAtlases[prio]->getAttachedDepthTexture()->width();
        int tileSize = m_shadowMapSizes[prio];
        int atlasIndex = light->getShadowAtlasIndex();

        int tilesPerRow = atlasBufferSize / tileSize;
        int x = (atlasIndex % tilesPerRow) * tileSize;
        int y = (atlasIndex / tilesPerRow) * tileSize;

        m_shadowMapAtlases[prio]->bind();
        GLCALL(glViewport(x, y, tileSize, tileSize));
    }
}

void LightProcessor::updateBuffers(const RawBuffer<Light*>& activeLights) {
	if (isInitialized) {
        m_lightBuffer.clear();
        m_lightViewProjectionBuffer.clear();

        for (Light* currLight : activeLights) {
            Transform lTr = currLight->getGlobalTransform();

            ShaderLightStruct lightStruct;
            lightStruct.lightType = static_cast<unsigned int>(currLight->getType());
            lightStruct.priority = static_cast<unsigned int>(currLight->getPriotity());
            lightStruct.shadowMapIndex = static_cast<unsigned int>(currLight->getShadowAtlasIndex());
            lightStruct.lightPosition = lTr.getPosition();
            lightStruct.direction = lTr.getForward();
            lightStruct.color = currLight->getColor();
            lightStruct.intensity = currLight->getIntensity();
            lightStruct.range = currLight->getRange();
            if (Spotlight* lSpot = dynamic_cast<Spotlight*>(currLight)) {
                lightStruct.fovy = lSpot->getFovy();
                lightStruct.innerCutoffAngle = lSpot->getInnerCutoffAngle();
            }

            m_lightBuffer.push_back(lightStruct);
            m_lightViewProjectionBuffer.push_back(currLight->getViewProjMatrix());
        }

        m_lightUniformBuffer->updateData(m_lightBuffer.data(), activeLights.size() * sizeof(ShaderLightStruct));
        m_lightViewProjectionUniformBuffer->updateData(m_lightViewProjectionBuffer.data(), activeLights.size() * sizeof(glm::mat4));
    }
}

std::array<std::weak_ptr<Texture>, LightPriority::Count> LightProcessor::getShadowMapAtlases() const {
    std::array<std::weak_ptr<Texture>, LightPriority::Count> shadowMapsTextures;
    if (isInitialized) {
        for (int i = 0; i < LightPriority::Count; i++) {
            shadowMapsTextures[i] = std::weak_ptr<Texture>(m_shadowMapAtlases[i]->getAttachedDepthTexture());
        }    
    }
    return shadowMapsTextures;
}