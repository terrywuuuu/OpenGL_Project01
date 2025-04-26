#pragma once

#include <array>
#include <string>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Utilty/JsonIO.h"
#include "Camera.h"
constexpr auto PARTSNUM = 11;
//new
// 0:body	1:ulefthand	2:dlefthand	3:head
// 4:urighthand	5:drighthand	6:uleftleg	7:dleftleg
// 8:urightleg	9:drightleg
// 10:background

namespace CG
{
	class MainScene
	{
	public:
		MainScene();
		~MainScene();

		auto Initialize() -> bool;
		void Update(double dt);
		void Render(float aspect);

		void OnResize(int width, int height);
		void OnKeyboard(int key);

		void SetAction(int action);

		void SetMode(int mode);
		void SetRotate(int bodyPart,float alpha, float beta, float gamma);
		void SetPosition(int axis, float position);
		void SetMtl(int partsNum, std::string material);
		void SetSpeed(float speed);
		void SetEdit(bool isEdit,int mode);
		void SetFrame(int frame);
		
		void SetActionData(JsonIO::Action actionData, int actionIndex);
		void SetFrameData(JsonIO::FrameData frameData, int frame);

		JsonIO::FrameData GetFrameData();
		JsonIO::Action GetAction();
		double GetFrame() { return frame; }

	private:
		auto LoadScene() -> bool;

		void LoadModel();
		void Load2Buffer(const char* obj, int i);
		void LoadAction();

		void UpdateAction(double dt);
		void UpdateModel();
		glm::mat4 bodyRotateMatrix(int body);

		void HandleAction(const std::vector<JsonIO::FrameData>&, double, double);
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

		float eyeX = 0.0;
		float angle = 0.0;
		float eyeAngley = 0.0;
		float eyedistance = 25.0;
		float size = 1;
		GLfloat movex, movey;
		GLint MatricesIdx;
		GLuint ModelID;
		int instancedNum = 1;  // How many robot

		int vertices_size[PARTSNUM];
		int uvs_size[PARTSNUM];
		int normals_size[PARTSNUM];
		int materialCount[PARTSNUM];

		GLuint M_KaID;
		GLuint M_KdID;
		GLuint M_KsID;
		GLuint BackGround;

		std::vector<std::string> mtls[PARTSNUM];//use material
		std::vector<unsigned int> faces[PARTSNUM];//face count
		std::map<std::string, glm::vec3> KDs;//mtl-name&Kd
		std::map<std::string, glm::vec3> KSs;//mtl-name&Ks

		glm::mat4 Model;
		glm::mat4 Models[PARTSNUM];

		float alphas[PARTSNUM];
		float betas[PARTSNUM];
		float gammas[PARTSNUM];
		float position[3];
		bool isActionChange;
		bool isEdit = false;
		std::vector<JsonIO::Action> actionDatas;
		JsonIO::Action curAction;
		
		//control speed
		float speed = 1;
		double frame;

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
			sit_up,
			push_up,
			multiple,
			hopak_dance,
			apt
		};

		enum Axis {
			x = 0,
			y = 1,
			z = 2
		};
	};
}

