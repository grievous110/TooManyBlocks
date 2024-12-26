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

void Renderer::beginShadowpass(const Scene& scene) {

}

void Renderer::endShadowpass(const Scene& scene) {

}

void Renderer::beginMainpass(const Scene& scene) {
	
}

void Renderer::endMainpass(const Scene& scene) {

}

Renderer::Renderer() {
	ASSERT(sizeof(float) == sizeof(GLfloat), "Size mismatch for GLfloat.");
	ASSERT(sizeof(double) == sizeof(GLdouble), "Size mismatch for GLdouble.");
	ASSERT(sizeof(int) == sizeof(GLint), "Size mismatch for GLint.");
	ASSERT(sizeof(unsigned int) == sizeof(GLuint), "Size mismatch for GLuint.");
	ASSERT(sizeof(short) == sizeof(GLshort), "Size mismatch for GLshort.");
	ASSERT(sizeof(unsigned short) == sizeof(GLushort), "Size mismatch for GLushort.");
	ASSERT(sizeof(char) == sizeof(GLbyte), "Size mismatch for GLbyte.");
	ASSERT(sizeof(unsigned char) == sizeof(GLubyte), "Size mismatch for GLubyte.");
}

void Renderer::renderScene(const Scene& scene, const ApplicationContext& context) {
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	beginShadowpass(scene);

	for (const std::shared_ptr<Mesh> mesh : scene.meshes) {
		const std::shared_ptr<Material> material = mesh->getMaterial();
		if (material->supportsPass(PassType::ShadowPass)) {
			material->bindForPass(PassType::ShadowPass, currentRenderContext);
			drawMesh(*mesh, currentRenderContext);
			material->unbindForPass(PassType::ShadowPass);
		}
	}

	endShadowpass(scene);

	const Transform& playerTr = context.instance->m_player->getTransform();
	Transform& camTr = context.instance->m_player->m_camera->getTransform();

	glm::vec3 position = playerTr.getPosition();
	glm::vec3 forward = camTr.getForward();
	glm::vec3 up = camTr.getUp();

	glm::mat4 view = glm::lookAt(position, position + forward, up);
	glm::mat4 proj = context.instance->m_player->m_camera->getProjectionMatrix();
	currentRenderContext.viewProjection = proj * view;

	beginMainpass(scene);

	for (const std::shared_ptr<Mesh> mesh : scene.meshes) {
		const Transform meshTr = mesh->getGlobalTransform();
		currentRenderContext.modelMatrix = meshTr.getModelMatrix();
		currentRenderContext.meshPosition = meshTr.getPosition();

		const std::shared_ptr<Material> material = mesh->getMaterial();
		if (material->supportsPass(PassType::MainPass)) {
			material->bindForPass(PassType::MainPass, currentRenderContext);
			drawMesh(*mesh, currentRenderContext);
			material->unbindForPass(PassType::MainPass);
		}
	}

	endMainpass(scene);
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
   	auto it = m_textureCacher.find(texturePath);
	if (it != m_textureCacher.end()) {
		return it->second;
	}

	// Use the provided creator function to create the new object
	std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(texturePath);
	m_textureCacher[texturePath] = newTexture;
	return newTexture;
}