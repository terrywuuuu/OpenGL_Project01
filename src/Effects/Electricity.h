#pragma once

#include <array>
#include <string>
#include <map>
#include <vector>
#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Scene/Camera.h"

namespace CG
{
	class Electricity
	{
	public:
		auto Initialize() -> bool;
		void Render(float camX, float camY, float camZ, float aspect, GLenum mode);
		void Update(double dt);
	private:
		auto LoadScene() -> bool;
	private:
		Camera camera;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;
		GLuint UBO;

		GLuint program;

		GLint MatricesIdx;
		GLuint ModelID;
		GLuint aspectID;
		GLuint ElectricityTextureID;
		GLuint ElectricityBallTextureID;
		GLuint isBallID;
		GLuint rowsID;
		GLuint colsID;
		GLuint nowFrameID;
		GLuint totalFramesID;
		GLuint instanceCountID;

		GLuint ElectricityTexture;
		GLuint ElectricityBallTexture;

		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 BallModel = glm::mat4(1.0f);

		float frame = 0;
		const int rows = 2;
		const int cols = 4;
		const int totalFrames = 6;
		const int instanceCount = 15;

		float ballFrame = 0;
		const int ballRows = 2;
		const int ballCols = 3;
		const int ballTotalFrames = 6;


		int ElectricitySize = 12;
		int ElectricityBallSize = 4;
	};
}