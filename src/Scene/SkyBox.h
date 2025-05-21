#pragma once

#include <array>
#include <string>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"

namespace CG
{
	class SkyBox
	{
	public:
		auto Initialize() -> bool;
		void Render(float camX, float camY, float camZ, float aspect, GLenum mode, bool environmentMap, GLuint envCubemap, bool isEnv);

	private:
		auto LoadScene() -> bool;
		void LoadModel();
		void Load2Buffer(const char* obj, int i);
	private:
		Camera camera;

		GLuint VAO;
		GLuint BallVAO;
		GLuint VBO;
		GLuint EBO;
		GLuint UBO;
		std::array<GLuint, 1> VBOs;
		std::array<GLuint, 1> uVBOs;
		std::array<GLuint, 1> nVBOs;
		GLuint BallVBO;
		GLuint BalluVBO;
		GLuint BallnVBO;

		GLuint cubemapTexture;

		GLuint program;
		GLuint Ball_program;

		GLint MatricesIdx;

		GLuint ModelID;
		GLuint ProID, ViewID;
		glm::mat4 Model;

		std::vector<unsigned int> faces[1];//face count
		std::vector<std::string> mtls[1];//use material

		int vertices_size[1];
		int uvs_size[1];
		int normals_size[1];
	};
}