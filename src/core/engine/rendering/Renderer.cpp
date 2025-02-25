#include "Application.h"
#include "engine/GameInstance.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/SceneOptimizing.h"
#include "engine/rendering/ShaderPathsConstants.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "Logger.h"
#include "Renderer.h"
#include <chrono>
#include <gl/glew.h>
#include <memory>
#include <sstream>
#include <vector>

#define SHADOWMAP_ATLAS_RESOLUTION 4096
#define HIGHPRIO_SHADOWMAP_SIZE 2048
#define MEDIUMPRIO_SHADOWMAP_SIZE 1024
#define LOWPRIO_SHADOWMAP_SIZE 512

static constexpr float fullScreenQuadCCW[] = {
	// Position   // UV-Koordinaten
	-1.0f,  1.0f,  0.0f, 1.0f,  // Oben-Links
	-1.0f, -1.0f,  0.0f, 0.0f,  // Unten-Links
	 1.0f, -1.0f,  1.0f, 0.0f,  // Unten-Rechts

	-1.0f,  1.0f,  0.0f, 1.0f,  // Oben-Links
	 1.0f, -1.0f,  1.0f, 0.0f,  // Unten-Rechts
	 1.0f,  1.0f,  1.0f, 1.0f   // Oben-Rechts
};

static constexpr float fullScreenQuadCW[] = {
	// Position   // UV-Koordinaten
	 1.0f, -1.0f,  1.0f, 0.0f,  // Unten-Rechts
	-1.0f, -1.0f,  0.0f, 0.0f,  // Unten-Links
	-1.0f,  1.0f,  0.0f, 1.0f,  // Oben-Links

	 1.0f, -1.0f,  1.0f, 0.0f,  // Unten-Rechts
	-1.0f,  1.0f,  0.0f, 1.0f,  // Oben-Links
	 1.0f,  1.0f,  1.0f, 1.0f   // Oben-Rechts
};

static inline void batchByMaterialForPass(std::unordered_map<Material*, std::vector<Mesh*>>& materialBatches, const RawBuffer<Mesh*>& meshBuff, PassType type) {
	materialBatches.clear();
	for (Mesh* mesh : meshBuff) {
		if (mesh->getMaterial()->supportsPass(type)) {
			materialBatches[mesh->getMaterial().get()].push_back(mesh);
		}
	}
}

static inline void setAtlasViewport(int tileSize, int index, int atlasBufferSize) {
	int tilesPerRow = atlasBufferSize / tileSize;
    int x = (index % tilesPerRow) * tileSize;
    int y = (index / tilesPerRow) * tileSize;

    GLCALL(glViewport(x, y, tileSize, tileSize));
}

void Renderer::beginShadowpass(const Scene &scene, const ApplicationContext& context) {
    for (int i = 0; i < LightPriority::Count; i++) {
		m_currentRenderContext.shadowMapAtlases[i]->bind();
		GLCALL(glClear(GL_DEPTH_BUFFER_BIT));
	}

	m_currentRenderContext.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
	m_currentRenderContext.viewportTransform = context.instance->m_player->getCamera()->getGlobalTransform();
	prioritizeLights(scene.lights, m_currentRenderContext.lights, m_maxShadowMapsPerPriority, m_currentRenderContext);
	
	int activeLightCount = m_currentRenderContext.lights.size();
	
	lightBuffer.clear();
	lightViewProjectionBuffer.clear();
	for (int i = 0; i < activeLightCount; i++) {
		Light* currLight = m_currentRenderContext.lights[i];
		Transform lTr = currLight->getGlobalTransform();
		lightBuffer[i].lightType = static_cast<unsigned int>(currLight->getType());
		lightBuffer[i].priority = static_cast<unsigned int>(currLight->getPriotity());
		lightBuffer[i].shadowMapIndex = static_cast<unsigned int>(currLight->getShadowAtlasIndex());
		lightBuffer[i].lightPosition = lTr.getPosition();
		lightBuffer[i].direction = lTr.getForward();
		lightBuffer[i].color = currLight->getColor();
		lightBuffer[i].intensity = currLight->getIntensity();
		lightBuffer[i].range = currLight->getRange();
		if (Spotlight* lSpot = dynamic_cast<Spotlight*>(currLight)) {
			lightBuffer[i].fovy = lSpot->getFovy();
			lightBuffer[i].innerCutoffAngle = lSpot->getInnerCutoffAngle();
		}

		lightViewProjectionBuffer[i] = currLight->getViewProjMatrix();
	}
	m_currentRenderContext.lightBuff->updateData(lightBuffer.data(), activeLightCount * sizeof(ShaderLightStruct));
	m_currentRenderContext.lightViewProjectionBuff->updateData(lightViewProjectionBuffer.data(), activeLightCount * sizeof(glm::mat4));
}

void Renderer::endShadowpass(const Scene& scene, const ApplicationContext& context) {

}

void Renderer::beginAmbientOcclusionPass(const Scene& scene, const ApplicationContext& context) {
	m_currentRenderContext.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
	m_currentRenderContext.projection = context.instance->m_player->getCamera()->getProjectionMatrix();
	m_currentRenderContext.view = context.instance->m_player->getCamera()->getViewMatrix();
	m_currentRenderContext.viewportTransform = context.instance->m_player->getCamera()->getGlobalTransform();
	// Disable blending cause rendering to textures that do not have 4 channels (alpha) will will be discarded. Blending expects a valid alpha component
	GLCALL(glDisable(GL_BLEND));
}

void Renderer::endAmbientOcclusionPass(const Scene &scene, const ApplicationContext &context) {
	m_currentRenderContext.ssaoOutput = m_ssaoProcessor.getOcclusionOutput();
	GLCALL(glEnable(GL_BLEND));
}

void Renderer::beginMainpass(const Scene &scene, const ApplicationContext &context) {
    FrameBuffer::bindDefault();
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	glm::uvec2 screenRes = m_currentRenderContext.currentScreenResolution;
	GLCALL(glViewport(0, 0, screenRes.x, screenRes.y));
	m_currentRenderContext.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
	m_currentRenderContext.projection = context.instance->m_player->getCamera()->getProjectionMatrix();
	m_currentRenderContext.view = context.instance->m_player->getCamera()->getViewMatrix();
	m_currentRenderContext.viewportTransform = context.instance->m_player->getCamera()->getGlobalTransform();
}

void Renderer::endMainpass(const Scene& scene, const ApplicationContext& context) {

}

void Renderer::initialize() {
	//GLCALL(glPolygonMode(GL_FRONT, GL_LINE)); // Grid View mode
	GLCALL(glEnable(GL_BLEND));
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glEnable(GL_CULL_FACE));         // Enable face culling
	GLCALL(glCullFace(GL_BACK));            // Specify that back faces should be culled (not rendered)
	GLCALL(glFrontFace(GL_CW));				// Specify frontfaces as faces with clockwise winding
	GLCALL(glEnable(GL_MULTISAMPLE));		// Enable MSAA
	GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));	// Blending
	GLCALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

	// Create buffers for shadowmapping
	m_currentRenderContext.shadowMapAtlases[LightPriority::High] = std::make_shared<FrameBuffer>();
	m_currentRenderContext.shadowMapAtlases[LightPriority::High]->attachTexture(std::make_shared<Texture>(TextureType::Depth, SHADOWMAP_ATLAS_RESOLUTION, SHADOWMAP_ATLAS_RESOLUTION));
	m_currentRenderContext.shadowMapSizes[LightPriority::High] = HIGHPRIO_SHADOWMAP_SIZE;
	m_currentRenderContext.shadowMapAtlases[LightPriority::Medium] = std::make_shared<FrameBuffer>();
	m_currentRenderContext.shadowMapAtlases[LightPriority::Medium]->attachTexture(std::make_shared<Texture>(TextureType::Depth, SHADOWMAP_ATLAS_RESOLUTION, SHADOWMAP_ATLAS_RESOLUTION));
	m_currentRenderContext.shadowMapSizes[LightPriority::Medium] = MEDIUMPRIO_SHADOWMAP_SIZE;
	m_currentRenderContext.shadowMapAtlases[LightPriority::Low] = std::make_shared<FrameBuffer>();
	m_currentRenderContext.shadowMapAtlases[LightPriority::Low]->attachTexture(std::make_shared<Texture>(TextureType::Depth, SHADOWMAP_ATLAS_RESOLUTION, SHADOWMAP_ATLAS_RESOLUTION));
	m_currentRenderContext.shadowMapSizes[LightPriority::Low] = LOWPRIO_SHADOWMAP_SIZE;

	m_totalSupportedLights = 0;
    for (int i = 0; i < LightPriority::Count; i++) {
        m_maxShadowMapsPerPriority[i] = m_currentRenderContext.shadowMapAtlases[i]->getAttachedDepthTexture()->width() / m_currentRenderContext.shadowMapSizes[i];
        m_maxShadowMapsPerPriority[i] *= m_maxShadowMapsPerPriority[i];
		m_totalSupportedLights += m_maxShadowMapsPerPriority[i];
    }
	m_currentRenderContext.lights = RawBuffer<Light*>(m_totalSupportedLights);
	m_currentRenderContext.lightBuff = std::make_shared<UniformBuffer>(nullptr, m_totalSupportedLights * sizeof(ShaderLightStruct));
	m_currentRenderContext.lightViewProjectionBuff = std::make_shared<UniformBuffer>(nullptr, m_totalSupportedLights * sizeof(glm::mat4));
	lightBuffer = RawBuffer<ShaderLightStruct>(m_totalSupportedLights);
	lightViewProjectionBuffer = RawBuffer<glm::mat4>(m_totalSupportedLights);
	
	// Create vertex array / buffer for fullscreen quad
	m_fullScreenQuad_vbo = std::make_unique<VertexBuffer>(fullScreenQuadCW, sizeof(fullScreenQuadCW));
	m_fullScreenQuad_vao = std::make_unique<VertexArray>();
	VertexBufferLayout layout;
	layout.push(GL_FLOAT, sizeof(float), 2); // Position
	layout.push(GL_FLOAT, sizeof(float), 2); // Screen UV
	m_fullScreenQuad_vao->addBuffer(*m_fullScreenQuad_vbo, layout);

	// Initialize SSAO renderer
	m_ssaoProcessor.initialize();

	FrameBuffer::bindDefault();
}

void Renderer::renderScene(const Scene &scene, const ApplicationContext &context) {
	static auto lastLogTime = std::chrono::high_resolution_clock::now();
    static std::chrono::duration<double> totalTime(0);
    static std::chrono::duration<double> testTime(0);
	static int frameCount = 0;

	auto totalTimerStart = std::chrono::high_resolution_clock::now();

	// Update screen resolution
	m_currentRenderContext.currentScreenResolution = glm::uvec2(context.screenWidth, context.screenHeight);

    beginShadowpass(scene, context);

	RawBuffer<Mesh*> culledMeshBuffer = RawBuffer<Mesh*>(scene.meshes.size());
	std::unordered_map<Material*, std::vector<Mesh*>> materialBatches;
	for (int i = 0; i < std::min<int>(m_currentRenderContext.lights.size(), m_totalSupportedLights); i++) {
		const Light* light = m_currentRenderContext.lights[i];
		m_currentRenderContext.currentLightPrio = light->getPriotity();
		m_currentRenderContext.lightShadowAtlasIndex = light->getShadowAtlasIndex();
		m_currentRenderContext.viewProjection = light->getViewProjMatrix();
		m_currentRenderContext.viewportTransform = light->getGlobalTransform();
		m_currentRenderContext.shadowMapAtlases[m_currentRenderContext.currentLightPrio]->bind();
		setAtlasViewport(
			m_currentRenderContext.shadowMapSizes[m_currentRenderContext.currentLightPrio],
			m_currentRenderContext.lightShadowAtlasIndex,
			m_currentRenderContext.shadowMapAtlases[m_currentRenderContext.currentLightPrio]->getAttachedDepthTexture()->width()
		);

		cullMeshesOutOfView(scene.meshes, culledMeshBuffer, m_currentRenderContext.viewProjection);
		batchByMaterialForPass(materialBatches, culledMeshBuffer, PassType::ShadowPass);

		for (const auto& batch : materialBatches) {
			batch.first->bindForPass(PassType::ShadowPass, m_currentRenderContext);
			
			for (const Mesh* mesh : batch.second) {
				m_currentRenderContext.meshTransform = mesh->getGlobalTransform();
				batch.first->bindForMeshDraw(PassType::ShadowPass, m_currentRenderContext);
				drawMesh(*mesh);
			}
		}
	}
	
	endShadowpass(scene, context);
	
	auto testTimerStart = std::chrono::high_resolution_clock::now();
	beginAmbientOcclusionPass(scene, context);
	
	cullMeshesOutOfView(scene.meshes, culledMeshBuffer, m_currentRenderContext.viewProjection);
	batchByMaterialForPass(materialBatches, culledMeshBuffer, PassType::AmbientOcclusion);
	
	if (!materialBatches.empty()) {		
		m_ssaoProcessor.prepareSSAOGBufferPass(context);
		
		for (const auto& batch : materialBatches) {
			batch.first->bindForPass(PassType::AmbientOcclusion, m_currentRenderContext);
			
			for (const Mesh* mesh : batch.second) {
				m_currentRenderContext.meshTransform = mesh->getGlobalTransform();
				batch.first->bindForMeshDraw(PassType::AmbientOcclusion, m_currentRenderContext);
				drawMesh(*mesh);
			}
		}
		
		m_ssaoProcessor.prepareSSAOPass(context);
		drawFullscreenQuad();
		
		m_ssaoProcessor.prepareSSAOBlurPass(context);
		drawFullscreenQuad();
	}
	
	endAmbientOcclusionPass(scene, context);
	testTime += std::chrono::high_resolution_clock::now() - testTimerStart;
	
	beginMainpass(scene, context);
	
	// No culling since main pass uses same view
	//cullMeshesOutOfView(scene.meshes, culledMeshBuffer, m_currentRenderContext.viewProjection);
	batchByMaterialForPass(materialBatches, culledMeshBuffer, PassType::MainPass);
	
	for (const auto& batch : materialBatches) {
		batch.first->bindForPass(PassType::MainPass, m_currentRenderContext);
		
		for (const Mesh* mesh : batch.second) {
			m_currentRenderContext.meshTransform = mesh->getGlobalTransform();
			batch.first->bindForMeshDraw(PassType::MainPass, m_currentRenderContext);
			drawMesh(*mesh);
		}
	}
	
	endMainpass(scene, context);
	totalTime += std::chrono::high_resolution_clock::now() - totalTimerStart;

    // Logging
	frameCount++;
    auto now = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastLogTime).count() >= 1) {
        std::stringstream msg;
		msg << "Time: " << testTime.count() * 1000.0 / frameCount << "ms (average per frame)" << std::endl;
		msg << "Tested part took " << testTime.count() / totalTime.count() * 100.0 << "% of " << totalTime.count() * 1000.0 / frameCount << "ms excution time" << std::endl;
        msg << "Lights processed: " << m_currentRenderContext.lights.size() << std::endl;
		msg << "Batches: " << materialBatches.size();
		lgr::lout.debug(msg.str());

        // Reset timers and counters
		totalTime = std::chrono::duration<double>(0);
        testTime = std::chrono::duration<double>(0);
        frameCount = 0;
        lastLogTime = now;
    }
}

void Renderer::drawMesh(const Mesh& mesh) {
	const std::shared_ptr<MeshRenderData> rData = mesh.renderData();
	rData->vao.bind();
	rData->ibo.bind();

	GLCALL(glDrawElements(GL_TRIANGLES, rData->ibo.count(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::drawFullscreenQuad() {
	m_fullScreenQuad_vao->bind();
	GLCALL(glDrawArrays(GL_TRIANGLES, 0, 6));
}

std::shared_ptr<Shader> Renderer::getShaderFromFile(const std::string& shaderPath) {
	auto it = m_shaderCache.find(shaderPath);
	if (it != m_shaderCache.end()) {
		return it->second;
	}

	// Use the provided creator function to create the new object
	std::shared_ptr<Shader> newShader = std::make_shared<Shader>(shaderPath);
	m_shaderCache[shaderPath] = newShader;
	return newShader;
}

std::shared_ptr<Texture> Renderer::getTextureFromFile(const std::string& texturePath) {
   	auto it = m_textureCache.find(texturePath);
	if (it != m_textureCache.end()) {
		return it->second;
	}

	// Use the provided creator function to create the new object
	std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(texturePath);
	m_textureCache[texturePath] = newTexture;
	return newTexture;
}

std::shared_ptr<Mesh> Renderer::getMeshFromFile(const std::string &meshPath) {
    auto it = m_meshCache.find(meshPath);
	if (it != m_meshCache.end()) {
		return std::make_shared<Mesh>(it->second.first, it->second.second);
	}

	// Use the provided creator function to create the new object
	std::shared_ptr<RawMeshData> meshData = readMeshDataFromObjFile(meshPath, true);
	std::shared_ptr<MeshRenderData> renderData = packToRenderData(*meshData);
	m_meshCache[meshPath] = std::make_pair(renderData, meshData->bounds);
	return std::make_shared<Mesh>(renderData, meshData->bounds);
}
