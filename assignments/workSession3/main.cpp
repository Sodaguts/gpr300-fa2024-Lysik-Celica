#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>

#include <ew/transform.h>

#include <ew/cameraController.h> 

#include <ew/texture.h>
#include <ew/procGen.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Camera camera;
ew::CameraController cameraController;

void resetCamera(ew::Camera* camera, ew::CameraController* controller);

struct Material
{
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

struct background_rgba 
{
	float red = 0.3;
	float green = 0.4;
	float blue = 0.8;
	float alpha = 1.0;
}bg_rgba;

glm::vec3 ambientModifier;
float heightScaler = 10.0f;

float cutoffValue = 0.5;

float quadVertices[] =
{
	//x     y      //u    v
	//triangle 1
	-1.0f,  1.0f,  0.0f,  1.0f,
	-1.0f, -1.0f,  0.0f,  0.0f,
	 1.0f,  1.0f,  1.0f,  1.0f,

	 //triangle 2
	 1.0f, -1.0f,  1.0f,  0.0f,
	 1.0f,  1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  0.0f,  0.0f
};

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	GLuint heightMapTexture = ew::loadTexture("assets/heightmap.png");

	GLuint gradient1 = ew::loadTexture("assets/transitions/transitions/gradient1.png");
	GLuint gradient2 = ew::loadTexture("assets/transitions/transitions/gradient2.png");
	GLuint gradient3 = ew::loadTexture("assets/transitions/transitions/gradient3.png");

	ew::Shader transitionShader = ew::Shader("assets/transitions.vert", "assets/transitions.frag");

	const int PLANE_WIDTH = 50, PLANE_HEIGHT = 50;
	int subdivisions = 128;
	ew::Mesh plane = ew::createPlane(PLANE_WIDTH, PLANE_HEIGHT, subdivisions);

	ew::Shader heightMapShader = ew::Shader("assets/height.vert", "assets/height.frag");


	camera.position = glm::vec3(0.0f, 5.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); // Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; // Vertical field of view in degrees

	ew::Transform monkeyTransform;

	unsigned int quadVAO, quadVBO;

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);


	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //back face culling
	glEnable(GL_DEPTH_TEST); // Depth testing

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();


		//MATH
		//=================================
		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);

		//RENDER
		//=================================
		
		glClearColor(bg_rgba.red,bg_rgba.green,bg_rgba.blue,bg_rgba.alpha);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gradient1);
		//use transition shader
		transitionShader.use();
		transitionShader.setInt("_Texture", 0);
		transitionShader.setFloat("cutoffValue", cutoffValue);

		glDrawArrays(GL_TRIANGLES, 0, 6);


		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMapTexture);

		heightMapShader.use();
		heightMapShader.setInt("_MainTex", 0);

		heightMapShader.setInt("_HeightMap", 0);
		heightMapShader.setFloat("_heightScalar", heightScaler);

		heightMapShader.setMat4("_Model", glm::mat4(1.0f));
		heightMapShader.setMat4("_ViewProjection", camera.projectionMatrix()* camera.viewMatrix());

		heightMapShader.setMat4("_Model", glm::mat4(1.0f));

		plane.draw();*/

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	ImGui::Text("Add Settings Here!");
	if (ImGui::Button("Reset Camera"))
	{
		resetCamera(&camera, &cameraController);
	}

	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}

	if (ImGui::CollapsingHeader("Background Color"))
	{
		ImGui::ColorEdit4("Background Color", &bg_rgba.red);
	}
	if (ImGui::CollapsingHeader("Height Map"))
	{
		ImGui::SliderFloat("Height", &heightScaler, 1.0f, 100.0f);
	}
	if (ImGui::CollapsingHeader("Transition"))
	{
		ImGui::SliderFloat("Cutoff Value", &cutoffValue, 0.0f, 1.0f);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	//aspect ratio correction
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) 
{
	camera->position = glm::vec3(0,0,5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}
