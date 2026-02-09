#include "Application.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <sstream>
#include <stdexcept>

#include "AppConstants.h"
#include "Logger.h"
#include "engine/GameInstance.h"
#include "engine/audio/AudioEngine.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/Renderer.h"
#include "engine/resource/providers/CPUAssetProvider.h"
#include "engine/ui/AboutScreen.h"
#include "engine/ui/GameOverlay.h"
#include "engine/ui/MainMenu.h"
#include "engine/ui/PauseMenu.h"
#include "engine/ui/Ui.h"
#include "engine/ui/WorldSelection.h"
#include "engine/ui/fonts/FontUtil.h"
#include "threading/Future.h"
#include "threading/ThreadPool.h"

#define WORKER_COUNT 6

ApplicationContext* Application::currentContext = nullptr;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
        KeyEventData data;
        data.keycode = key;
        data.mods = mods;

        ApplicationContext* context = Application::getContext();
        context->io->keyAdapter().notifyObservers(
            action == GLFW_PRESS ? KeyEvent::ButtonDown : KeyEvent::ButtonUp, data
        );
    }
}

static void mouseKeyCallback(GLFWwindow* window, int button, int action, int mods) {
    MouseEventData data;
    data.key.code = button;

    ApplicationContext* context = Application::getContext();
    context->io->mouseAdapter().notifyObservers(
        action == GLFW_PRESS ? MousEvent::ButtonDown : MousEvent::ButtonUp, data
    );
}

static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    MouseEventData data;
    data.delta.x = xoffset;
    data.delta.y = yoffset;

    ApplicationContext* context = Application::getContext();
    context->io->mouseAdapter().notifyObservers(MousEvent::Scroll, data);
}

static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    ApplicationContext* context = Application::getContext();
    MouseEventData data;
    data.delta.x = xpos - context->state.lastMousepositionX;
    data.delta.y = ypos - context->state.lastMousepositionY;

    context->io->mouseAdapter().notifyObservers(MousEvent::Move, data);
    context->state.lastMousepositionX = xpos;
    context->state.lastMousepositionY = ypos;
}

static void windowResizeCallback(GLFWwindow* window, int width, int height) {
    ApplicationContext* context = Application::getContext();
    context->state.screenWidth = static_cast<unsigned int>(width);
    context->state.screenHeight = static_cast<unsigned int>(height);
}

static void scheduleCallback(std::unique_ptr<FutureBase> future, Executor executor) {
    ApplicationContext* context = Application::getContext();
    context->workerPool->pushJob(std::move(future), executor);
}

void Application::init() {
    if (!glfwInit()) throw std::runtime_error("Error initializing glew!");

    // Specifiy OpenGl Version to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Core does not auto create vertex array objects, compatibitly does
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);  // Request MSAA 4

#ifdef DEBUG_MODE  // Create debug open gl context (Some graphics cards do not enable debugging by default)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    // Create current context
    ApplicationContext* context = Application::createContext();
    // Set initial screen dimensions
    context->state.screenWidth = 960;
    context->state.screenHeight = 540;
    Application::setCurrentContext(context);

    // Create a windowed mode window and its OpenGL context
    context->window = glfwCreateWindow(
        context->state.screenWidth, context->state.screenHeight, "TooManyBlocks", NULL, NULL
    );
    if (!context->window) {
        glfwTerminate();
        throw std::runtime_error("Could not create window!");
    }

    // Make the window's context current
    glfwMakeContextCurrent(context->window);

    // IO Callbacks
    glfwSetKeyCallback(context->window, keyCallback);
    glfwSetMouseButtonCallback(context->window, mouseKeyCallback);
    glfwSetScrollCallback(context->window, mouseScrollCallback);
    glfwSetCursorPosCallback(context->window, cursorPositionCallback);
    glfwSetFramebufferSizeCallback(context->window, windowResizeCallback);

    FutureBase::scheduleCallback = scheduleCallback;

    // Sync with refresh rate
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Error Glew ok");
    }

    // Graphic api details
    std::ostringstream detailsBuf;
    detailsBuf << "Open GL Version: " << glGetString(GL_VERSION) << "\n";
    detailsBuf << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
    detailsBuf << "Graphics: " << glGetString(GL_RENDERER) << "[" << glGetString(GL_VENDOR) << "]" << "\n";

    // Check Antialisasing
    int samples;
    GLCALL(glGetIntegerv(GL_SAMPLES, &samples));
    detailsBuf << "Antialiasing: MSAA " << samples << "\n";
    lgr::lout.info(detailsBuf.str());

    // ImGui setup
    ImGui::CreateContext();
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplGlfw_InitForOpenGL(context->window, true);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    context->fontPool->loadFontSizes(Res::Font::PROGGY_CLEAN, {16.0f, 32.0f, 48.0f, 64.0f});

    UI::registerWindow<UI::MainMenu>("MainMenu");
    UI::registerWindow<UI::GameOverlay>("GameOverlay");
    UI::registerWindow<UI::WorldSelection>("WorldSelection");
    UI::registerWindow<UI::PauseMenu>("PauseMenu");
    UI::registerWindow<UI::AboutScreen>("AboutScreen");
    UI::navigateToWindow(*context, "MainMenu");
}

void Application::updateStats(float deltaTime) {
    ApplicationContext* context = Application::getContext();
    CpuTimes currentCpuTimes = getCpuTimes();
    context->stats.cpuUsage = getCpuUsage(context->stats.cpuTimes, currentCpuTimes, deltaTime);
    context->stats.memInfo = getSystemMemoryInfo();
    context->stats.processUsedBytes = getProcessUsedBytes();
    context->stats.processIo = getProcessIO();
    context->stats.cpuTimes = currentCpuTimes;
}

void Application::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Application::deleteCurrentContext();
    // Dont use GLCALL because gl context is removed when reaching this
    glfwTerminate();
}

void Application::setCurrentContext(ApplicationContext* context) {
    if (Application::currentContext) {
        Application::deleteCurrentContext();
    }
    Application::currentContext = context;
}

ApplicationContext* Application::createContext() {
    ApplicationContext* context = new ApplicationContext{};

    context->workerPool = new ThreadPool(WORKER_COUNT);
    context->window = nullptr;
    context->provider = new CPUAssetProvider;
    context->renderer = new Renderer;
    context->audioEngine = new AudioEngine;
    context->instance = new GameInstance;
    context->currentWindow = nullptr;
    context->nextWindow = nullptr;
    context->fontPool = new FontPool;
    context->io = new AppIO;
    return context;
}

void Application::deleteCurrentContext() {
    if (ApplicationContext* context = Application::currentContext) {
        context->workerPool->shutdown();

        delete context->provider;
        delete context->renderer;
        delete context->instance;
        if (context->currentWindow) {
            delete context->currentWindow;
        }
        delete context->audioEngine;
        delete context->workerPool;
        delete context->fontPool;
        delete context->io;
        
        // Keep as last deletion!!!
        if (context->window) {
            glfwDestroyWindow(
                context->window
            );  // This implicitly destroys open gl context -> gl calls afterwards will cause error
        }
        delete context;

        Application::currentContext = nullptr;
    }
}

ApplicationContext* Application::getContext() {
    if (!Application::currentContext) throw std::runtime_error("Application context unavailable");
    return Application::currentContext;
}

void Application::run() {
    init();

    ApplicationContext* context = Application::getContext();
    // Loop until the user closes the window
    try {
        context->renderer->initialize();
        float previousTime = static_cast<float>(glfwGetTime());
        float statUpdateAccumulator = previousTime;
        while (!glfwWindowShouldClose(context->window) && !context->instance->gameState.quitGame) {
            context->stats.elapsedAppTime = static_cast<float>(glfwGetTime());
            context->stats.deltaAppTime = context->stats.elapsedAppTime - previousTime;
            previousTime = context->stats.elapsedAppTime;
            statUpdateAccumulator += context->stats.deltaAppTime;

            if (statUpdateAccumulator >= 1.0f) {
                updateStats(statUpdateAccumulator);
                statUpdateAccumulator = 0.0f;
            }

            if (context->instance->isWorldInitialized()) {
                if (!context->instance->gameState.gamePaused) {
                    context->instance->update(context->stats.deltaAppTime);
                } else {
                    context->instance->gameState.deltaTime = 0.0f;
                }
                context->instance->pushWorldRenderData();
                context->renderer->render(*context);
            }

            context->audioEngine->update(context->stats.deltaAppTime);
            context->workerPool->processMainThreadJobs();
            context->workerPool->cleanupFinishedJobs();

            if (context->nextWindow) {
                // Navigate safely to new window
                if (context->currentWindow) {
                    delete context->currentWindow;
                }
                context->currentWindow = context->nextWindow;
                context->nextWindow = nullptr;
            }
            if (context->currentWindow) {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                context->currentWindow->render(*context);
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            // Swap front and back buffers
            glfwSwapBuffers(context->window);
            // Poll for and process events, including set callbacks
            glfwPollEvents();
        }
    } catch (const std::exception& e) {
        lgr::lout.error("Error during game loop: " + std::string(e.what()));
    }

    shutdown();
}