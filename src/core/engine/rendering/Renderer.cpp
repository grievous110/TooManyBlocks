#include "engine/rendering/Renderer.h"
#include "engine/GameInstance.h"
#include "engine/rendering/Camera.h"
#include "Logger.h"
#include <memory>
#include <sstream>
#include <vector>

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

	for (const Mesh* mesh : scene.m_meshes) {
		const std::shared_ptr<Material> material = mesh->m_material;
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
	currentRenderContext.modelViewProjection = proj * view * glm::mat4(1.0f);

	beginMainpass(scene);

	for (const Mesh* mesh : scene.m_meshes) {
		const std::shared_ptr<Material> material = mesh->m_material;
		if (material->supportsPass(PassType::MainPass)) {
			material->bindForPass(PassType::MainPass, currentRenderContext);
			drawMesh(*mesh, currentRenderContext);
			material->unbindForPass(PassType::MainPass);
		}
	}

	endMainpass(scene);
}

void Renderer::drawMesh(const Mesh& mesh, const RenderContext& context) {
	mesh.m_material->m_shader->bind();
	mesh.m_vao->bind();
	mesh.m_ibo->bind();

	GLCALL(glDrawElements(GL_TRIANGLES, mesh.m_ibo->count(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const {
	shader.bind();
	va.bind();
	ib.bind();

	// For use with IBOs
	GLCALL(glDrawElements(GL_TRIANGLES, ib.count(), GL_UNSIGNED_INT, nullptr));

	// For use with solely VBOs
	// glDrawArrays(GL_TRIANGLES, 0, 3 * 2);
}

std::shared_ptr<Shader> Renderer::getShader(const std::string& shader) {
	return m_shaderCache.getShader(shader);
}