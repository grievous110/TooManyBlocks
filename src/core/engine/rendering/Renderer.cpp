#include "engine/rendering/Renderer.h"
#include "engine/GameInstance.h"
#include "engine/rendering/Camera.h"
#include "Logger.h"
#include <memory>
#include <sstream>
#include <vector>
#include "Renderer.h"

void GLClearError() {
	const int maxChecks = 10;  // Limit number of error checks
    int errorCount = 0;

    while (glGetError() != GL_NO_ERROR) {
        errorCount++;
        if (errorCount >= maxChecks) {
            lgr::lout.error("GLClearError() reached maximum error checks. Possible infinite error generation.");
            break;
        }
    }
}

bool GLLogCall(const char* functionName, const char* file, int line) {
	const int maxChecks = 10;  // Limit number of error checks
    int errorCount = 0;
	std::vector<GLenum> errors;
	while (GLenum error = glGetError()) {
		errors.push_back(error);
		errorCount++;
		if (errorCount >= maxChecks) {
            lgr::lout.error("GLClearError() reached maximum error checks. Possible infinite error generation.");
            break;
        }
	}
	if (!errors.empty()) {
		for (const GLenum& e : errors) {
			std::stringstream stream;
			stream << "[OpenGL Error 0x" << std::hex << e <<
				"] caused by " << functionName <<
				" in " << file <<
				" line " << line;
			lgr::lout.error(stream.str());
		}
		return false;
	}
	return true;
}

void Renderer::beginShadowpass(const Scene& scene, const ApplicationContext& context) {

}

void Renderer::endShadowpass(const Scene& scene, const ApplicationContext& context) {

}

void Renderer::beginMainpass(const Scene& scene, const ApplicationContext& context) {
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	currentRenderContext.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
	currentRenderContext.cameraTransform = context.instance->m_player->getCamera()->getGlobalTransform();
}

void Renderer::endMainpass(const Scene& scene, const ApplicationContext& context) {

}

void Renderer::renderScene(const Scene& scene, const ApplicationContext& context) {
	beginShadowpass(scene, context);

	for (const std::shared_ptr<Light> light : scene.lights) {
		GLCALL(glClear(GL_DEPTH_BUFFER_BIT));
		
		for (const std::shared_ptr<Mesh> mesh : scene.meshes) {
			const std::shared_ptr<Material> material = mesh->getMaterial();
			if (material->supportsPass(PassType::ShadowPass)) {
				material->bindForPass(PassType::ShadowPass, currentRenderContext);
				drawMesh(*mesh, currentRenderContext);
				material->unbindForPass(PassType::ShadowPass);
			}
		}
	}

	endShadowpass(scene, context);

	beginMainpass(scene, context);

	for (const std::shared_ptr<Mesh> mesh : scene.meshes) {
		currentRenderContext.meshTransform = mesh->getGlobalTransform();

		const std::shared_ptr<Material> material = mesh->getMaterial();
		if (material->supportsPass(PassType::MainPass)) {
			material->bindForPass(PassType::MainPass, currentRenderContext);
			drawMesh(*mesh, currentRenderContext);
			material->unbindForPass(PassType::MainPass);
		}
	}

	endMainpass(scene, context);
}

void Renderer::drawMesh(const Mesh& mesh, const RenderContext& context) {
	const std::shared_ptr<MeshRenderData> rData = mesh.renderData();
	mesh.getMaterial()->getShader()->bind();
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