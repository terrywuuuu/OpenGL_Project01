#pragma once

#include <cmath>
#include <array>
#include <string>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "./WaterTile.h"
#include "./WaterFrameBuffer.h"
#include <Scene/Camera.h>

constexpr auto GRIDSIZE = 1;
constexpr float WAVESPEED = 0.08f;

namespace CG
{
	class Water
	{
	public:
		auto Initialize(WaterFrameBuffer& waterFrameBuffer) -> bool;
		void Render(float camX, float camY, float camZ, float aspect, GLenum mode, glm::vec3 LightPos, bool enableWave, bool enableLightReflection);
		void Update(double dt);
		float getHeight() const { return height; }
	private:
		auto LoadScene() -> bool;
	private:
		Camera camera;

		WaterFrameBuffer *waterFrameBuffer;

		GLuint VAO;
		GLuint VBO;
		GLuint UBO;

		GLuint program;

		GLint MatricesIdx;
		GLuint ModelID;
		GLuint ReflectionTextureID;
		GLuint RefractionTextureID;
		GLuint dudvMapID;
		GLuint normalID;
		GLuint moveFactorID;
		GLuint cameraPositionID;
		GLuint lightPosID;
		GLuint enableWaveID;
		GLuint enableLightReflectionID;

		std::vector <glm::mat4> Models;

		std::vector<WaterTile> waterTiles;

		GLuint dudvMapTexture;
		GLuint normalMapTexture;

		float height = -10.0f;
		float moveFactor = 0;

	};
}