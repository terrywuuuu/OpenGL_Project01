#pragma once

#include <array>
#include <string>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

namespace CG
{
	class SkyBox
	{
	public:
		auto Initialize() -> bool;
		void Render(float camX, float camY, float camZ, float aspect, GLenum mode);

	private:
		auto LoadScene() -> bool;
		void LoadModel();
	private:
		Camera camera;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;
		GLuint UBO;

		GLuint cubemapTexture;

		GLuint program;

		GLint MatricesIdx;
	};
}