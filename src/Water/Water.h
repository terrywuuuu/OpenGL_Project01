#pragma once

#include <array>
#include <string>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "./WaterTile.h"
#include <Scene/Camera.h>

constexpr auto GRIDSIZE = 6;

namespace CG
{
	class Water
	{
	public:
		auto Initialize() -> bool;
		void Render(float camX, float camY, float camZ, float aspect, GLenum mode);

		float getHeight() const { return height; }
	private:
		auto LoadScene() -> bool;
	private:
		Camera camera;

		GLuint VAO;
		GLuint VBO;
		GLuint UBO;

		GLuint program;

		GLint MatricesIdx;
		GLuint ModelID;

		std::vector <glm::mat4> Models;

		std::vector<WaterTile> waterTiles;

		float height = 0.0f;
	};
}