#include "Application.h"
#include "engine/GameInstance.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/Renderer.h"
#include "engine/ui/GameOverlay.h"
#include "engine/ui/MainMenu.h"
#include "engine/ui/Ui.h"
#include "Logger.h"
#include "providers/Provider.h"
#include "threading/ThreadPool.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <sstream>

using namespace std;

ApplicationContext* Application::currentContext = nullptr;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_RELEASE) {
		KeyEvent event = action == GLFW_PRESS ? KeyEvent::ButtonDown : KeyEvent::ButtonUp;
		KeyEventData data;
		data.keycode = key;
		data.mods = mods;

		Application::getContext()->io->notifyObservers(event, data);
	}
}

static void mouseKeyCallback(GLFWwindow* window, int button, int action, int mods) {
	MousEvent event = action == GLFW_PRESS ? MousEvent::ButtonDown : MousEvent::ButtonUp;
	MouseEventData data;
	data.key.code = button;
	Application::getContext()->io->notifyObservers(event, data);
}

static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	MousEvent event = MousEvent::Scroll;
	MouseEventData data;
	data.delta.x = xoffset;
	data.delta.y = yoffset;
	Application::getContext()->io->notifyObservers(event, data);
}

static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	static double lastXPos = 0;
	static double lastYPos = 0;

	double xDelta = xpos - lastXPos;
	double yDelta = ypos - lastYPos;
	lastXPos = xpos;
	lastYPos = ypos;

	MousEvent event = MousEvent::Move;
	MouseEventData data;
	data.delta.x = xDelta;
	data.delta.y = yDelta;
	Application::getContext()->io->notifyObservers(event, data);
}

void Application::setCurrentContext(ApplicationContext* context) {
	if (Application::currentContext) {
		Application::deleteCurrentContext();
	}
	Application::currentContext = context;
}

ApplicationContext* Application::createContext() {
	ApplicationContext* context = new ApplicationContext;
	context->workerPool = new ThreadPool(6);
	context->window = nullptr;
	context->provider = new Provider;
	context->renderer = new Renderer;
	context->instance = new GameInstance;
	context->currentWindow = nullptr;
	context->io = new AppIO;
	return context;
}

void Application::deleteCurrentContext() {
	ApplicationContext* context = Application::currentContext;
	if (context) {
		delete context->workerPool;
		delete context->provider;
		delete context->renderer;
		delete context->instance;
		if (context->currentWindow) {
			delete context->currentWindow;
		}
		delete context->io;

		// Keep as last deletion!!!
		if (context->window) {
			glfwDestroyWindow(context->window); // This implicitly destroys open gl context -> gl calls afterwards will cause error
		}
		delete context;
	}
}

ApplicationContext* Application::getContext() {
	return Application::currentContext;
}

void Application::run() {
	{
		if (!glfwInit())
			throw runtime_error("Error initializing glew!");

		// Specifiy OpenGl Version to use
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		// Core does not auto create vertex array objects, compatibitly does
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4); // Request MSAA 4

		// Create current context
		ApplicationContext* context = Application::createContext();
		Application::setCurrentContext(context);

		// Create a windowed mode window and its OpenGL context
		context->window = glfwCreateWindow(960, 540, "TooManyBlocks", NULL, NULL);
		if (!context->window) {
			glfwTerminate();
			throw runtime_error("Could not create window!");
		}

		// Make the window's context current
		glfwMakeContextCurrent(context->window);

		// IO Callbacks
		glfwSetKeyCallback(context->window, keyCallback);
		glfwSetMouseButtonCallback(context->window, mouseKeyCallback);
		glfwSetScrollCallback(context->window, mouseScrollCallback);
		glfwSetCursorPosCallback(context->window, cursorPositionCallback);

		// Sync with refresh rate
		glfwSwapInterval(1);

		if (glewInit() != GLEW_OK) {
			throw std::runtime_error("Error Glew ok");
		}

		// Check Antialisasing
		std::stringstream detailsBuf;

		detailsBuf << "Open GL Version: " << glGetString(GL_VERSION) << std::endl;
		detailsBuf << "Graphics: " << glGetString(GL_RENDERER) << "[" << glGetString(GL_VENDOR) << "]" << std::endl;

		int samples;
		GLCALL(glGetIntegerv(GL_SAMPLES, &samples));
		detailsBuf << "Antialiasing: MSAA " << samples << std::endl;
		lgr::lout.info(detailsBuf.str());
	}
	{
		ApplicationContext* context = Application::getContext();

		ImGui::CreateContext();
		ImGui_ImplOpenGL3_Init();
		ImGui_ImplGlfw_InitForOpenGL(context->window, true);
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		ImGui::StyleColorsDark();
		
		float fontSize1 = 32.0f;
		float fontSize2 = 24.0f;
		ImFont* font1 = io.Fonts->AddFontFromFileTTF("res/fonts/ProggyClean.ttf", fontSize1);
		ImFont* font2 = io.Fonts->AddFontFromFileTTF("res/fonts/ProggyClean.ttf", fontSize2);

		if (!font1 || !font2) {
			lgr::lout.error("Failed to load font!");
		}

		io.Fonts->Build();

		ImGui_ImplOpenGL3_CreateFontsTexture();
		
		UI::registerWindow<UI::MainMenu>("MainMenu");
		UI::registerWindow<UI::GameOverlay>("GameOverlay");
		UI::navigateToWindow(*context, "MainMenu");

		// Loop until the user closes the window
		try {
			context->renderer->initialize();
			double previousTime = glfwGetTime();

			while (!glfwWindowShouldClose(context->window)) {
				double currentTime = glfwGetTime();
				float msframeTime = static_cast<float>(currentTime - previousTime) * 1000.0f;
				previousTime = currentTime;

				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				ImGui::PushFont(font1);

				if (context->instance->isInitialized) {
					context->instance->update(msframeTime);
					Scene scene = context->instance->craftScene();
					context->renderer->renderScene(scene, *context);
				}

				context->currentWindow->render(*context);
				
				ImGui::PopFont();

				// Rendering
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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