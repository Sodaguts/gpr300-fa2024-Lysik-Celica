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

bool invertColors = false;
bool gamma = false;
glm::vec3 lightPosition = glm::vec3(0.0, 1.0, 0.0);

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

//static void create_debug_pass(void) 
//{
//	glGenVertexArrays(1, &debugVertices);
//}


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader postShader = ew::Shader("assets/post.vert", "assets/post.frag");

	// outline shader
	ew::Shader singleColorShader = ew::Shader("assets/lit.vert", "assets/singleColor.frag");

	//shadow mapping
	ew::Shader shadowShader = ew::Shader("assets/depthShader.vert", "assets/depthShader.frag");
	ew::Shader debugShader = ew::Shader("assets/debug.vert", "assets/debug.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	GLuint goldTexture = ew::loadTexture("assets/gold_color.jpg");

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); // Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; // Vertical field of view in degrees

	ew::Transform monkeyTransform;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //back face culling
	glEnable(GL_DEPTH_TEST); // Depth testing

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);


	float debugVertices[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(debugVertices), &debugVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindVertexArray(0);


	//Adding changes for shadow mapping here
	unsigned int fbo;
	glCreateFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*unsigned int colorBuffer;
	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);*/

	//rbo
	//unsigned int rbo;
	//glGenRenderbuffers(1, &rbo);
	//glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "\nERROR:FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		// ALL MATH NEEDED HERE
		// light source variables
		auto light_pos = glm::vec3(0.0, 10.0, 0.0);
		auto light_view = glm::lookAt(light_pos, glm::vec3(0.0f), glm::vec3(0.0, 0.0, -1.0));
		auto light_proj = glm::ortho(-5.0f, 5.0f, 0.0f, 1000.0f);

		// from learnopengl 
		float near_plane = 1.0f, far_plane = 7.5f;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(
			glm::vec3(-2.0f, 4.0f, -1.0f),
			glm::vec3(0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		cameraController.move(window, &camera, deltaTime);
		//====================================
		//RENDER DEPTH TO FBO
		glViewport(0, 0, screenWidth, screenHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		shadowShader.use();
		shadowShader.setMat4("_Model", monkeyTransform.modelMatrix());
		shadowShader.setMat4("_ViewProjection", lightSpaceMatrix);
		monkeyModel.draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//====================================

		//RENDER COMPLETE SCENE
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// pipeline
		//glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK); //back face culling

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, brickTexture);

		// use a custom shader that smaples from depth buffer.
		// https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/3.1.1.shadow_mapping_depth/shadow_mapping_depth.cpp
		shader.use();
		shader.setInt("_MainTex", 0);
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setVec3("_EyePos", camera.position);
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		monkeyModel.draw();
		//====================================

		// RENDER DEBUG QUAD
		// DEBUG TEXTURE SGHOULD BE CENTERED, could be UV or vertex issue
		glViewport(screenWidth - 150, 0, 150, 150);
		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		debugShader.use();
		debugShader.setInt("debug_image", 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//====================================
		


		/*glClearColor(bg_rgba.red,bg_rgba.green,bg_rgba.blue, bg_rgba.alpha);
		if (invertColors == true)
		{
			glClearColor(1.0 - bg_rgba.red, 1.0 - bg_rgba.green, 1.0 - bg_rgba.blue, bg_rgba.alpha);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);		*/

		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, goldTexture);*/

		////ambientModifier = glm::vec3(bg_rgba.red, bg_rgba.blue, bg_rgba.green);

		//singleColorShader.use();
		//singleColorShader.setInt("_MainTex", 0);
		//singleColorShader.setMat4("_Model", glm::mat4(1.0f));
		//singleColorShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		//singleColorShader.setVec3("_EyePos", camera.position);

		/*shader.use();
		shader.setInt("_MainTex", 0);

		shader.setMat4("_Model", glm::mat4(1.0f));
		shader.setMat4("_ViewProjection", camera.projectionMatrix()* camera.viewMatrix());

		shader.setVec3("_EyePos", camera.position);

		
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);*/

		//shader.setVec3("_AmbientModifier", ambientModifier);
		/*if (invertColors == true)
		{
			shader.setInt("_isInverted", 1);
		}
		else
		{
			shader.setInt("_isInverted", 0);
		}

		if (gamma == true)
		{
			shader.setInt("_gamma", 1);
		}
		else
		{
			shader.setInt("_gamma", 0);
		}
		shader.setVec3("_LightPos", lightPosition);*/



		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_DEPTH_TEST);

		//glViewport(screenWidth - 150, 0, 150, 150);


		////glClearColor(bg_rgba.red, bg_rgba.green, bg_rgba.blue, bg_rgba.alpha);
		////glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//
		//glClear(GL_DEPTH_BUFFER_BIT);

		//postShader.use();
		//postShader.setInt("screenTexture", 0);

		////glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


		//glBindVertexArray(quadVAO);
		////glBindTexture(GL_TEXTURE_2D, colorBuffer);


		//glDrawArrays(GL_TRIANGLES, 0, 6);

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
	if (ImGui::CollapsingHeader("Post Processing Effects"))
	{
		ImGui::Checkbox("Inverted", &invertColors);
		ImGui::Checkbox("Gamma", &gamma);
		ImGui::SliderFloat3("Light Pos", &lightPosition.x, -1.0, 1.0);
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
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}
