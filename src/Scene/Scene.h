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

constexpr auto SCENESUM = 1;

namespace CG
{
	struct Texture {
		GLuint texture;
		bool hasTexture = true;
	};

	class Scene
	{
	public:
		auto Initialize() -> bool;
		void Update(double dt);
		void Render(float camX, float camY, float camZ, float aspect, GLenum mode);

	private:
		auto LoadScene() -> bool;

		void LoadModel();
		void Load2Buffer(const char* obj, int i);
	private:
		Camera camera;

		GLuint VAO;
		GLuint VBO;
		GLuint uVBO;
		GLuint nVBO;
		GLuint mVBO;
		GLuint UBO;
		std::array<GLuint, SCENESUM> VBOs;
		std::array<GLuint, SCENESUM> uVBOs;
		std::array<GLuint, SCENESUM> nVBOs;
		GLuint program;

		GLint MatricesIdx;
		GLuint ModelID;

		int vertices_size[SCENESUM];
		int uvs_size[SCENESUM];
		int normals_size[SCENESUM];
		int materialCount[SCENESUM];

		GLuint M_KaID;
		GLuint M_KdID;
		GLuint M_KsID;
		GLuint BackGround;


		std::vector<std::string> mtls[SCENESUM];//use material
		std::vector<unsigned int> faces[SCENESUM];//face count
		std::vector<std::string> Mtls;//mtl-name
		std::map<std::string, glm::vec3> KDs;//mtl-name&Kd
		std::map<std::string, glm::vec3> KSs;//mtl-name&Ks
		std::map<std::string, glm::vec3> KAs;//mtl-name&Ka
		std::map<std::string, Texture> Textures;//mtl-name&texture

		glm::mat4 Model;
		glm::mat4 Models[SCENESUM] = { glm::mat4(1.0f) };

		enum Type
		{
			Building = 0
		};
	};
}