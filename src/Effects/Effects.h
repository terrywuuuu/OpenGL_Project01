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
		void renderEffects(bool, float camX, float camY, float camZ, float aspect, GLenum mode, std::string effects, float time);
		void setupMesh();
		void setProgram(GLuint program);

	private:
		auto LoadTexture() -> bool;

		void updateSmoke(float Time);
		void updateModel(int);
	private:
		Camera camera;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;
		GLuint modelVBO;

		GLuint Effect_Texture[1];
		GLuint program;
		std::vector<std::vector<glm::mat4>> effects_Model;
		int effectsNum = 1;
		std::vector<std::string> enableEffects = { "enableSmoke" };

		struct EffectInform {
			glm::vec3 trans;
			float alpha;
		};

		std::vector<EffectInform> EffectInforms;
		float times[1] = { 20.0f };

		enum Enable
		{
			Smoke = 0
		};
	};
}