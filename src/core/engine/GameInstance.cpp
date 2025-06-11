#include "GameInstance.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <chrono>
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
#include "engine/rendering/lowlevelapi/TransformFeedbackShader.h"
#include "engine/rendering/mat/ChunkMaterial.h"
#include "engine/rendering/mat/LineMaterial.h"
#include "engine/rendering/mat/ParticleMaterial.h"
#include "engine/rendering/mat/SimpleMaterial.h"
#include "engine/rendering/mat/SkeletalMaterial.h"
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
        m_mesh1 = provider->getStaticMeshFromFile(Res::Model::TEST_UNIT_BLOCK);
        m_mesh1->assignMaterial(testMaterial1);
        m_mesh2 = provider->getStaticMeshFromFile(Res::Model::TEST_UNIT_BLOCK);
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

        std::shared_ptr<Shader> skeletalShader = provider->getShaderFromFile(Res::Shader::SKELETAL_MESH);
        auto start = std::chrono::high_resolution_clock::now();
        std::shared_ptr<Texture> skeletalTexture = provider->getTextureFromFile(Res::Texture::TESTFLY_TEXTURE);
        auto end = std::chrono::high_resolution_clock::now();
        m_skeletalMesh = provider->getSkeletalMeshFromFile(Res::Model::TESTFLY);
        m_skeletalMesh->assignMaterial(std::make_shared<SkeletalMaterial>(skeletalShader, skeletalTexture));
        m_skeletalMesh->getLocalTransform().setPosition(glm::vec3(10.0f, 8.0f, 5.0f));

        // Particles
        m_particles = std::make_shared<ParticleSystem>(std::vector<GenericGPUParticleModule>{
            ParticleModules::SphereSpawn(0.5f),
            ParticleModules::InitialVelocityInCone(5.0f, 10.0f, glm::vec3(1.0, 5.0, 0.5), 45.0f),
            ParticleModules::InitialLifetime(0.5f, 2.0f), ParticleModules::Acceleration(glm::vec3(0, -9.81, 0)),
            ParticleModules::ColorOverLife({
                {0.0f, glm::vec4(1, 0, 0, 1)},  // red at birth
                {0.5f, glm::vec4(0, 1, 0, 1)},  // green halfway
                {1.0f, glm::vec4(0, 0, 1, 0)}   // fade to blue and alpha 0 at death
            })
        });
        std::shared_ptr<Shader> particleTFShader =
            std::make_shared<TransformFeedbackShader>(TransformFeedbackShader::create(
                "res/shaders/particleTFShader", {"tf_color", "tf_velocity", "tf_position", "tf_timeToLive",
                                                 "tf_initialTimeToLive", "tf_size", "tf_flags"}
            ));
        std::shared_ptr<Shader> particleShader = provider->getShaderFromFile("res/shaders/particleShader");
        m_particles->assignMaterial(std::make_shared<ParticleMaterial>(particleShader, particleTFShader));
        m_particles->getLocalTransform().setPosition(glm::vec3(10.0f, 12.0f, 5.0f));

        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        lgr::lout.debug("Loaded world in: " + std::to_string(dur.count()) + " ms");
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
        renderer->submitRenderable(m_skeletalMesh.get());
        renderer->submitRenderable(m_particles.get());

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

    if (!m_skeletalMesh->getActiveAnimation()) {
        m_skeletalMesh->playAnimation("Idle", true);
    }
    m_skeletalMesh->update(deltaTime);
}