#include "GameInstance.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <random>
#include <vector>

#include "AppConstants.h"
#include "Application.h"
#include "Logger.h"
#include "engine/controllers/PlayerController.h"
#include "engine/env/lights/Spotlight.h"
#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/MeshCreate.h"
#include "engine/rendering/Renderer.h"
#include "engine/rendering/mat/ChunkMaterial.h"
#include "engine/rendering/mat/LineMaterial.h"
#include "engine/rendering/mat/SimpleMaterial.h"
#include "providers/Provider.h"
#include "rendering/StaticMesh.h"
#include "threading/ThreadPool.h"

GameInstance::GameInstance() : m_playerController(nullptr), m_player(nullptr), m_world(nullptr) {}

GameInstance::~GameInstance() { deinitWorld(); }

void GameInstance::initializeWorld(World* newWorld) {
    if (m_world == nullptr) {
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<uint32_t> distribution(0, UINT32_MAX);
        m_playerController = new PlayerController;
        m_player = new Player;
        m_world = newWorld;
        m_playerController->possess(m_player);

        Provider* provider = Application::getContext()->provider;

        std::uniform_real_distribution<float> positionDist(-50.0f, 50.0f);
        std::uniform_real_distribution<float> lookAtDist(-50.0f, 50.0f);
        for (int i = 0; i < 25; ++i) {
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

        std::shared_ptr<Shader> shader = provider->getShaderFromFile(Res::Shader::SIMPLE);
        std::shared_ptr<Texture> texture = provider->getTextureFromFile(Res::Texture::TESTBLOCK_TEXTURE);
        std::shared_ptr<Material> testMaterial1 = std::make_shared<SimpleMaterial>(shader, glm::vec3(0.0f), texture);
        std::shared_ptr<Material> testMaterial2 = std::make_shared<SimpleMaterial>(shader, glm::vec3(1, 0.5f, 0));
        m_mesh1 = provider->getMeshFromFile(Res::Model::TEST_UNIT_BLOCK);
        m_mesh1->assignMaterial(testMaterial1);
        m_mesh2 = provider->getMeshFromFile(Res::Model::TEST_UNIT_BLOCK);
        m_mesh2->assignMaterial(testMaterial2);

        m_mesh1->getLocalTransform().setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        m_mesh1->getLocalTransform().setScale(1.0f);
        m_mesh1->attachChild(m_mesh2.get(), AttachRule::Full);
        m_mesh2->getLocalTransform().translate(glm::vec3(0.0f, 3.0f, 0.0f));

        std::shared_ptr<Shader> lineShader = provider->getShaderFromFile(Res::Shader::LINE);
        m_focusedBlockOutline =
            std::make_shared<Wireframe>(Wireframe::fromBoundigBox({glm::vec3(-0.005), glm::vec3(1.005)}));
        m_focusedBlockOutline->assignMaterial(std::make_shared<LineMaterial>(lineShader, glm::vec3(0.05, 0.05, 0.05)));
        m_focusedBlockOutline->setLineWidth(3.5f);
    }
}

void GameInstance::deinitWorld() {
    if (m_playerController) {
        delete m_playerController;
        m_playerController = nullptr;
    }
    if (m_player) {
        delete m_player;
        m_player = nullptr;
    }
    if (m_world) {
        try {
            m_world->syncedSaveChunks();
        } catch (const std::exception& e) {
            lgr::lout.error(e.what());
        }
        if (ApplicationContext* context = Application::getContext()) {
            context->workerPool->cancelJobs(m_world);
            context->workerPool->waitForOwnerCompletion(m_world);
            delete m_world;
            m_world = nullptr;
        }
    }
}

void GameInstance::pushWorldRenderData() const {
    if (ApplicationContext* context = Application::getContext()) {
        Renderer* renderer = context->renderer;
        for (const auto& light : m_lights) {
            renderer->submitLight(light.get());
        }

        for (const auto& val : m_world->loadedChunks()) {
            if (val.second.getMesh()) {
                renderer->submitRenderable(val.second.getMesh());
            }
        }
        renderer->submitRenderable(m_mesh1.get());
        renderer->submitRenderable(m_mesh2.get());

        if (m_player->isFocusingBlock()) {
            m_focusedBlockOutline->getLocalTransform().setPosition(m_player->getFocusedBlock());
            renderer->submitRenderable(m_focusedBlockOutline.get());
        }
    }
}

void GameInstance::update(float deltaTime) {
    m_player->update(deltaTime);
    for (const auto& light : m_lights) {
        light->getLocalTransform().rotate(glm::vec3(0.0f, 10.0f * deltaTime, 0.0f));
    }

    Transform& mehs1Tr = m_mesh1->getLocalTransform();
    mehs1Tr.rotate(10.0f * deltaTime, WorldUp);
    m_world->updateChunks(m_player->getTransform().getPosition(), 2);
}