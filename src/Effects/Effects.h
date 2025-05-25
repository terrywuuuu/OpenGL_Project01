#pragma once

#include <array>
#include <string>
#include <map>
#include <vector>
/*#include <cstdlib> 
#include <ctime>*/
#include <random>
#include <deque>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Scene/Camera.h"

namespace CG
{
	class Effects
	{
	public:
		auto Initialize() -> bool;
		void renderEffects(bool, float camX, float camY, float camZ, float aspect, GLenum mode, std::string effects, float time, int index, int width, int height);		// index : 選定該特效的program
		void setupMesh();
		void setAngle(std::string effect, float angle);
		void effectInit();
		void fireBallInit();

	private:
		auto LoadTexture() -> bool;

		void setProgram(std::string vPath, std::string fPath, int index);		// index: 選定該特效的program
		void updateSmoke(float Time);
		void updateFireWork(float Time);
		void updateFireBall(float Time);
		void updateModel(int);
		void updateTrail();
		GLuint CreateSphereVAO();		// For fireBall
		glm::vec3 randomPointOnSphere(float radius);
	private:
		Camera camera;
	
		GLuint sVAO;
		GLuint sVBO;
		GLuint sEBO;
		GLuint fVAO;
		GLuint fVBO;
		GLuint bVAO;
		GLuint bVBO;
		GLuint bEBO;
		GLuint instanceVBO;
		GLuint sphereVAO;
		GLuint colorVBO;

		GLuint Effect_Texture[3];		// Texture's picture
		GLuint program[2];				// Effects program
		int effectsNum = 3;				// Number of effects
		std::vector<std::string> enableEffects = { "enableSmoke", "enableFireWorks", "enableTrail", "enableFireBall" };
		std::vector<int> effectCount = { 10, 300, 5000 };		// Number of particles of effects

		struct EffectInform {
			glm::mat4 Model;
			glm::vec3 trans;										// 位移量
			glm::vec3 velocity;
			float alpha;												// 透明度
			float time;													// 持續時間
			bool firstAppear;
			std::deque<glm::vec3> trailHistory;
			glm::vec3 color;
		};

		std::vector<std::vector<EffectInform>> EffectInforms;

		std::random_device rd;
		std::mt19937 gen;

		GLuint sphereIndexCount;
		std::vector<glm::vec3> positions;
		std::vector<unsigned int> indic;
		unsigned int X_SEGMENTS = 32;
		unsigned int Y_SEGMENTS = 16;

		std::vector<glm::mat4> trailVertices;
		std::vector<glm::vec3> trailColors;
		std::vector<GLsizei> trailVertexCounts;
		std::vector<GLsizei> baseVertexOffsets;

		int fireBallNum = 9;
		float Radius = 3.0f;
	};
}