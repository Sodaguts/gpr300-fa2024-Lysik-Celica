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
#include <iostream>

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

glm::vec3 outlineColor;

struct Material
{
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

struct background_rgba 
{
	float red = 0.9;
	float green = 1.0;
	float blue = 1.0;
	float alpha = 1.0;
}bg_rgba;

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

glm::vec3 ambientModifier;



int main() {
	GLFWwindow* window = initWindow("Test Project", screenWidth, screenHeight);

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader postShader = ew::Shader("assets/post.vert", "assets/post.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	GLuint goldTexture = ew::loadTexture("assets/gold_color.jpg");

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); // Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; // Vertical field of view in degrees

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

	//generate framebuffer
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
		std::cout << "ERROR: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ew::Transform monkeyTransform;
	

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //back face culling
	glEnable(GL_DEPTH_TEST); // Depth testing

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		cameraController.move(window, &camera, deltaTime);
		//RENDER
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glEnable(GL_DEPTH_TEST);

		glClearColor(bg_rgba.red,bg_rgba.green,bg_rgba.blue,bg_rgba.alpha);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, goldTexture);

		ambientModifier = glm::vec3(bg_rgba.red, bg_rgba.blue, bg_rgba.green);

		shader.use();
		shader.setInt("_MainTex", 0);

		shader.setMat4("_Model", glm::mat4(1.0f));
		shader.setMat4("_ViewProjection", camera.projectionMatrix()* camera.viewMatrix());

		shader.setVec3("_EyePos", camera.position);

		
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		shader.setVec3("_AmbientModifier", ambientModifier);

		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();


		

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

		postShader.use();
		postShader.setInt("screenTexture", 0);
		postShader.setVec3("outlineColor", outlineColor);

		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

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

	if (ImGui::CollapsingHeader("Outline"))
	{
		ImGui::ColorEdit3("Color", &outlineColor.r);
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
