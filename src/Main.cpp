#include "context.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// matX, X = 2, 3, 4
// vecX, X = 2, 3, 4
// 벡터 원소 접근 : .x, .y, .z, .w
// 얻어오고 싶은 인덱스 연속으로 쓰기 가능, ex) .xyz => vec3
// ex) vec2 someVec; vec4 = someVec.xyxx;

// 콜백함수 정의
// 윈도우 사이즈가 변경되었을 때
void OnFramebufferSizeChange(GLFWwindow* window, int width, int height) {
	SPDLOG_INFO("framebuffer size changed: ({} x {})", width, height);
	auto context = reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
	context->Reshape(width, height);
}
// 키보드가 입력되었을 때
void OnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	SPDLOG_INFO("key: {}, scancode: {}, action: {}, mods: {}{}{}",
		key, scancode,
		action == GLFW_PRESS ? "Pressed" :
		action == GLFW_RELEASE ? "Released" :
		action == GLFW_REPEAT ? "Repeat" : "Unknown",
		mods & GLFW_MOD_CONTROL ? "C" : "-",
		mods & GLFW_MOD_SHIFT ? "S" : "-",
		mods & GLFW_MOD_ALT ? "A" : "-");
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}
// 마우스가 입력되었을 때
void OnCursorPos(GLFWwindow* window, double x, double y) {
	auto context = reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
	context->MouseMove(x, y);
}
void OnMouseButton(GLFWwindow* window, int button, int action, int modifier) {
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, modifier);
	auto context = reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	context->MouseButton(button, action, x, y);
}

void OnCharEvent(GLFWwindow* window, unsigned int ch) {
    ImGui_ImplGlfw_CharCallback(window, ch);
}

void OnScroll(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

int main(int argc, const char** argv){
    SPDLOG_INFO("Start Program");

	// glfw 라이브러리 초기화, 실패하면 에러 출력 후 종료
	SPDLOG_INFO("Initialize glfw");
	if (!glfwInit()) {
		const char* description = nullptr;
		glfwGetError(&description);
		SPDLOG_ERROR("failed to initialize glfw : {}", description);
		return -1;
	}

	// glfw context 세팅과 버전 세팅
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw 윈도우 생성, 성공하면 에러 출력후 종료
	SPDLOG_INFO("Create glfw Window");
	auto window = glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"Tree Generator",nullptr,nullptr);
	if (!window) {
		SPDLOG_ERROR("failed to create glfw window");
		glfwTerminate();
		return -1;
	}
	// 해당 context를 사용
	glfwMakeContextCurrent(window);

	// glad 라이브러리를 사용해 OpenGL 함수 로딩
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		SPDLOG_ERROR("failed to initialize glad");
		glfwTerminate();
		return -1;
	}
	auto glVersion = glGetString(GL_VERSION);
	SPDLOG_INFO("OpenGL context version: {}", (const char*)glVersion);

	// imgui context 생성
	auto imguiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiContext);
	ImGui_ImplGlfw_InitForOpenGL(window, false);
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplOpenGL3_CreateFontsTexture();
	ImGui_ImplOpenGL3_CreateDeviceObjects();

	auto context = Context::Create();
	if(!context){
		SPDLOG_ERROR("failed to create context");
		glfwTerminate();
		return -1;
	}
	glfwSetWindowUserPointer(window, context.get());

	// 윈도우가 생성된 직후에는 해당 콜백함수가 자동으로 실행되지 않으므로 수동으로 실행
	OnFramebufferSizeChange(window, WINDOW_WIDTH, WINDOW_HEIGHT);

	// 콜백함수 등록
	glfwSetFramebufferSizeCallback(window, OnFramebufferSizeChange);
	glfwSetKeyCallback(window, OnKeyEvent);
	glfwSetCharCallback(window, OnCharEvent);
	glfwSetCursorPosCallback(window, OnCursorPos);
	glfwSetMouseButtonCallback(window, OnMouseButton);
	glfwSetScrollCallback(window, OnScroll);
	
	// glfw 루프 실행, 윈도우 close 버튼을 누르면 정상 종료
	SPDLOG_INFO("Start main loop");
	while (!glfwWindowShouldClose(window)) {
		// loop에서 이벤트를 수집
		// 이벤트가 발생했을 때 호출을 무엇을 할지 콜백 함수를 통해 정의
		glfwPollEvents();
		ImGui_ImplGlfw_NewFrame();
    	ImGui::NewFrame();

		context->ProcessInput(window);
		context->Render();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
	// main loop 종료
	context.reset();

	ImGui_ImplOpenGL3_DestroyFontsTexture();
	ImGui_ImplOpenGL3_DestroyDeviceObjects();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(imguiContext);

	glfwTerminate();
	return 0;
}