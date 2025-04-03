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

constexpr auto PARTSNUM = 18;

namespace CG
{
	class MainScene
	{
	public:
		MainScene();
		~MainScene();

		auto Initialize() -> bool;
		void Update(double dt);
		void Render();

		void OnResize(int width, int height);
		void OnKeyboard(int key, int action);

		void ResetAction();
		void SetAction(int action);

		void SetMode(int mode);

	private:
		auto LoadScene() -> bool;

		void LoadModel();
		void Load2Buffer(const char* obj, int i);

		void UpdateAction(double dt);
		void UpdateModel();

	private:
		Camera camera;

		GLuint VAO;
		GLuint VBO;
		GLuint uVBO;
		GLuint nVBO;
		GLuint mVBO;
		GLuint UBO;
		std::array<GLuint, PARTSNUM> VBOs;
		std::array<GLuint, PARTSNUM> uVBOs;
		std::array<GLuint, PARTSNUM> nVBOs;
		GLuint program;

		int action = 0; // idle
		GLenum mode = 0; // fill

		float angles[PARTSNUM];
		float position = 0.0;
		float angle = 0.0;
		float eyeAngley = 0.0;
		float eyedistance = 20.0;
		float size = 1;
		GLfloat movex, movey;
		GLint MatricesIdx;
		GLuint ModelID;

		int vertices_size[PARTSNUM];
		int uvs_size[PARTSNUM];
		int normals_size[PARTSNUM];
		int materialCount[PARTSNUM];

		GLuint M_KaID;
		GLuint M_KdID;
		GLuint M_KsID;

		std::vector<std::string> mtls[PARTSNUM];//use material
		std::vector<unsigned int> faces[PARTSNUM];//face count
		std::map<std::string, glm::vec3> KDs;//mtl-name&Kd
		std::map<std::string, glm::vec3> KSs;//mtl-name&Ks

		glm::mat4 Model;
		glm::mat4 Models[PARTSNUM];
	};
}

