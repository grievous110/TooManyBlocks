#include "Application.h"
#include "engine/GameInstance.h"
#include "engine/rendering/Renderer.h"
#include "engine/rendering/mat/ChunkMaterial.h"
#include "engine/rendering/mat/SimpleMaterial.h"
#include "engine/rendering/MeshCreate.h"
#include "engine/rendering/ShaderPathsConstants.h"
#include "rendering/Mesh.h"
#include "engine/controllers/PlayerController.h"
#include "engine/env/lights/Spotlight.h"
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

	std::uniform_real_distribution<float> positionDist(-50.0f, 50.0f);
    std::uniform_real_distribution<float> lookAtDist(-50.0f, 50.0f);
	for (int i = 0; i < 100; ++i) {
        auto light = std::make_shared<Spotlight>(glm::vec3(1.0f), 1.0f, 45.0f, 50.0f);
        light->setInnerCutoffAngle(25.0f);

        // Randomize position
        glm::vec3 randomPosition(positionDist(generator), 10.0f, positionDist(generator));
        light->getLocalTransform().setPosition(randomPosition);

        // Randomize lookAt target
        glm::vec3 randomTarget(lookAtDist(generator), 5.0f, lookAtDist(generator));
        light->getLocalTransform().lookAt(randomTarget);

        // Add to the vector
        m_lights.push_back(light);
    }

	m_mesh = renderer->getMeshFromFile("res/models/testUnitBlock.obj");
	shared_ptr<Shader> shader = renderer->getShaderFromFile(SIMPLE_SHADER);
	shared_ptr<Texture> texture = renderer->getTextureFromFile("res/textures/testTexture.png");	
	std::shared_ptr<Material> m_testMaterial = make_shared<SimpleMaterial>(shader, glm::vec3(0.0f), texture);
	m_mesh->assignMaterial(m_testMaterial);
	m_mesh->getLocalTransform().setPosition(glm::vec3(0.0f, 10.0f, 0.0f));

	// Capture and hide the mouse cursor
	glfwSetInputMode(Application::getContext()->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	isInitialized = true;
}

Scene GameInstance::craftScene() {
	Scene scene;
	for (const auto& light : m_lights) {
        scene.lights.push_back(light);
    }

	for (const auto& val : m_world->loadedChunks()) {
		if (val.second && val.second->mesh) {
			scene.meshes.push_back(val.second->mesh);
		}
	}
	scene.meshes.push_back(m_mesh);
	return scene;
}

void GameInstance::update(float msDelta) {
	m_player->update(msDelta);
	for (const auto& light : m_lights) {
        light->getLocalTransform().rotate(glm::vec3(0.0f, 10.0f * (msDelta / 1000.0f), 0.0f));
    }
	
	m_world->updateChunks(m_player->getTransform().getPosition(), 3);
}