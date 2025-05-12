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

	private:
		auto LoadTexture() -> bool;

		void setProgram(std::string vPath, std::string fPath, int index);		// index: 選定該特效的program
		void updateSmoke(float Time);
		void updateModel(int);
	private:
		Camera camera;
	
		GLuint VAO;
		GLuint VBO;
		GLuint EBO;
		GLuint modelVBO;

		GLuint Effect_Texture[1];
		GLuint program[1];
		int effectsNum = 1;	// 特效數
		std::vector<std::string> enableEffects = { "enableSmoke" };

		struct EffectInform {
			std::vector<glm::mat4> effect_Model;		// 每個特效的 model matrix
			glm::vec3 trans;										// 位移量
			float alpha;												// 透明度
			float time;													// 持續時間
			bool firstAppear;
		};

		std::vector<EffectInform> EffectInforms;

		enum Enable
		{
			Smoke = 0
		};
	};
}