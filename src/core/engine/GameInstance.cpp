#include "Application.h"
#include "engine/GameInstance.h"
#include "engine/rendering/Renderer.h"
#include "engine/rendering/mat/ChunkMaterial.h"
#include "engine/rendering/mat/SimpleMaterial.h"
#include "engine/rendering/MeshCreate.h"
#include "engine/rendering/ShaderPathsConstants.h"
#include "rendering/Mesh.h"
#include "engine/controllers/PlayerController.h"
#include "Logger.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <random>
#include <iostream>
#include <vector>

using namespace std;

GameInstance::GameInstance() 
	: m_playerController(nullptr), m_player(nullptr), m_world(nullptr), m_mesh(nullptr), isInitialized(false) {}

GameInstance::~GameInstance() {
	if (m_player)
		delete m_player;
	if (m_world)
		delete m_world;
}

void GameInstance::initialize() {
	// Random seed
	std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<uint32_t> distribution(0, UINT32_MAX);
	uint32_t seed = distribution(generator);
	m_playerController = new PlayerController;
	m_player = new Player;
	m_world = new World(seed);
	m_playerController->possess(m_player);

	Renderer* renderer = Application::getContext()->renderer;

	m_mesh = buildFromMeshData(*readMeshDataFromObjFile("res/models/testUnitBlock.obj", true)); 
	shared_ptr<Shader> shader = renderer->getShaderFromFile(SIMPLE_SHADER);
	shared_ptr<Texture> texture = renderer->getTextureFromFile("res/textures/testTexture.png");	
	std::shared_ptr<Material> m_testMaterial = make_shared<SimpleMaterial>(shader, glm::vec3(0.0f), texture);
	m_mesh->assignMaterial(m_testMaterial);
	m_mesh->getLocalTransform().setPosition(glm::vec3(0.0f, 10.0f, 0.0f));

	// Capture and hide the mouse cursor
	glfwSetInputMode(Application::getContext()->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	isInitialized = true;
}

// static bool isBoxInFrustum(const glm::vec3& position, const glm::vec3& size, const glm::vec4* planes) {
// 	for (int i = 0; i < 6; i++) {
// 		const glm::vec4& plane = planes[i];
// 		glm::vec3 corner = position;

// 		// Choose the farthest corner along the plane's normal
// 		if (plane.x >= 0) corner.x += size.x;
// 		if (plane.y >= 0) corner.y += size.y;
// 		if (plane.z >= 0) corner.z += size.z;

// 		// Check if the corner is outside the plane
// 		if (plane.x * corner.x + plane.y * corner.y + plane.z * corner.z + plane.w < 0) {
// 			return false; // Completely outside the frustum
// 		}
// 	}
// 	return true; // Inside or intersecting the frustum
// }

Scene GameInstance::craftScene() {
	Scene scene;

	// glm::vec4 planes[6];

	// int count = 0;

	for (const auto& val : m_world->loadedChunks()) {
		if (val.second && val.second->mesh) {
			// glm::mat4 viewProjMatrix = m_player->getCamera()->getViewProjMatrix();
			// viewProjMatrix *= m_player->getTransform().getModelMatrix();
			// planes[0] = viewProjMatrix[3] + viewProjMatrix[0]; // Left plane
			// planes[1] = viewProjMatrix[3] - viewProjMatrix[0]; // Right plane
			// planes[2] = viewProjMatrix[3] + viewProjMatrix[1]; // Bottom plane
			// planes[3] = viewProjMatrix[3] - viewProjMatrix[1]; // Top plane
			// planes[4] = viewProjMatrix[3] + viewProjMatrix[2]; // Near plane
			// planes[5] = viewProjMatrix[3] - viewProjMatrix[2]; // Far plane

			// // Normalize planes
			// for (int i = 0; i < 6; i++) {
			// 	float length = glm::length(glm::vec3(planes[i]));
			// 	planes[i] /= length;
			// }

			// glm::vec3 chunkPosition = val.second->mesh->getLocalTransform().getPosition();

			// if (isBoxInFrustum(chunkPosition, {CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH}, planes)) {
            //     scene.meshes.push_back(val.second->mesh);
			// 	count++;
            // }

			scene.meshes.push_back(val.second->mesh);
		}
	}
	//lgr::lout.debug(std::to_string(count) + " meshes" + std::string("Rendering "));
	scene.meshes.push_back(m_mesh);
	return scene;
}

void GameInstance::update(float msDelta) {
	m_player->update(msDelta);
	m_mesh->getLocalTransform().rotate(glm::vec3(0.0f, 90.0f * (msDelta / 1000.0f), 0.0f));
	m_world->updateChunks(m_player->getTransform().getPosition(), 3);
}

	// ######## Constructor #########
	//float* vertices = new float[data.vertices.size()];
	//unsigned int* indices = new unsigned int[data.indices.size()];
	//
	//for (int i = 0; i < data.vertices.size(); i++) {
	//	vertices[i] = data.vertices[i];
	//}

	//for (int i = 0; i < data.indices.size(); i++) {
	//	indices[i] = data.indices[i];
	//}

	//// Vertex Array Object (VAO)
	//m_vao = new VertexArray();

	//// Vertex Buffer Object (VBO)
	//m_vbo = new VertexBuffer(vertices, static_cast<int>(data.vertices.size() * sizeof(float)));

	//// Vertex Attribute Pointer 
	//VertexBufferLayout layout;
	//layout.push<float>(3);
	//layout.push<float>(2);
	//layout.push<float>(3);
	//m_vao->addBuffer(*m_vbo, layout);

	//// Index Buffer Object (IBO)
	//m_ibo = new IndexBuffer(indices, static_cast<int>(data.indices.size()));

	//m_shader->bind();

	//m_shadowMapFBO = new FrameBuffer(4096, 4096);
	//
	//unsigned int tex_slot = 0;
	//m_texture->bind(tex_slot);
	//m_shader->setUniform1i("u_texture", tex_slot);

	//m_vao->unbind();
	//m_vbo->unbind();
	//m_ibo->unbind();
	//m_shader->unbind();

//void GameInstance::render(Renderer& renderer) {
	//static float bias = 0.0005f;

	//m_shadowMapFBO->bind();
	//GLCALL(glViewport(0, 0, m_shadowMapFBO->getDepthTexture()->width(), m_shadowMapFBO->getDepthTexture()->height()));
	//renderer.clear();
	//// Use the depth shader program
	//m_depthShader->bind();

	//// Set up light view and projection matrices
	////glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 50.0f);
	//glm::mat4 lightProjection = glm::perspective(90.0f, 1.0f, 0.1f, 100.0f);
	//glm::mat4 lightView = glm::lookAt(m_lightPosition, m_lightPosition + glm::vec3(1.0f, -1.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	//m_depthShader->setUniformMat4f("u_lightSpaceMatrix", lightSpaceMatrix);
	//
	//// Render your scene with the depth shader
	//renderer.draw(*m_vao, *m_ibo, *m_depthShader);
	//
	//m_shadowMapFBO->unbind();

	//renderer.clear();
	//int width, height;
	//GLCALL(glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height));
	//GLCALL(glViewport(0, 0, width, height));

	//m_texture->bind(0);
	//m_shadowMapFBO->getDepthTexture()->bind(1);
	//m_shader->bind();
	//
	//{
	//	glm::mat4 model = glm::mat4(1.0f);// * z_rot * y_rot * x_rot;
	//	glm::mat4 mvp = m_camera.getViewProjMatrix() * model;

	//	m_shader->setUniform1f("bias", bias);
	//	m_shader->setUniform1i("u_texture", 0);
	//	m_shader->setUniform1i("u_shadowMap", 1);
	//	m_shader->setUniform3f("u_lightPos", m_lightPosition.x, m_lightPosition.y, m_lightPosition.z);
	//	m_shader->setUniformMat4f("u_mvp", mvp);
	//	m_shader->setUniformMat4f("u_model", model);
	//	m_shader->setUniformMat4f("u_lightSpaceMatrix", lightSpaceMatrix);
	//	renderer.draw(*m_vao, *m_ibo, *m_shader);
	//}
//}