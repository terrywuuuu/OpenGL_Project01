#pragma once

#include <array>
#include <string>
#include <map>
#include <vector>
/*#include <cstdlib> 
#include <ctime>*/
#include <random>

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
		void renderEffects(bool, float camX, float camY, float camZ, float aspect, GLenum mode, std::string effects, float time, int index);		// index : 選定該特效的program
		void setupMesh();
		void setAngle(std::string effect, float angle);
		void effectInit();

	private:
		auto LoadTexture() -> bool;

		void setProgram(std::string vPath, std::string fPath, int index);		// index: 選定該特效的program
		void updateSmoke(float Time);
		void updateFireWork(float Time);
		void updateModel(int);
	private:
		Camera camera;
	
		GLuint sVAO;
		GLuint sVBO;
		GLuint sEBO;
		GLuint fVAO;
		GLuint fVBO;
		GLuint instanceVBO;

		GLuint Effect_Texture[2];		// Texture's picture
		GLuint program[1];				// Effects program
		int effectsNum = 2;				// Number of effects
		std::vector<std::string> enableEffects = { "enableSmoke", "enableFireWorks"};
		std::vector<int> effectCount = { 10,1000 };		// Number of particles of effects

		struct EffectInform {
			glm::mat4 Model;
			glm::vec3 trans;										// 位移量
			glm::vec2 velocity;
			float alpha;												// 透明度
			float time;													// 持續時間
			bool firstAppear;
		};

		std::vector<std::vector<EffectInform>> EffectInforms;

		std::random_device rd;
		std::mt19937 gen;

		enum Enable
		{
			Smoke = 0
		};
	};
}