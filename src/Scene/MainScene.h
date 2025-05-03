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
#include "Scene.h"
#include "SkyBox.h"
<<<<<<< HEAD
#include "Effects/Effects.h"
=======
#include "MusicPlayer.h"
>>>>>>> db8ed3d3e6a82772d969f0b5a2e54d05426c23d4

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
		void Texture_Render();

		void OnResize(int width, int height);
		void OnKeyboard(int key);

		void PlayMusic();

		// Control panel functions
		void SetMode(int mode);
		void SetMultipleNumber(int num);
		void SetkeepMultipleActive(bool keepMultipleActive);
		void SetMultipleMode(int multipleMode);

		// Action editor panel functions
		void SetAction(int action);
		void SetSpeed(float speed);
		void SetEdit(bool isEdit);
		void SetFrame(int frame);
		void SetCurFrameData(JsonIO::FrameData curFD, int frame); // update curAction
		void SetNewFrameData(JsonIO::FrameData frameData, int frame, bool isAdd); // add a new frame to curAction
		void SaveAction(std::string fileName);
		// send to ControlWindow
		JsonIO::FrameData GetFrameData();
		JsonIO::Action GetAction();
		std::vector<std::string> GetActionNames();

		// Effect panel functions
		void SetEffect(float num, int effect, bool isActive);
		void SetMtl(int partsNum, std::string material);
		// Initialize texture and framebuffer
		void SetTexture();
		void CreateScreenQuad();

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
		Scene *scene;
		SkyBox *skyBox;
<<<<<<< HEAD
		Effects* effect;
=======
		MusicPlayer* musicPlayer;
>>>>>>> db8ed3d3e6a82772d969f0b5a2e54d05426c23d4

		GLuint VAO;
		GLuint VBO;
		GLuint uVBO;
		GLuint nVBO;
		GLuint mVBO;
		GLuint UBO;
		std::array<GLuint, PARTSNUM> VBOs;
		std::array<GLuint, PARTSNUM> uVBOs;
		std::array<GLuint, PARTSNUM> nVBOs;
		GLuint screenQuadVAO, screenQuadVBO;
		GLuint FBO;
		GLuint texture;
		GLuint depth_texture;
		GLuint program;
		GLuint Post_Process;		// ¯S®ÄªºProgram

		int actionIndex = 0; // idle
		GLenum mode = 0; // fill

		float eyeX = 0.0;
		float angle = 0.0;
		float eyeAngley = 0.0;
		float eyedistance = 65.0;
		float size = 1;
		GLfloat movex, movey;
		GLint MatricesIdx;
		GLuint ModelID;

		bool keepMultipleActive = false;
		int multipleMode = 0;
		int curInstancedNum = 1;
		int instancedNum = 100;

		int vertices_size[PARTSNUM];
		int uvs_size[PARTSNUM];
		int normals_size[PARTSNUM];
		int materialCount[PARTSNUM];

		GLuint M_KaID;
		GLuint M_KdID;
		GLuint M_KsID;
		GLuint BackGround;
		GLuint MultipleMode;

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
		bool isEdit = false;

		std::vector<JsonIO::Action> actionDatas;
		JsonIO::Action curAction;
		
		double frame;
    
		int screenWidth = 1280;
		int screenHeight = 720;

		bool enableBlur = false;
		bool enableQuan = false;
		float blurStrength;
		float quanStrength;
		std::map<std::string, float> effectTime;

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

		enum Axis {
			x = 0,
			y = 1,
			z = 2
		};
	};
}

