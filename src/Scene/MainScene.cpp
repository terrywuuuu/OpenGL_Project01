#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>

#include "MainScene.h"

static glm::mat4 translate(float x, float y, float z)
{
	glm::vec4 t = glm::vec4(x, y, z, 1);//w = 1 ,則x,y,z=0時也能translate
	glm::vec4 c1 = glm::vec4(1, 0, 0, 0);
	glm::vec4 c2 = glm::vec4(0, 1, 0, 0);
	glm::vec4 c3 = glm::vec4(0, 0, 1, 0);
	glm::mat4 M = glm::mat4(c1, c2, c3, t);
	return M;
}
static glm::mat4 scale(float x, float y, float z)
{
	glm::vec4 c1 = glm::vec4(x, 0, 0, 0);
	glm::vec4 c2 = glm::vec4(0, y, 0, 0);
	glm::vec4 c3 = glm::vec4(0, 0, z, 0);
	glm::vec4 c4 = glm::vec4(0, 0, 0, 1);
	glm::mat4 M = glm::mat4(c1, c2, c3, c4);
	return M;
}

static glm::mat4 rotate(float angle, float x, float y, float z)
{
	float r = glm::radians(angle);
	glm::mat4 M = glm::mat4(1);

	glm::vec4 c1 = glm::vec4(cos(r) + (1 - cos(r)) * x * x, (1 - cos(r)) * y * x + sin(r) * z, (1 - cos(r)) * z * x - sin(r) * y, 0);
	glm::vec4 c2 = glm::vec4((1 - cos(r)) * y * x - sin(r) * z, cos(r) + (1 - cos(r)) * y * y, (1 - cos(r)) * z * y + sin(r) * x, 0);
	glm::vec4 c3 = glm::vec4((1 - cos(r)) * z * x + sin(r) * y, (1 - cos(r)) * z * y - sin(r) * x, cos(r) + (1 - cos(r)) * z * z, 0);
	glm::vec4 c4 = glm::vec4(0, 0, 0, 1);
	M = glm::mat4(c1, c2, c3, c4);
	return M;
}

namespace CG
{
	MainScene::MainScene()
	{
	}

	MainScene::~MainScene()
	{
	}

	auto MainScene::Initialize() -> bool
	{
		return LoadScene();
	}

	void MainScene::Update(double dt)
	{
		UpdateAction(dt);
		UpdateModel();
	}

	void MainScene::Render()
	{
		glClearColor(0.0, 0.0, 0.0, 1); //black screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, mode);

		glBindVertexArray(VAO);
		glUseProgram(program);//uniform參數數值前必須先use shader

		float eyey = glm::radians(eyeAngley);
		camera.LookAt(
			glm::vec3(eyedistance * sin(eyey), 2, eyedistance * cos(eyey)), // Camera is at (0,0,20), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		//update data to UBO for MVP
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &camera.GetViewMatrix()[0][0]);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &camera.GetProjectionMatrix()[0][0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		GLuint offset[3] = { 0,0,0 };//offset for vertices , uvs , normals
		for (int i = 0; i < PARTSNUM; i++)
		{
			glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Models[i][0][0]);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0,				//location
				3,				//vec3
				GL_FLOAT,			//type
				GL_FALSE,			//not normalized
				0,				//strip
				(void*)offset[0]);//buffer offset
			//(location,vec3,type,固定點,連續點的偏移量,buffer point)
			offset[0] += vertices_size[i] * sizeof(glm::vec3);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);//location 1 :vec2 UV
			glBindBuffer(GL_ARRAY_BUFFER, uVBO);
			glVertexAttribPointer(1,
				2,
				GL_FLOAT,
				GL_FALSE,
				0,
				(void*)offset[1]);
			//(location,vec2,type,固定點,連續點的偏移量,point)
			offset[1] += uvs_size[i] * sizeof(glm::vec2);

			// 3rd attribute buffer : normals
			glEnableVertexAttribArray(2);//location 2 :vec3 Normal
			glBindBuffer(GL_ARRAY_BUFFER, nVBO);
			glVertexAttribPointer(2,
				3,
				GL_FLOAT,
				GL_FALSE,
				0,
				(void*)offset[2]);
			//(location,vec3,type,固定點,連續點的偏移量,point)
			offset[2] += normals_size[i] * sizeof(glm::vec3);

			int vertexIDoffset = 0;//glVertexID's offset 
			std::string mtlname;//material name
			glm::vec3 Ks = glm::vec3(1, 1, 1);//because .mtl excluding specular , so give it here.
			for (int j = 0; j < mtls[i].size(); j++)
			{
				mtlname = mtls[i][j];
				//find the material diffuse color in map:KDs by material name.
				glUniform3fv(M_KdID, 1, &KDs[mtlname][0]);
				glUniform3fv(M_KsID, 1, &Ks[0]);
				//          (primitive   , glVertexID base , vertex count    )
				glDrawArrays(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3);
				//we draw triangles by giving the glVertexID base and vertex count is face count*3
				vertexIDoffset += faces[i][j + 1] * 3;//glVertexID's base offset is face count*3
			}//end for loop for draw one part of the robot	

		}//end for loop for updating and drawing model
		glFlush();
	}

	void MainScene::OnResize(int width, int height)
	{
		std::cout << "MainScene Resize: " << width << " " << height << std::endl;
	}

	void MainScene::OnKeyboard(int key, int action)
	{
		std::cout << "MainScene OnKeyboard: " << key << " " << action << std::endl;

		if (action == GLFW_RELEASE)
		{
			switch (key)
			{
			case GLFW_KEY_1:
				angle += 5;
				if (angle >= 360) angle = 0;
				printf("beta:%f\n", angle);
				break;
			case GLFW_KEY_2:
				angle -= 5;
				if (angle <= 0) angle = 360;
				printf("beta:%f\n", angle);
				break;
			case GLFW_KEY_W:
				eyedistance -= 0.2;
				break;
			case GLFW_KEY_S:
				eyedistance += 0.2;
				break;
			case GLFW_KEY_A:
				eyeAngley -= 10;
				break;
			case GLFW_KEY_D:
				eyeAngley += 10;
				break;
			case GLFW_KEY_R:
				angles[1] -= 5;
				if (angles[1] == -360) angles[1] = 0;
				movey = 0;
				movex = 0;
				break;
			case GLFW_KEY_T:
				angles[2] -= 5;
				if (angles[2] == -360) angles[2] = 0;
				movey = 0;
				movex = 0;
				break;
			}
		}
	}

	void MainScene::ResetAction()
	{
		this->action = 0; // idle
	}

	void MainScene::SetAction(int action)
	{
		this->action = action;
	}

	void MainScene::SetMode(int mode)
	{
		switch (mode)
		{
		case 0:
			this->mode = GL_FILL;
			break;
		case 1:
			this->mode = GL_LINE;
			break;
		default:
			this->mode = GL_FILL;
			break;
		}
	}

	auto MainScene::LoadScene() -> bool
	{
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		//VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		ShaderInfo shaders[] = {
			{ GL_VERTEX_SHADER, "res/shaders/DSPhong_Material.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "res/shaders/DSPhong_Material.fp" },//fragment shader
			{ GL_NONE, NULL } };
		program = LoadShaders(shaders); //讀取shader

		glUseProgram(program);//uniform參數數值前必須先use shader

		MatricesIdx = glGetUniformBlockIndex(program, "MatVP");
		ModelID = glGetUniformLocation(program, "Model");
		M_KaID = glGetUniformLocation(program, "Material.Ka");
		M_KdID = glGetUniformLocation(program, "Material.Kd");
		M_KsID = glGetUniformLocation(program, "Material.Ks");

		// Camera matrix
		camera.LookAt(glm::vec3(0, 10, 25), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		LoadModel();

		//UBO
		glGenBuffers(1, &UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_DYNAMIC_DRAW);
		//get uniform struct size
		int UBOsize = 0;
		glGetActiveUniformBlockiv(program, MatricesIdx, GL_UNIFORM_BLOCK_DATA_SIZE, &UBOsize);
		//bind UBO to its idx
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, UBOsize);
		glUniformBlockBinding(program, MatricesIdx, 0);

		return true;
	}

	void MainScene::LoadModel()
	{
		std::vector<glm::vec3> Kds;
		std::vector<glm::vec3> Kas;
		std::vector<glm::vec3> Kss;
		std::vector<std::string> Materials;//mtl-name
		std::string texture;
		LoadMTL("res/Parts/material.mtl", Kds, Kas, Kss, Materials, texture);
		for (int i = 0; i < Materials.size(); i++)
		{
			std::string mtlname = Materials[i];
			//  name       vec3
			KDs[mtlname] = Kds[i];
		}

		Load2Buffer("res/Parts/body.obj", 0);

		Load2Buffer("res/Parts/left_arm.obj", 1);
		Load2Buffer("res/Parts/left_hand.obj", 2);
		Load2Buffer("res/models/lefthand.obj", 3);
		Load2Buffer("res/models/lshouder.obj", 4);

		Load2Buffer("res/Parts/head.obj", 5);

		Load2Buffer("res/Parts/right_arm.obj", 6);
		Load2Buffer("res/Parts/right_hand.obj", 7);
		Load2Buffer("res/models/righthand.obj", 8);
		Load2Buffer("res/models/rshouder.obj", 9);

		Load2Buffer("res/models/dbody.obj", 11);
		Load2Buffer("res/models/back2.obj", 10);

		Load2Buffer("res/Parts/left_leg.obj", 12);
		Load2Buffer("res/Parts/left_foot.obj", 13);
		Load2Buffer("res/models/leftfoot.obj", 14);

		Load2Buffer("res/Parts/right_leg.obj", 15);
		Load2Buffer("res/Parts/right_foot.obj", 16);
		Load2Buffer("res/models/rightfoot.obj", 17);

		GLuint totalSize[3] = { 0,0,0 };
		GLuint offset[3] = { 0,0,0 };
		for (int i = 0; i < PARTSNUM; i++)
		{
			totalSize[0] += vertices_size[i] * sizeof(glm::vec3);
			totalSize[1] += uvs_size[i] * sizeof(glm::vec2);
			totalSize[2] += normals_size[i] * sizeof(glm::vec3);
		}

		//generate vbo
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &uVBO);
		glGenBuffers(1, &nVBO);
		//bind vbo ,第一次bind也同等於 create vbo 
		glBindBuffer(GL_ARRAY_BUFFER, VBO);//VBO的target是GL_ARRAY_BUFFER
		glBufferData(GL_ARRAY_BUFFER, totalSize[0], NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, uVBO);//VBO的target是GL_ARRAY_BUFFER
		glBufferData(GL_ARRAY_BUFFER, totalSize[1], NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, nVBO);//VBO的target是GL_ARRAY_BUFFER
		glBufferData(GL_ARRAY_BUFFER, totalSize[2], NULL, GL_STATIC_DRAW);

		for (int i = 0; i < PARTSNUM; i++)
		{
			glBindBuffer(GL_COPY_WRITE_BUFFER, VBO);
			glBindBuffer(GL_COPY_READ_BUFFER, VBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset[0], vertices_size[i] * sizeof(glm::vec3));
			offset[0] += vertices_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(VBOs[i]);//free vbo
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			glBindBuffer(GL_COPY_WRITE_BUFFER, uVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, uVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset[1], uvs_size[i] * sizeof(glm::vec2));
			offset[1] += uvs_size[i] * sizeof(glm::vec2);
			glInvalidateBufferData(uVBOs[i]);//free vbo
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			glBindBuffer(GL_COPY_WRITE_BUFFER, nVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, nVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset[2], normals_size[i] * sizeof(glm::vec3));
			offset[2] += normals_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(uVBOs[i]);//free vbo
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		}
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	}

	void MainScene::Load2Buffer(const char* obj, int i)
	{
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals; // Won't be used at the moment.
		std::vector<unsigned int> materialIndices;

		bool res = LoadOBJ(obj, vertices, uvs, normals, faces[i], mtls[i]);
		if (!res) printf("load failed\n");

		glGenBuffers(1, &VBOs[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		vertices_size[i] = vertices.size();

		glGenBuffers(1, &uVBOs[i]);
		glBindBuffer(GL_ARRAY_BUFFER, uVBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
		uvs_size[i] = uvs.size();

		glGenBuffers(1, &nVBOs[i]);
		glBindBuffer(GL_ARRAY_BUFFER, nVBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
		normals_size[i] = normals.size();
	}

	void MainScene::UpdateAction(double dt)
	{
		static double _frame = 0;

		if (action == 0)
		{
			_frame = 0;
			for (int i = 0; i < PARTSNUM; i++)
			{
				angles[i] = 0.0f;
			}
			position = 0;
		}
		else if (action == 1)
		{
			_frame += dt;

			if (_frame > 13)
			{
				_frame = 0;
			}

			int frame = _frame;

			switch (frame) 
			{
			case 0:
				//左手
				angles[2] = -45 * dt;
				//右手

				//腿
				angles[13] = 45 * dt;

				break;
			case 1:
			case 2:
			case 3:
				angles[1] += 10 * dt;
				angles[12] -= 15 * dt;
				position += 0.1 * dt;
				break;
			case 4:
			case 5:
			case 6:
				angles[1] -= 10 * dt;
				angles[12] += 15 * dt;
				angles[13] -= 15 * dt;
				position -= 0.1 * dt;
				break;
			case 7:
			case 8:
			case 9:
				angles[1] -= 10 * dt;
				angles[12] += 15 * dt;
				angles[13] = 0;
				position += 0.1 * dt;
				break;
			case 10:
			case 11:
			case 12:
				angles[1] += 10 * dt;
				angles[12] -= 15 * dt;
				angles[13] += 15 * dt;
				position -= 0.1 * dt;
				break;
			}
		}
	}

	void MainScene::UpdateModel()
	{
		glm::mat4 Rotatation[PARTSNUM];
		glm::mat4 Translation[PARTSNUM];
		glm::mat4 Scale[PARTSNUM];
		for (int i = 0; i < PARTSNUM; i++)
		{
			Models[i] = glm::mat4(1.0f);
			Rotatation[i] = glm::mat4(1.0f);
			Translation[i] = glm::mat4(1.0f);
		}
		float r, pitch, yaw, roll;
		float alpha, beta, gamma;

		//Body
		beta = angle;
		Rotatation[0] = rotate(beta, 0, 1, 0);
		Translation[0] = translate(0, 2.9 + position, 0);
		Scale[0] = scale(5.0, 5.0, 5.0);
		Models[0] = Translation[0] * Rotatation[0];
		//左手=======================================================
		//左上手臂
		yaw = glm::radians(beta); r = 3.7;
		alpha = angles[1];
		gamma = 10;
		Rotatation[1] = rotate(alpha, 1, 0, 0) * rotate(gamma, 0, 0, 1);//向前旋轉*向右旋轉
		Translation[1] = translate(3.7, 1, -0.5);

		Models[1] = Models[0] * Translation[1] * Rotatation[1];

		//左肩膀
		Rotatation[4] = rotate(alpha, 1, 0, 0) * rotate(gamma, 0, 0, 1);//向前旋轉*向右旋轉
		Translation[4] = translate(3.7, 1, -0.5);//位移到左上手臂處
		Models[4] = Models[0] * Translation[1] * Rotatation[1];

		//左下手臂
		pitch = glm::radians(alpha); r = 3;
		roll = glm::radians(gamma);
		static int i = 0;
		i += 5;
		alpha = angles[2] - 20;
		//上手臂+下手臂向前旋轉*向右旋轉
		Rotatation[2] = rotate(alpha, 1, 0, 0);
		//延x軸位移以上手臂為半徑的圓周長:translate(0,r*cos,r*sin)
		//延z軸位移以上手臂為半徑角度:translate(r*sin,-rcos,0)
		Translation[2] = translate(0, -3, 0);

		Models[2] = Models[1] * Translation[2] * Rotatation[2];


		pitch = glm::radians(alpha);
		//b = glm::radians(angles[2]);
		roll = glm::radians(gamma);
		//手掌角度與下手臂相同
		//Rotatation[3] = Rotatation[2];
		//延x軸位移以上手臂為半徑的圓周長:translate(0,r*cos,r*sin) ,角度為上手臂+下手臂
		Translation[3] = translate(0, -4.8, 0);
		Models[3] = Models[2] * Translation[3] * Rotatation[3];
		//============================================================
		//頭==========================================================
		Translation[5] = translate(0, 3.9, -0.5);
		Models[5] = Models[0] * Translation[5] * Rotatation[5];
		//============================================================
		//右手=========================================================
		gamma = -10; alpha = angles[6] = -angles[1];
		Rotatation[6] = rotate(alpha, 1, 0, 0) * rotate(gamma, 0, 0, 1);
		Translation[6] = translate(-3.9, 1.7, -0.2);
		Models[6] = Models[0] * Translation[6] * Rotatation[6];

		Rotatation[9] = rotate(alpha, 1, 0, 0) * rotate(gamma, 0, 0, 1);
		Translation[9] = translate(-3.9, 1.1, -0.2);
		Models[9] = Models[0] * Translation[9] * Rotatation[9];

		angles[7] = angles[2];
		pitch = glm::radians(alpha); r = -3;
		roll = glm::radians(gamma);
		alpha = angles[7] - 20;
		Rotatation[7] = rotate(alpha, 1, 0, 0);
		Translation[7] = translate(0, -3, 0);
		Models[7] = Models[6] * Translation[7] * Rotatation[7];

		pitch = glm::radians(alpha);
		//b = glm::radians(angles[7]);
		roll = glm::radians(gamma);
		Translation[8] = translate(0, -6, 0);
		Models[8] = Models[7] * Translation[8] * Rotatation[8];
		//=============================================================
		//back&DBody===================================================
		Translation[10] = translate(0, 2, -4.5);
		Models[10] = Models[0] * Translation[10] * Rotatation[10];

		Translation[11] = translate(0, -5.3, 0);
		Models[11] = Models[0] * Translation[11] * Rotatation[11];
		//=============================================================
		//左腳
		alpha = angles[12]; gamma = 10;
		Rotatation[12] = rotate(alpha, 1, 0, 0) * rotate(gamma, 0, 0, 1);
		Translation[12] = translate(1.8, -4.5, 0);
		Models[12] = Translation[12] * Rotatation[12] * Models[12];

		pitch = glm::radians(alpha); r = -7;
		roll = glm::radians(gamma);
		alpha = angles[13] + angles[12];
		Translation[13] = translate(-r * sin(roll), r * cos(pitch), r * sin(pitch)) * Translation[12];
		Rotatation[13] = rotate(alpha, 1, 0, 0);
		Models[13] = Translation[13] * Rotatation[13] * Models[13];

		pitch = glm::radians(alpha); r = -5;
		//b = glm::radians(angles[13]);
		roll = glm::radians(gamma);
		Translation[14] = translate(-(r + 2) * sin(roll), r * cos(pitch), r * sin(pitch) - 1) * Translation[13];
		Rotatation[14] = rotate(alpha, 1, 0, 0);
		Models[14] = Translation[14] * Rotatation[14] * Models[14];
		//=============================================================
		//右腳
		alpha = angles[15] = -angles[12];
		gamma = -10;
		Rotatation[15] = rotate(alpha, 1, 0, 0) * rotate(gamma, 0, 0, 1);
		Translation[15] = translate(-1.8, -4.5, 0);
		Models[15] = Translation[15] * Rotatation[15] * Models[15];

		angles[16] = angles[13];
		pitch = glm::radians(alpha); r = -7;
		roll = glm::radians(gamma);
		alpha = angles[16] + angles[15];
		Rotatation[16] = rotate(alpha, 1, 0, 0);
		Translation[16] = translate(-r * sin(roll), r * cos(pitch), r * sin(pitch)) * Translation[15];
		Models[16] = Translation[16] * Rotatation[16] * Models[16];

		pitch = glm::radians(alpha); r = -5;
		//b = glm::radians(angles[16]);
		roll = glm::radians(gamma);
		alpha = angles[15] + angles[16];
		Translation[17] = translate(-(r + 2) * sin(roll), r * cos(pitch), r * sin(pitch) - 0.5) * Translation[16];
		Rotatation[17] = rotate(alpha, 1, 0, 0);
		Models[17] = Translation[17] * Rotatation[17] * Models[17];
		//=============================================================
	}
}