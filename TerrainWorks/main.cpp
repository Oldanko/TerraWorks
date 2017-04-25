#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "WindowManager.h"
#include "Camera.h"
#include "LandscapesCUDA.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cuda_gl_interop.h"

#include <glm\glm.hpp>

#define GLSL(src) "#version 150 core\n" #src


const int size = 512;

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

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




int main()
{
	WindowManager::init();
	cudaGLSetGLDevice(0);
	LandscapesCUDA::init(size);

	
	
	GLuint program[4];
	program[0] = LoadShaders("shader.vertex", "shader.fragment");
	program[1] = LoadShaders("waterShader.vertex", "waterShader.fragment");
	program[2] = LoadShaders("heightShader.vertex", "heightShader.fragment");
	program[3] = LoadShaders("tectonicPlatesShader.vertex", "tectonicPlatesShader.fragment");

	GLuint vao[3];
	

	glGenVertexArrays(3, vao);

	//Land Vertex Array
	glBindVertexArray(vao[0]);

	LandscapesCUDA::bindVboGrid();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	LandscapesCUDA::bindVboHeightmap();
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*)0);
	LandscapesCUDA::bindVboNormals();
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	LandscapesCUDA::bindEbo();

	glBindVertexArray(vao[1]);

	LandscapesCUDA::bindVboGrid();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	LandscapesCUDA::bindVboHeightmap();
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*)0);
	LandscapesCUDA::bindVboWatermap();
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*)0);
	LandscapesCUDA::bindVboWaterNormals();
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	LandscapesCUDA::bindEbo();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	LandscapesCUDA::PerlinNoise(257, 2, .5, 2, 8, 60);
	LandscapesCUDA::NormalMapping();

	Camera camera;

	long frameNum = 0;
	
	//LandscapesCUDA::rainfall(1);

	bool rain = true;

	bool isSaveKeyPressed = false;
	bool isLoadKeyPressed = false;
	bool isSmoothingKeyPressed = false;

	do
	{
		LandscapesCUDA::HydraulicErosion(1, 0.0005);
		
		LandscapesCUDA::NormalMapping();
		LandscapesCUDA::WaterNormalMapping();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glm::mat4 MVP = WindowManager::projectionMatrix() * camera.ViewMatrix();

		glUseProgram(program[0]);
		glUniformMatrix4fv(glGetUniformLocation(program[1], "MVP"), 1, GL_FALSE, &MVP[0][0]);

		glBindVertexArray(vao[0]);
		glDrawElements(GL_TRIANGLES, (size - 1)*(size - 1) * 6, GL_UNSIGNED_INT, 0);

		if (glfwGetKey(WindowManager::window(), GLFW_KEY_SPACE) == GLFW_RELEASE)
		{
			glUseProgram(program[1]);
			glUniformMatrix4fv(glGetUniformLocation(program[1], "MVP"), 1, GL_FALSE, &MVP[0][0]);

			glBindVertexArray(vao[1]);
			glDrawElements(GL_TRIANGLES, (size - 1)*(size - 1) * 6, GL_UNSIGNED_INT, 0);
		}

		if (glfwGetKey(WindowManager::window(), GLFW_KEY_R) == GLFW_PRESS)
			LandscapesCUDA::rainfall(0.5f);
		if (glfwGetKey(WindowManager::window(), GLFW_KEY_T) == GLFW_PRESS)
			LandscapesCUDA::rainfall(-0.001f);



		if (glfwGetKey(WindowManager::window(), GLFW_KEY_O) == GLFW_PRESS)
		{
			if (!isSaveKeyPressed)
			{
				std::cout << "Saving heightmap\n";
				LandscapesCUDA::saveHeightMap("heightmap.hm");
				isSaveKeyPressed = true;
				std::cout << "Heightmap saved\n";
			}
		}
		else
			isSaveKeyPressed = false;

		if (glfwGetKey(WindowManager::window(), GLFW_KEY_L) == GLFW_PRESS)
		{
			if (!isLoadKeyPressed)
			{
				std::cout << "Loading heightmap\n";
				LandscapesCUDA::loadHeightMap("heightmap.hm");
				isLoadKeyPressed = true;
				std::cout << "Heightmap loaded or not\n";
			}
		}
		else
			isLoadKeyPressed = false;

		Controls::update();
		camera.update();
		frameNum++;
	} while (WindowManager::update());

	LandscapesCUDA::close();

	glDeleteProgram(program[0]);
	glDeleteProgram(program[1]);
	glDeleteProgram(program[2]);
	glDeleteProgram(program[3]);
	glDeleteVertexArrays(3, vao);

	return 0;
}
