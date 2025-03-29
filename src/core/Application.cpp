#include "Application.h"
#include "engine/GameInstance.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/Renderer.h"
#include "engine/ui/fonts/FontUtil.h"
#include "engine/ui/GameOverlay.h"
#include "engine/ui/MainMenu.h"
#include "engine/ui/PauseMenu.h"
#include "engine/ui/Ui.h"
#include "engine/ui/WorldSelection.h"
#include "Logger.h"
#include "providers/Provider.h"
#include "threading/ThreadPool.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <sstream>

#define WORKER_COUNT 6

ApplicationContext* Application::currentContext = nullptr;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (ApplicationContext* context = Application::getContext()) {
		if (action == GLFW_PRESS || action == GLFW_RELEASE) {
			KeyEventData data;
			data.keycode = key;
			data.mods = mods;

			context->io->keyAdapter().notifyObservers(action == GLFW_PRESS ? KeyEvent::ButtonDown : KeyEvent::ButtonUp, data);
		}
	}
}

static void mouseKeyCallback(GLFWwindow* window, int button, int action, int mods) {
	if (ApplicationContext* context = Application::getContext()) {
		MouseEventData data;
		data.key.code = button;
		context->io->mouseAdapter().notifyObservers(action == GLFW_PRESS ? MousEvent::ButtonDown : MousEvent::ButtonUp, data);
	}
}

static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	if (ApplicationContext* context = Application::getContext()) {
		MouseEventData data;
		data.delta.x = xoffset;
		data.delta.y = yoffset;
		context->io->mouseAdapter().notifyObservers(MousEvent::Scroll, data);
	}
}

static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	if (ApplicationContext* context = Application::getContext()) {
		MouseEventData data;
		data.delta.x = xpos - context->lastMousepositionX;
		data.delta.y = ypos - context->lastMousepositionY;

		context->io->mouseAdapter().notifyObservers(MousEvent::Move, data);
		context->lastMousepositionX = xpos;
		context->lastMousepositionY = ypos;
	}
}

static void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (ApplicationContext* context = Application::getContext()) {
		context->screenWidth = static_cast<unsigned int>(width);
		context->screenHeight = static_cast<unsigned int>(height);
	}
}

void Application::setCurrentContext(ApplicationContext* context) {
	if (Application::currentContext) {
		Application::deleteCurrentContext();
	}
	Application::currentContext = context;
}

ApplicationContext* Application::createContext() {
	ApplicationContext* context = new ApplicationContext;
	context->screenWidth = 0;
	context->screenHeight = 0;
	context->lastMousepositionX = 0;
	context->lastMousepositionY = 0;

	context->workerPool = new ThreadPool(WORKER_COUNT);
	context->window = nullptr;
	context->provider = new Provider;
	context->renderer = new Renderer;
	context->instance = new GameInstance;
	context->currentWindow = nullptr;
	context->nextWindow = nullptr;
	context->fontPool = new FontPool;
	context->io = new AppIO;
	return context;
}

void Application::deleteCurrentContext() {
	if (ApplicationContext* context = Application::currentContext) {
		context->workerPool->forceCancelAllJobs();
		context->workerPool->waitForCompletion();
		delete context->workerPool;
		delete context->provider;
		delete context->renderer;
		delete context->instance;
		if (context->currentWindow) {
			delete context->currentWindow;
		}
		delete context->fontPool;
		delete context->io;

		// Keep as last deletion!!!
		if (context->window) {
			glfwDestroyWindow(context->window); // This implicitly destroys open gl context -> gl calls afterwards will cause error
		}
		delete context;
		Application::currentContext = nullptr;
	}
}

ApplicationContext* Application::getContext() {
	return Application::currentContext;
}

void Application::run() {
	{
		if (!glfwInit())
			throw std::runtime_error("Error initializing glew!");

		// Specifiy OpenGl Version to use
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		// Core does not auto create vertex array objects, compatibitly does
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4); // Request MSAA 4

		#ifdef DEBUG_MODE // Create debug open gl context (Some graphics cards do not enable debugging by default)
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		#endif

		// Create current context
		ApplicationContext* context = Application::createContext();
		// Set initial screen dimensions
		context->screenWidth = 960;
		context->screenHeight = 540;
		Application::setCurrentContext(context);

		// Create a windowed mode window and its OpenGL context
		context->window = glfwCreateWindow(context->screenWidth, context->screenHeight, "TooManyBlocks", NULL, NULL);
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

		// Sync with refresh rate
		glfwSwapInterval(1);

		if (glewInit() != GLEW_OK) {
			throw std::runtime_error("Error Glew ok");
		}

		// Graphic api details
		std::ostringstream detailsBuf;		
		detailsBuf << "Open GL Version: " << glGetString(GL_VERSION) << std::endl;
		detailsBuf << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
		detailsBuf << "Graphics: " << glGetString(GL_RENDERER) << "[" << glGetString(GL_VENDOR) << "]" << std::endl;
		
		// Check Antialisasing
		int samples;
		GLCALL(glGetIntegerv(GL_SAMPLES, &samples));
		detailsBuf << "Antialiasing: MSAA " << samples << std::endl;
		lgr::lout.info(detailsBuf.str());

		// ImGui setup
		ImGui::CreateContext();
		ImGui_ImplOpenGL3_Init();
		ImGui_ImplGlfw_InitForOpenGL(context->window, true);
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();
		
		context->fontPool->loadFontSizes("res/fonts/ProggyClean.ttf", {16.0f, 32.0f, 48.0f, 64.0f});
		
		UI::registerWindow<UI::MainMenu>("MainMenu");
		UI::registerWindow<UI::GameOverlay>("GameOverlay");
		UI::registerWindow<UI::WorldSelection>("WorldSelection");
		UI::registerWindow<UI::PauseMenu>("PauseMenu");
		UI::navigateToWindow(*context, "MainMenu");
	}
	{
		ApplicationContext* context = Application::getContext();
		// Loop until the user closes the window
		try {
			context->renderer->initialize();
			double previousTime = glfwGetTime();

			while (!glfwWindowShouldClose(context->window) && !context->instance->gameState.quitGame) {
				double currentTime = glfwGetTime();
				float msframeTime = static_cast<float>(currentTime - previousTime) * 1000.0f;
				previousTime = currentTime;
				
				if (context->instance->isWorldInitialized()) {
					if (!context->instance->gameState.gamePaused) {
						context->instance->update(msframeTime);
					}
					Scene scene = context->instance->craftScene();
					context->renderer->renderScene(scene, *context);
				}
				if (context->nextWindow) {
					// Navigate safely to new window
					if (context->currentWindow) {
						context->workerPool->cancelJobs(context->currentWindow);
                    	context->workerPool->waitForOwnerCompletion(context->currentWindow);
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
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	Application::deleteCurrentContext();
	// Dont use GLCALL because gl context is removed when reaching this
	glfwTerminate();
}