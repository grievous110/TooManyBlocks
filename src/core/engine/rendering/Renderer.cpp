#include "engine/GameInstance.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/SceneOptimizing.h"
#include "Logger.h"
#include "Renderer.h"
#include <chrono>
#include <GLFW/glfw3.h>
#include <memory>
#include <sstream>
#include <vector>

#define SHADOWMAP_ATLAS_RESOLUTION 4096
#define HIGHPRIO_SHADOWMAP_SIZE 2048
#define MEDIUMPRIO_SHADOWMAP_SIZE 1024
#define LOWPRIO_SHADOWMAP_SIZE 512

void Renderer::beginShadowpass(const Scene& scene, const ApplicationContext& context) {
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

void Renderer::beginSSAOpass(const Scene& scene, const ApplicationContext& context) {
	m_currentRenderContext.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
	m_currentRenderContext.viewportTransform = context.instance->m_player->getCamera()->getGlobalTransform();

	FrameBuffer::bindDefault();
	int display_w, display_h;
	glfwGetFramebufferSize(context.window, &display_w, &display_h);
	std::shared_ptr<Texture> screenDBuff = m_currentRenderContext.screenDepthBuffer->getAttachedTexture();
	if (screenDBuff->width() != display_w || screenDBuff->height() != display_h) {
		m_currentRenderContext.screenDepthBuffer->attachTexture(std::make_shared<Texture>(TextureType::Depth, display_w, display_h));
	}
	m_currentRenderContext.screenDepthBuffer->bind();
	GLCALL(glClear(GL_DEPTH_BUFFER_BIT));

	// Random sampling kernel
	for (int i = 0; i < 64; ++i) {
		glm::vec3 sample(
			(float)rand() / RAND_MAX * 2.0f - 1.0f,
			(float)rand() / RAND_MAX * 2.0f - 1.0f,
			(float)rand() / RAND_MAX
		);
		sample = glm::normalize(sample);
		sample *= (float)rand() / RAND_MAX;
		float scale = (float)i / 64.0f;
		scale = glm::mix(0.1f, 1.0f, scale * scale); // Focus samples closer to the fragment
		sample *= scale;
		m_currentRenderContext.ssaoKernel[i] = sample;
	}
}

void Renderer::endSSAOpass(const Scene& scene, const ApplicationContext& context) {

}

void Renderer::beginMainpass(const Scene& scene, const ApplicationContext& context) {
	FrameBuffer::bindDefault();
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	int display_w, display_h;
	glfwGetFramebufferSize(context.window, &display_w, &display_h);
	GLCALL(glViewport(0, 0, display_w, display_h));
	m_currentRenderContext.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
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

	// Query meta info
	GLCALL(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_metaInfo.maxVertexAttribs));
	GLCALL(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_metaInfo.maxTextureImageUnits));
	GLCALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_metaInfo.maxCombinedTextureImageUnits));
	GLCALL(glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &m_metaInfo.maxUniformBlockSize));
	GLCALL(glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &m_metaInfo.maxVertUniformBlocks));
	GLCALL(glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &m_metaInfo.maxFragUniformBlocks));
	GLCALL(glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &m_metaInfo.maxVertUniformComponents));
	GLCALL(glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS , &m_metaInfo.maxFragUniformComponents));
	GLCALL(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_metaInfo.maxTextureSize));
	GLCALL(glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &m_metaInfo.max3DTextureSize));
	GLCALL(glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &m_metaInfo.maxArrayTextureLayers));
	GLCALL(glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &m_metaInfo.maxFramebufferWidth));
	GLCALL(glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &m_metaInfo.maxFramebufferHeight));
	GLCALL(glGetIntegerv(GL_MAX_VARYING_VECTORS, &m_metaInfo.maxVaryingVectors));
	GLCALL(glGetIntegerv(GL_MAX_SAMPLES, &m_metaInfo.maxMSAASamples));
	GLCALL(glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &m_metaInfo.maxSSBOBindings));
	GLCALL(glGetIntegerv(GL_MAX_DRAW_BUFFERS, &m_metaInfo.maxDrawBuffers));
	GLCALL(glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &m_metaInfo.maxElementsIndices));
	GLCALL(glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &m_metaInfo.maxElementsVertices));

	std::stringstream details;
	details << "OpenGL API MetaInfo:" << std::endl;
	details << "Max vertex attributes: " << m_metaInfo.maxVertexAttribs << std::endl;
	details << "Max texture image units: " << m_metaInfo.maxTextureImageUnits << std::endl;
	details << "Max combined texture image units: " << m_metaInfo.maxCombinedTextureImageUnits << std::endl;
	details << "Max uniform block size: " << m_metaInfo.maxUniformBlockSize << " bytes" << std::endl;
	details << "Max vertex uniform blocks: " << m_metaInfo.maxVertUniformBlocks << std::endl;
	details << "Max fragment uniform blocks: " << m_metaInfo.maxFragUniformBlocks << std::endl;
	details << "Max vertex uniform components: " << m_metaInfo.maxVertUniformComponents << std::endl;
	details << "Max fragment uniform components: " << m_metaInfo.maxFragUniformComponents << std::endl;
	details << "Max texture size: " << m_metaInfo.maxTextureSize << "x" << m_metaInfo.maxTextureSize << std::endl;
	details << "Max 3D texture size: " << m_metaInfo.max3DTextureSize << "x" << m_metaInfo.max3DTextureSize << "x" << m_metaInfo.max3DTextureSize << std::endl;
	details << "Max array texture layers: " << m_metaInfo.maxArrayTextureLayers << std::endl;
	details << "Max framebuffer size: " << m_metaInfo.maxFramebufferWidth << "x" << m_metaInfo.maxFramebufferHeight << std::endl;
	details << "Max varying vectors: " << m_metaInfo.maxVaryingVectors << std::endl;
	details << "Max MSAA samples: " << m_metaInfo.maxMSAASamples << std::endl;
	details << "Max SSBO bindings: " << m_metaInfo.maxSSBOBindings << std::endl;
	details << "Max draw buffers: " << m_metaInfo.maxDrawBuffers << std::endl;
	details << "Max elements indices: " << m_metaInfo.maxElementsIndices << std::endl;
	details << "Max elements vertices: " << m_metaInfo.maxElementsVertices << std::endl;
	lgr::lout.info(details.str());

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

	FrameBuffer::bindDefault();
	int display_w, display_h;
	glfwGetFramebufferSize(Application::getContext()->window, &display_w, &display_h);
	m_currentRenderContext.screenDepthBuffer = std::make_shared<FrameBuffer>(std::make_shared<Texture>(TextureType::Depth, display_w, display_h));

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
}

static void setAtlasViewport(int tileSize, int index, int atlasBufferSize) {
	int tilesPerRow = atlasBufferSize / tileSize;
    int x = (index % tilesPerRow) * tileSize;
    int y = (index / tilesPerRow) * tileSize;

    GLCALL(glViewport(x, y, tileSize, tileSize));
}

void Renderer::renderScene(const Scene &scene, const ApplicationContext &context) {
	static auto lastLogTime = std::chrono::high_resolution_clock::now();
    static std::chrono::duration<double> totalTime(0);
    static std::chrono::duration<double> testTime(0);
	static int frameCount = 0;

	auto totalTimerStart = std::chrono::high_resolution_clock::now();
    beginShadowpass(scene, context);

	RawBuffer<Mesh*> culledMeshBuffer = RawBuffer<Mesh*>(scene.meshes.size());
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
		std::unordered_map<Material*, std::vector<Mesh*>> materialBatches;
		for (Mesh* mesh : culledMeshBuffer) {
			if (mesh->getMaterial()->supportsPass(PassType::ShadowPass)) {
				materialBatches[mesh->getMaterial().get()].push_back(mesh);
			}
		}

		auto testTimerStart = std::chrono::high_resolution_clock::now();
		for (const auto& batch : materialBatches) {
			batch.first->bindForPass(PassType::ShadowPass, m_currentRenderContext);
			
			for (const Mesh* mesh : batch.second) {
				m_currentRenderContext.meshTransform = mesh->getGlobalTransform();
				batch.first->bindForMeshDraw(PassType::ShadowPass, m_currentRenderContext);
				drawMesh(*mesh);
			}
		}
		testTime += std::chrono::high_resolution_clock::now() - testTimerStart;
	}

	endShadowpass(scene, context);


	std::unordered_map<Material*, std::vector<Mesh*>> materialBatches;
	{
		cullMeshesOutOfView(scene.meshes, culledMeshBuffer, m_currentRenderContext.viewProjection);
		for (Mesh* mesh : culledMeshBuffer) {
			if (mesh->getMaterial()->supportsPass(PassType::MainPass)) {
				materialBatches[mesh->getMaterial().get()].push_back(mesh);
			}
		}
		
		beginSSAOpass(scene, context);
		
		for (const auto& batch : materialBatches) {
			batch.first->bindForPass(PassType::ScreenSpaceAmbientOcclusion, m_currentRenderContext);

			for (const Mesh* mesh : batch.second) {
				m_currentRenderContext.meshTransform = mesh->getGlobalTransform();
				batch.first->bindForMeshDraw(PassType::ScreenSpaceAmbientOcclusion, m_currentRenderContext);
				drawMesh(*mesh);
			}
		}

		endSSAOpass(scene, context);
		
		beginMainpass(scene, context);
		
		for (const auto& batch : materialBatches) {
			batch.first->bindForPass(PassType::MainPass, m_currentRenderContext);

			for (const Mesh* mesh : batch.second) {
				m_currentRenderContext.meshTransform = mesh->getGlobalTransform();
				batch.first->bindForMeshDraw(PassType::MainPass, m_currentRenderContext);
				drawMesh(*mesh);
			}
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
