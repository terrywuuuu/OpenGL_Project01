#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>
#include <../src/Utilty/JsonIO.h>

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
		isActionChange = true;
		return LoadScene();
	}

	void MainScene::Update(double dt)
	{
		UpdateAction(dt);
		UpdateModel();
	}

	void MainScene::Render(float aspect)
	{
		glClearColor(0.0, 0.0, 0.0, 1); //black screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, mode);// mode = 0, fill

		glBindVertexArray(VAO);
		glUseProgram(program);//uniform參數數值前必須先use shader

		float eyey = glm::radians(eyeAngley);
		camera.LookAt(
			glm::vec3(eyedistance * sin(eyey), 2, eyedistance * cos(eyey)), // Camera is at (0,0,20), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		camera.SetAspect(aspect);

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
				if (instancedNum == 1) {
					glDrawArrays(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3);
				}
				else {
					if (i == PARTSNUM - 1) {
						glUniform1i(BackGround, 0);
					}
					else {
						glUniform1i(BackGround, 1);
					}

					glDrawArraysInstanced(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3, instancedNum);
				}
				//we draw triangles by giving the glVertexID base and vertex count is face count*3
				vertexIDoffset += faces[i][j + 1] * 3;//glVertexID's base offset is face count*3
			}//end for loop for draw one part of the robot	

		}//end for loop for updating and drawing model
		glFlush();
	}

	void MainScene::OnResize(int width, int height)
	{
		std::cout << "MainScene Resize: " << width << " " << height << std::endl;

		// avoid divid 0
		if (height == 0) height = 1;

		// set new view port
		glViewport(0, 0, width, height);

		// calc aspect and update camera
		float aspect = static_cast<float>(width) / static_cast<float>(height);
		camera.SetAspect(aspect);
	}

	void MainScene::OnKeyboard(int key, int action)
	{
		std::cout << "MainScene OnKeyboard: " << key << " " << action << std::endl;

		// changed GLFW_RELEASE to GLFW_REPEAT for continuous key events when key is held down
		if (action == GLFW_REPEAT || action == GLFW_RELEASE)
		{
			switch (key)
			{
			case GLFW_KEY_Q:
				angle += 5;
				if (angle >= 360) angle = 0;
				printf("beta:%f\n", angle);
				break;
			case GLFW_KEY_E:
				angle -= 5;
				if (angle <= 0) angle = 360;
				printf("beta:%f\n", angle);
				break;
			case GLFW_KEY_W:
				eyedistance -= 0.5;
				break;
			case GLFW_KEY_S:
				eyedistance += 0.5;
				break;
			case GLFW_KEY_A:
				eyeAngley -= 10;
				break;
			case GLFW_KEY_D:
				eyeAngley += 10;
				break;
			}
		}
	}

	void MainScene::SetAction(int action)
	{
		this->action = action;
		isActionChange = true;
		instancedNum = 1;
		for (int i = 0; i < PARTSNUM; i++) //reset model pos
		{
			alphas[i] = 0.0f;
			betas[i] = 0.0f;
			gammas[i] = 0.0f;
		}
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

	void MainScene::SetRotate(int bodyPart, float alpha, float beta, float gamma)
	{
		alphas[bodyPart] = alpha;
		betas[bodyPart] = beta;
		gammas[bodyPart] = gamma;
	}

	void MainScene::SetPosition(int axis, float position)
	{
		this->position[axis] = position;
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
			{ GL_VERTEX_SHADER, "../../res/shaders/DSPhong_Material.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/DSPhong_Material.fp" },//fragment shader
			{ GL_NONE, NULL } };
		program = LoadShaders(shaders); //讀取shader

		glUseProgram(program);//uniform參數數值前必須先use shader

		MatricesIdx = glGetUniformBlockIndex(program, "MatVP");
		ModelID = glGetUniformLocation(program, "Model");
		M_KaID = glGetUniformLocation(program, "Material.Ka");
		M_KdID = glGetUniformLocation(program, "Material.Kd");
		M_KsID = glGetUniformLocation(program, "Material.Ks");
		BackGround = glGetUniformLocation(program, "isInstanced");

		// Camera matrix
		//camera.LookAt(glm::vec3(0, 10, 25), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

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
		std::vector<std::string> Materials; // mtl-name
		std::string texture;
		LoadMTL("../../res/Parts/material.mtl", Kds, Kas, Kss, Materials, texture);
		for (int i = 0; i < Materials.size(); i++)
		{
			std::string mtlname = Materials[i];
			KDs[mtlname] = Kds[i];
		}

		// 加載各部件
		Load2Buffer("../../res/Parts/body.obj", Body::body);           // body
		Load2Buffer("../../res/Parts/left_arm.obj", Body::left_arm);      // upper left arm
		Load2Buffer("../../res/Parts/left_hand.obj", Body::left_hand);       // down left arm
		Load2Buffer("../../res/Parts/head.obj", Body::head);           // head
		Load2Buffer("../../res/Parts/right_arm.obj", Body::right_arm);      // upper right arm
		Load2Buffer("../../res/Parts/right_hand.obj", Body::right_hand);      // down right arm
		Load2Buffer("../../res/Parts/left_leg.obj", Body::left_leg);        // upperleftleg
		Load2Buffer("../../res/Parts/left_foot.obj", Body::left_foot);       // downleftleg
		Load2Buffer("../../res/Parts/right_leg.obj", Body::right_leg);       // uprightleg
		Load2Buffer("../../res/Parts/right_foot.obj", Body::right_foot);      // downrightleg
		//Load2Buffer("../../res/Parts/Tree.obj", 10);		// BackGround

		GLuint totalSize[3] = { 0, 0, 0 };
		GLuint offset[3] = { 0, 0, 0 };
		for (int i = 0; i < PARTSNUM; i++)
		{
			totalSize[0] += vertices_size[i] * sizeof(glm::vec3);
			totalSize[1] += uvs_size[i] * sizeof(glm::vec2);
			totalSize[2] += normals_size[i] * sizeof(glm::vec3);
		}

		// 生成 VBO
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &uVBO);
		glGenBuffers(1, &nVBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, totalSize[0], NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, uVBO);
		glBufferData(GL_ARRAY_BUFFER, totalSize[1], NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, nVBO);
		glBufferData(GL_ARRAY_BUFFER, totalSize[2], NULL, GL_STATIC_DRAW);

		for (int i = 0; i < PARTSNUM; i++)
		{
			// 複製頂點資料
			glBindBuffer(GL_COPY_WRITE_BUFFER, VBO);
			glBindBuffer(GL_COPY_READ_BUFFER, VBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[0], vertices_size[i] * sizeof(glm::vec3));
			offset[0] += vertices_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(VBOs[i]); // 釋放 VBO
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			// 複製 UV 資料
			glBindBuffer(GL_COPY_WRITE_BUFFER, uVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, uVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[1], uvs_size[i] * sizeof(glm::vec2));
			offset[1] += uvs_size[i] * sizeof(glm::vec2);
			glInvalidateBufferData(uVBOs[i]); // 釋放 VBO
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			// 複製法線資料
			glBindBuffer(GL_COPY_WRITE_BUFFER, nVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, nVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[2], normals_size[i] * sizeof(glm::vec3));
			offset[2] += normals_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(nVBOs[i]); // 釋放 VBO
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
		static double _frame = 0.0;
		static short isMultiple = 0;
		std::vector<JsonIO::FrameData> frameData;
		int end = 0;
		if (isActionChange) {
			isActionChange = false;
			_frame = 0;
		}

		if (action == Action::idle)
		{
			if (JsonIO::LoadFrames("../../res/actions/idle.json", frameData)) {
				end = frameData.size();
				HandleAction(frameData, _frame, dt);
			}
		}
		else if (action == Action::walk)
		{
			if (JsonIO::LoadFrames("../../res/actions/walk.json", frameData))
			{
				end = frameData.size();
				HandleAction(frameData, _frame, dt);
			}
		}
		else if (action == Action::sit_up) {
			if (JsonIO::LoadFrames("../../res/actions/sit_up.json", frameData)) {
				end = frameData.size();
				HandleAction(frameData, _frame, dt);
			}
		}
		else if (action == Action::push_up) {
			if (JsonIO::LoadFrames("../../res/actions/push_up.json", frameData)) {
				end = frameData.size();
				HandleAction(frameData, _frame, dt);
			}
		}
		else if (action == Action::multiple) {
			if (instancedNum == 1 && JsonIO::LoadFrames("../../res/actions/multiple.json", frameData)) {
				end = frameData.size();
				HandleAction(frameData, _frame, dt);
			}
			if (_frame >= end-1) {
				instancedNum = 50;
			}
		}
		else if (action == Action::hopak_dance) {
			if (JsonIO::LoadFrames("../../res/actions/hopak_dance.json", frameData)) {
				end = frameData.size();
				HandleAction(frameData, _frame, dt);
			}
		}
		_frame += dt;
		if (_frame > end)
			_frame = 0;
	}

	glm::mat4 MainScene::bodyRotateMatrix(int body)
	{
		glm::mat4 M = glm::mat4(1.0f);
		M = rotate(alphas[body], 1, 0, 0) * rotate(betas[body], 0, 1, 0) * rotate(gammas[body], 0, 0, 1);
		return M;
	}

	void MainScene::UpdateModel()
	{
		glm::mat4 Translation[PARTSNUM];

		Translation[Body::body] = translate(position[Axis::x], -2.9f + position[Axis::y], position[Axis::z]);
		Models[Body::body] = Translation[Body::body] * bodyRotateMatrix(Body::body);

		Translation[Body::head] = translate(0, 12.0f, 0);
		Models[Body::head] = Models[Body::body] * Translation[Body::head] * bodyRotateMatrix(Body::head);

		Translation[Body::left_arm] = translate(3.5f, 11.0f, -1.0f);
		Models[Body::left_arm] = Models[Body::body] * Translation[Body::left_arm] * bodyRotateMatrix(Body::left_arm);

		Translation[Body::left_hand] = translate(4.8f, -0.8f, 0);
		Models[Body::left_hand] = Models[Body::left_arm] * Translation[Body::left_hand] * bodyRotateMatrix(Body::left_hand);

		Translation[Body::right_arm] = translate(-3.5f, 11.0f, -0.5f);

		Models[Body::right_arm] = Models[Body::body] * Translation[Body::right_arm] * bodyRotateMatrix(Body::right_arm);

		Translation[Body::right_hand] = translate(-4.8, -0.8f, 0);
		Models[Body::right_hand] = Models[Body::right_arm] * Translation[Body::right_hand] * bodyRotateMatrix(Body::right_hand);

		Translation[Body::left_leg] = translate(1.5f, 0.5f, -1.0);
		Models[Body::left_leg] = Models[Body::body] * Translation[Body::left_leg] * bodyRotateMatrix(Body::left_leg);

		Translation[Body::left_foot] = translate(1.0, -7.0f, 0);
		Models[Body::left_foot] = Models[Body::left_leg] * Translation[Body::left_foot] * bodyRotateMatrix(Body::left_foot);

		Translation[Body::right_leg] = translate(-1.0f, 0.5f, -1.0);
		Models[Body::right_leg] = Models[Body::body] * Translation[Body::right_leg] * bodyRotateMatrix(Body::right_leg);

		Translation[Body::right_foot] = translate(-1.0, -7.0f, 0);
		Models[Body::right_foot] = Models[Body::right_leg] * Translation[Body::right_foot] * bodyRotateMatrix(Body::right_foot);
	}

	void MainScene::HandleAction(std::vector<JsonIO::FrameData>& frameDatas, double frame, double dt) {
		JsonIO::FrameData curFD = frameDatas[frame], perFD;
		if (frame == 0) {
			for (int i = 0; i < 3; ++i) {
				position[i] = curFD.position[i];
			}
			for (int i = 0; i < PARTSNUM; i++) {
				alphas[i] = curFD.partRotations[i].alpha;
				betas[i] = curFD.partRotations[i].beta;
				gammas[i] = curFD.partRotations[i].gamma;
			}
		}
		else {
			perFD = frame >= frameDatas.size() - 1 ? frameDatas[0] : frameDatas[frame + 1];
			for (int i = 0; i < 3; ++i) {
				position[i] += (perFD.position[i] - curFD.position[i]) * dt;
			}
			for (int i = 0; i < PARTSNUM; i++) {
				alphas[i] += (perFD.partRotations[i].alpha - curFD.partRotations[i].alpha) * dt;
				betas[i] += (perFD.partRotations[i].beta - curFD.partRotations[i].beta) * dt;
				gammas[i] += (perFD.partRotations[i].gamma - curFD.partRotations[i].gamma) * dt;
			}
		}
	}
}