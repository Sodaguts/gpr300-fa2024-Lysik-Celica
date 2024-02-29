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
#include <ew/procGen.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

struct 
{
	GLuint fbo;
	GLuint world_position;
	GLuint world_normal;
	GLuint albedo;
	GLuint depth;
} deferred;

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
unsigned int quadVAO;

void createDeferredPass(void) 
{
	glCreateFramebuffers(1, &deferred.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, deferred.fbo);

	glGenTextures(1, &deferred.world_position);
	glBindTexture(GL_TEXTURE_2D, deferred.world_position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &deferred.world_normal);
	glBindTexture(GL_TEXTURE_2D, deferred.world_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);


	glGenTextures(1, &deferred.albedo);
	glBindTexture(GL_TEXTURE_2D, deferred.albedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &deferred.depth);
	glBindTexture(GL_TEXTURE_2D, deferred.depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	//attach components to bound framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferred.world_position, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferred.world_normal, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, deferred.albedo, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, deferred.depth, 0);

	GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "\nERROR:FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void createDisplayPass() 
{
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

	unsigned int quadVBO;
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
}

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	GLuint goldTexture = ew::loadTexture("assets/gold_color.jpg");

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); // Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; // Vertical field of view in degrees

	ew::Shader geometryShader = ew::Shader("assets/geometry.vert", "assets/geometry.frag");
	ew::Shader lightingShader = ew::Shader("assets/lighting.vert", "assets/lighting.frag");

	ew::Transform monkeyTransform;

	ew::Mesh plane = ew::createPlane(1000, 1000, 100);
	ew::Transform planeTransform;


	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	createDeferredPass();
	createDisplayPass();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		//MATH
		//================================
		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
		planeTransform.position.y = -1.0;



		//RENDER
		//================================

		//geometry pass
		glBindFramebuffer(GL_FRAMEBUFFER, deferred.fbo);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); //back face culling
		glEnable(GL_DEPTH_TEST); // Depth testing
		glClearColor(bg_rgba.red, bg_rgba.green, bg_rgba.blue, bg_rgba.alpha);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, goldTexture);

		ambientModifier = glm::vec3(bg_rgba.red, bg_rgba.blue, bg_rgba.green);

		geometryShader.use();
		geometryShader.setInt("_MainTex", 0);

		geometryShader.setMat4("_Model", glm::mat4(1.0f));
		geometryShader.setMat4("_ViewProjection", camera.projectionMatrix()* camera.viewMatrix());

		geometryShader.setVec3("_EyePos", camera.position);

		
		geometryShader.setFloat("_Material.Ka", material.Ka);
		geometryShader.setFloat("_Material.Kd", material.Kd);
		geometryShader.setFloat("_Material.Ks", material.Ks);
		geometryShader.setFloat("_Material.Shininess", material.Shininess);

		//draw plane
		
		geometryShader.setMat4("_Model", planeTransform.modelMatrix());
		plane.draw();

		geometryShader.setVec3("_AmbientModifier", ambientModifier);
		/*geometryShader.setMat4("_Model", monkeyTransform.modelMatrix());

		monkeyModel.draw();*/
		glBindTexture(GL_TEXTURE_2D, brickTexture);

		for (int z = 0; z < 100; z++) 
		{
			for (int x = 0; x < 100; x++) 
			{
				monkeyTransform.position.x = x * -5.0f;
				monkeyTransform.position.z = z * -5.0f;
				geometryShader.setMat4("_Model", monkeyTransform.modelMatrix());
				monkeyModel.draw();
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//lighting pass
		glBindVertexArray(quadVAO);
		glBindTextureUnit(0, deferred.world_position);
		glBindTextureUnit(1, deferred.world_normal);
		glBindTextureUnit(2, deferred.albedo);

		lightingShader.use();
		lightingShader.setMat4("_Model", glm::mat4(1.0f));
		lightingShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		lightingShader.setVec3("_EyePos", camera.position);
		lightingShader.setFloat("_Material.Ka", material.Ka);
		lightingShader.setFloat("_Material.Kd", material.Kd);
		lightingShader.setFloat("_Material.Ks", material.Ks);
		lightingShader.setFloat("_Material.Shininess", material.Shininess);

		lightingShader.setVec3("_AmbientModifier", ambientModifier);

		lightingShader.setInt("gPosition", 0);
		lightingShader.setInt("gNormals", 1);
		lightingShader.setInt("gAlbedo", 2);

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
	//Using a Child allow to fill all the space of the window.
	ImVec2 windowSize = ImGui::GetWindowSize();

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
	ImGui::Text("WorldPosition");
	ImGui::Image((ImTextureID)deferred.world_position, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::Text("WorldNormal");
	ImGui::Image((ImTextureID)deferred.world_normal, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::Text("Albedo");
	ImGui::Image((ImTextureID)deferred.albedo, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::Text("Depth");
	ImGui::Image((ImTextureID)deferred.depth, windowSize, ImVec2(0, 1), ImVec2(1, 0));
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