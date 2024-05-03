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

struct 
{
	GLuint fbo;

}gBuffer;

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
	float red = 0.0;
	float green = 0.0;
	float blue = 0.0;
	float alpha = 1.0;
}bg_rgba;

glm::vec3 ambientModifier;
unsigned int quadVAO;

//lighting stuff
glm::vec3 lightPosition = glm::vec3(0.01f, 2.0f, 0.01f);
glm::vec3 lightDirection = glm::vec3(0.0f, 0.0f, 0.0f);
float texelMod = 3.0f;

unsigned int depthFBO;
unsigned int depthMap;

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
	GLFWwindow* window = initWindow("Assignment 3", screenWidth, screenHeight);

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

	ew::Shader lightOrbShader = ew::Shader("assets/lightOrb.vert", "assets/lightOrb.frag");

	ew::Shader shadowShader = ew::Shader("assets/depthShader.vert","assets/depthShader.frag");

	ew::Transform monkeyTransform;

	ew::Mesh plane = ew::createPlane(500,500,500);
	ew::Transform planeTransform;

	ew::Mesh lightOrbMesh = ew::createSphere(2,50);
	ew::Transform orbTransform;

	//Adding more lights
	struct PointLight 
	{
		glm::vec3 position;
		float radius;
		glm::vec4 color;
	};

	const int MAX_POINT_LIGHTS = 64;
	PointLight pointLights[MAX_POINT_LIGHTS];

	//plane
	float planeVertices[] =
	{
		//positions            //normals         //UVs
		 125.0f, -1.5f, -125.0f, 0.0f, 1.0f, 0.0f,  0.0f, 25.0f,
		-125.0f, -1.5f,  125.0f, 0.0f, 1.0f, 0.0f,  0.0f,  0.0f,
		 125.0f, -1.5f,  125.0f, 0.0f, 1.0f, 0.0f, 25.0f,  0.0f,

		 125.0f, -1.5f, -125.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
		-125.0f, -1.5f, -125.0f, 0.0f, 1.0f, 0.0f,  0.0f, 25.0f,
		-125.0f, -1.5f,  125.0f, 0.0f, 1.0f, 0.0f, 25.0f,  0.0f
	};

	unsigned int planeVBO, planeVAO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);


	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	createDeferredPass();
	createDisplayPass();

	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		//position
		int x, y, z;
		x = rand() % 10 + 1;
		y = rand() % 10 + 1;
		z = rand() % 10 + 1;

		pointLights[i].position = glm::vec3(x, y, z);

		//color
		float r, g, b;
		r = ((rand() % 101) / 100.0);
		g = ((rand() % 101) / 100.0);
		b = ((rand() % 101) / 100.0);

		pointLights[i].color = glm::vec4(r, g, b, 1.0);

		pointLights[i].radius = 10;
	}

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

		float near_plane = 1.0f, far_plane = 10.5f;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(
			glm::vec3(lightPosition),
			glm::vec3(lightDirection), //light direction
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		//RENDER
		//================================

		//geometry pass
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, deferred.fbo);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); //back face culling
		glEnable(GL_DEPTH_TEST); // Depth testing

		glClearColor(bg_rgba.red, bg_rgba.green, bg_rgba.blue, bg_rgba.alpha);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		


		glBindTextureUnit(0, goldTexture);

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
		glBindTextureUnit(0, brickTexture);
		geometryShader.setMat4("_Model", glm::mat4(1.0f));
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES,0,6);

		geometryShader.setVec3("_AmbientModifier", ambientModifier);

		glBindTextureUnit(1, brickTexture);

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

		lightOrbShader.use();
		lightOrbShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		for (int i = 0; i < MAX_POINT_LIGHTS; i++)
		{
			glm::mat4 m = glm::mat4(1.0f);
			m = glm::translate(m, pointLights[i].position);
			m = glm::scale(m, glm::vec3(0.2f));

			lightOrbShader.setMat4("_Model", m);
			lightOrbShader.setVec3("_Color", pointLights[i].color);
			lightOrbMesh.draw();
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

		lightingShader.setMat4("LightSpaceMatrix", lightSpaceMatrix);

		lightingShader.setInt("gPosition", 0);
		lightingShader.setInt("gNormals", 1);
		lightingShader.setInt("gAlbedo", 2);

		//Create Point Lights
		

		//Set Point Lights
		for (int i = 0; i < MAX_POINT_LIGHTS; i++)
		{
			std::string prefix = "_PointLights[" + std::to_string(i) + "].";
			lightingShader.setVec3(prefix + "position", pointLights[i].position);
			lightingShader.setVec3(prefix + "color", pointLights[i].color);
			lightingShader.setFloat(prefix + "radius", pointLights[i].radius);
		}

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
	ImGui::Text("Deferred Depth");
	ImGui::Image((ImTextureID)deferred.depth, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::Text("Depth");
	//ImGui::Image((ImTextureID)depthMap, windowSize, ImVec2(0, 1), ImVec2(1, 0));
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
