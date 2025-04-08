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

constexpr auto PARTSNUM = 10;
//old
// 0:body	1:ulefthand	2:dlefthand	3:lefthand
// 4:lshouder	5:head	6:urighthand	7:drighthand
// 8:righthand	9:rshouder	10:back2	11:dbody
// 12:uleftleg	13:dleftleg	14:leftfoot	15:urightleg
// 16:drightleg	17:rightfoot

//new
// 0:body	1:ulefthand	2:dlefthand	3:head
// 4:urighthand	5:drighthand	6:uleftleg	7:dleftleg
// 8:urightleg	9:drightleg

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
		void SetRotate(int bodyPart,float alpha, float beta, float gamma);

	private:
		auto LoadScene() -> bool;

		void LoadModel();
		void Load2Buffer(const char* obj, int i);

		void UpdateAction(double dt);
		void UpdateModel();
		glm::mat4 bodyRotateMatrix(int body);
		
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

		float position = 0.0;
		float angle = 0.0;
		float eyeAngley = 0.0;
		float eyedistance = 20.0;
		float size = 1;
		GLfloat movex, movey;
		GLint MatricesIdx;
		GLuint ModelID;
		int instancedNum = 1;  // 決定畫幾個機器人

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

		float alphas[PARTSNUM];
		float betas[PARTSNUM];
		float gammas[PARTSNUM];
		bool isActionChange;
		
		// Six Action
		void Walk(int, double);
		void PushUp(int, double);
		void SitUp(int, double);
		void HoPak(int, double);
		void APT(int, double);
		void Multiple(int, double);

		enum Body
		{
			body = 0,
			left_arm,
			left_hand,
			head,
			right_arm,
			right_hand,
			left_leg,
			left_foot,
			right_leg,
			right_foot
		};

		enum Action
		{
			idle = 0,
			walk,
			lay_face_up,
			lay_face_down,
			multiple
		};
	};
}

