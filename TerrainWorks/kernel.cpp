
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
//#include "cuda_gl_interop.h"

#include <stdio.h>

#include <vector>
#include <fstream>
#include <string>
#include <iostream>

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

#include "Controls.h"
#include "Window.h"
#include "Camera.h"
#include "Terrain.h"
#include "CudaTools.h"
#include "Tools.h"

using namespace glm;

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

GLbyte scroll = 0;


int main()
{
	GLuint size = 512;
	int width = 1280;
	int height = 720;
	float FoV = 70.0f;
	float near = 0.1f;
	float far = 10000.0f;
	float H = 64.0f;
	mat4 projection = perspective(radians(FoV), (float)width / height, near, far);
	
	Camera camera(0.0f, 115.0f, 0.0f, 0.015f, 0.445f);
	camera.update();

	Window window(width, height);
	Controls::init(&window);

	Terrain terrain(size);
	CudaTools cudaTools(terrain);

	ToolElevate elevate(&cudaTools);
	ToolPlateau plateau(&cudaTools);
	ToolAveragize averagize(&cudaTools);
			
	Tool * current = &elevate;


	auto pickHeight = [&terrain, &projection, &camera, &width, &height, &H](float x, float y)
	{
		vec4 ray_eye =
			inverse(projection)
			* vec4(
				(2.0f * x) / width - 1.0f,
				1.0f - (2.0f * y) / height,
				-1.0,
				1.0);

		vec4 ray_wor = inverse(camera.matrix()) * vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
		vec3 ray = normalize(vec3(ray_wor.x, ray_wor.y, ray_wor.z));
		
		float distance = 128.0f;
		float outDistance = 0.0f;
		for (int i = 0; i < 32; i++)
		{
			vec3 v = camera.position() + ray*(distance+outDistance);
			if (v.y > terrain.getHeight(v.x, v.z)*H)
				outDistance += distance;
			distance /= 2;
		}
		vec3 v = camera.position() + ray*outDistance;
		return vec3(v.x, v.z, terrain.getHeight(v.x, v.z));
	};

	GLuint program[2];
	program[0] = LoadShaders("heightVertex.shader", "heightFragment.shader");
	program[1] = LoadShaders("waterVertex.shader", "waterFragment.shader");


	cudaTools.PerlinNoise(128, 2, 0.5, 2, 5);
	cudaTools.square();
	cudaTools.mapNormals();
	cudaTools.fetchHeight();
	
	vec3 target;

	bool isKPressed = false;

	glfwSetScrollCallback(window.ptr(), [](GLFWwindow *, double, double y) { scroll = y; });

	long long frame = 0;
	while (window.update())
	{
		Controls::update();
		camera.update();
		cudaTools.waterFlow(H);
		cudaTools.mapNormals();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		mat4 MVP = projection * camera.matrix();
		glUseProgram(program[0]);
		glUniformMatrix4fv(0, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(1, H);
		glUniform2f(2, target.x, target.y);
		if (current)
		{
			glUniform1f(3, current->outerRadius());
			glUniform1f(4, current->innerRadius());
		}

		terrain.drawTerrain();

		if (glfwGetKey(window.ptr(), GLFW_KEY_SPACE) == GLFW_RELEASE)
		{

			glUseProgram(program[1]);
			glUniformMatrix4fv(0, 1, GL_FALSE, &MVP[0][0]);
			glUniform1f(1, H);

			terrain.drawWater();
		}
		if (glfwGetKey(window.ptr(), GLFW_KEY_R) == GLFW_PRESS)
			cudaTools.addWater(0.005f);
		if (glfwGetKey(window.ptr(), GLFW_KEY_E) == GLFW_PRESS)
			cudaTools.addWater(-1000.0f);
		cudaTools.addWater(-0.0005f);





		target = pickHeight((float)Controls::x(), (float)Controls::y());

		if (glfwGetMouseButton(window.ptr(), GLFW_MOUSE_BUTTON_LEFT))
		{
			if (current)
				current->apply(target);
			else
				cudaTools.raindrop(vec2(target), 5, 2, 0.2f);
		}


		if (glfwGetKey(window.ptr(), GLFW_KEY_KP_ADD) == GLFW_PRESS)
			elevate.factorPositive();
		if (glfwGetKey(window.ptr(), GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
			elevate.factorNegative();



		if (glfwGetKey(window.ptr(), GLFW_KEY_1) == GLFW_PRESS)
			current = &elevate;

		if (glfwGetKey(window.ptr(), GLFW_KEY_2) == GLFW_PRESS)
			current = &plateau;

		if (glfwGetKey(window.ptr(), GLFW_KEY_3) == GLFW_PRESS)
			current = &averagize;

		if (glfwGetKey(window.ptr(), GLFW_KEY_4) == GLFW_PRESS)
			current = nullptr;

		GLubyte AltCtrl = (glfwGetKey(window.ptr(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS ? 0b00000001 : 0) | (glfwGetKey(window.ptr(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 0b00000010 : 0);

		if(current)
			switch (scroll)
			{
			case 1:
				if (!AltCtrl)
					current->increaseFactor();
				else
				{
					if (AltCtrl & 1)
						current->increaseInner();
					if (AltCtrl & 2)
						current->increaseOuter();
				}
				break;
			case -1:
				if (!AltCtrl)
					current->decreaseFactor();
				else
				{
					if (AltCtrl & 1)
						current->decreaseInner();
					if (AltCtrl & 2)
						current->decreaseOuter();
				}
			default:
				break;
			}
		scroll = 0;


		if (glfwGetKey(window.ptr(), GLFW_KEY_K) == GLFW_PRESS)
		{
			if (!isKPressed)
			{
				isKPressed = true;
				/*auto pos = camera.position();
				auto angX = camera.angleX();
				auto angY = camera.angleY();
				std::cout << pos.x << " " << pos.y << " " << pos.z << " " << angX << " " << angY << "\n";*/
				if (current)
					current->printFactor();
			}
		}
		else
			isKPressed = false;
	}

	glDeleteProgram(program[0]);
	glDeleteProgram(program[1]);

	return 0;
}

// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE333333333333333333333333333333333333333333333333333333333
GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);

	if (VertexShaderStream.is_open()) {

		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;

		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}
	// Read the Fragment Shader code from the file

	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);

	if (FragmentShaderStream.is_open()) {

		std::string Line = "";

		while (getline(FragmentShaderStream, Line))

			FragmentShaderCode += "\n" + Line;

		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();

	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);

	char const * FragmentSourcePointer = FragmentShaderCode.c_str();

	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;

}
