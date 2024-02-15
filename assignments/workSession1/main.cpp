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

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	ew::Model shellModel = ew::Model("assets/greenshell/greenshell.obj");

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	GLuint goldTexture = ew::loadTexture("assets/gold_color.jpg");

	GLuint shellTextureColor = ew::loadTexture("assets/greenshell/greenshell_col.png");
	GLuint shellTextureAO = ew::loadTexture("assets/greenshell/greenshell_ao.png");
	GLuint shellTextureMTL = ew::loadTexture("assets/greenshell/greenshell_mtl.png");
	GLuint shellTextureRGH = ew::loadTexture("assets/greenshell/greenshell_rgh.png");
	GLuint shellTextureSPC = ew::loadTexture("assets/greenshell/greenshell_spc.png");

	ew::Shader shellShader = ew::Shader("assets/shell.vert", "assets/shell.frag");

	camera.position = glm::vec3(0.0f, 0.0f, 10.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); // Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; // Vertical field of view in degrees

	ew::Transform monkeyTransform;
	ew::Transform shellTransform;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //back face culling
	glEnable(GL_DEPTH_TEST); // Depth testing

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glClearColor(bg_rgba.red,bg_rgba.green,bg_rgba.blue,bg_rgba.alpha);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shellTextureColor); //color
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shellTextureAO); //ambient occlusion
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, shellTextureMTL); //metallic
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shellTextureRGH); //roughness
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shellTextureSPC); 

		ambientModifier = glm::vec3(bg_rgba.red, bg_rgba.blue, bg_rgba.green);

		shellShader.use();
		//shellShader.setInt("_MainTex", 0);

		shellShader.setInt("material.color", 0);
		shellShader.setInt("material.roughness",3);
		shellShader.setInt("material.metallic",2);
		shellShader.setInt("material.occlusion", 1);
		shellShader.setInt("material.specular", 4);

		shellShader.setMat4("_Model", glm::mat4(1.0f));
		shellShader.setMat4("_ViewProjection", camera.projectionMatrix()* camera.viewMatrix());

		shellShader.setVec3("_EyePos", camera.position);

		
		shellShader.setFloat("_Material.Ka", material.Ka);
		shellShader.setFloat("_Material.Kd", material.Kd);
		shellShader.setFloat("_Material.Ks", material.Ks);
		shellShader.setFloat("_Material.Shininess", material.Shininess);

		shellShader.setVec3("_AmbientModifier", ambientModifier);

		shellTransform.rotation = glm::rotate(shellTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
		shellShader.setMat4("_Model", shellTransform.modelMatrix());
		cameraController.move(window, &camera, deltaTime);
		shellModel.draw();



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
