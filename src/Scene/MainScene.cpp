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
		return LoadScene();
	}

	void MainScene::Update(double dt)
	{
		UpdateAction(dt);
		UpdateModel();
	}

	void MainScene::Render(float aspect)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glClearColor(0.0, 0.0, 0.0, 1); //black screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, mode);// mode = 0, fill

		glBindVertexArray(VAO);
		glUseProgram(program);//uniform參數數值前必須先use shader

		/*
		float eyey = glm::radians(eyeAngley);
		camera.LookAt(
			glm::vec3(eyedistance * sin(eyey), 2, eyedistance * cos(eyey)), // Camera is at (0,0,20), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		*/

		float theta = glm::radians(eyeAngley); // 左右
		float phi = glm::radians(angle);   // 上下

		float camX = eyedistance * cos(phi) * sin(theta);
		float camY = eyedistance * sin(phi);
		float camZ = eyedistance * cos(phi) * cos(theta);

		camera.LookAt(
			glm::vec3(camX, camY, camZ),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
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

			for (int j = 0; j < mtls[i].size(); j++)
			{
				mtlname = mtls[i][j];
				//find the material diffuse color in map:KDs by material name.
				glUniform3fv(M_KdID, 1, &KDs[mtlname][0]);
				glUniform3fv(M_KsID, 1, &KSs[mtlname][0]);
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
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Texture_Render();

		glFlush();
	}

	void MainScene::Texture_Render() {
		// 顯示渲染結果並應用模糊
		glUseProgram(Post_Process);  // 使用另一個 program
		glBindVertexArray(screenQuadVAO);  // 綁定四邊形 VAO
		glClear(GL_COLOR_BUFFER_BIT); // 這裡只清 color，不清 depth
		glDisable(GL_DEPTH_TEST); // 關掉深度測試

		// 傳遞 FBO 渲染結果的紋理和紋理大小
		glActiveTexture(GL_TEXTURE0);  // 激活紋理單元
		glBindTexture(GL_TEXTURE_2D, texture);  // 綁定場景渲染的紋理
		glUniform1i(glGetUniformLocation(Post_Process, "sceneTexture"), 0);  // 傳遞紋理到 shader
		glUniform2f(glGetUniformLocation(Post_Process, "texSize"), screenWidth, screenHeight);  // 傳遞紋理大小到 shader
		glUniform1f(glGetUniformLocation(Post_Process, "blurStrength"), blurStrength);
		glUniform1f(glGetUniformLocation(Post_Process, "quanStrength"), quanStrength);
		glUniform1i(glGetUniformLocation(Post_Process, "enableBlur"), enableBlur);
		glUniform1i(glGetUniformLocation(Post_Process, "enableQuan"), enableQuan);

		// 渲染屏幕四邊形顯示結果
		glDrawArrays(GL_TRIANGLES, 0, 6);  // 渲染四邊形*/
<<<<<<< HEAD
		glEnable(GL_DEPTH_TEST); // 重新啟用深度測試
		glFlush();
=======
		glEnable(GL_DEPTH_TEST);
>>>>>>> main
	}

	void MainScene::OnResize(int width, int height)
	{
		std::cout << "MainScene Resize: " << width << " " << height << std::endl;

		// avoid divid 0
		if (height == 0) height = 1;

		// set new view port
		glViewport(0, 0, width, height);

		screenWidth = width;
		screenHeight = height;
		SetTexture();

		// calc aspect and update camera
		float aspect = static_cast<float>(width) / static_cast<float>(height);
		camera.SetAspect(aspect);
		SetTexture();
	}

	void MainScene::OnKeyboard(int key)
	{
		//0: key "a" press
		//1: key "d" press
		//2: key "w" press
		//3: key "s" press
		//4: Mouse wheel up
		//5: Mouse wheel down
		switch (key)
		{
		case 0:
			eyeAngley -= 10;
			break;
		case 1:
			eyeAngley += 10;
			break;
		case 2:
			angle += 3;
			if (angle >= 90) angle = 89;
			printf("beta:%f\n", angle);
			break;
		case 3:
			angle -= 3;
			if (angle <= -90) angle = -89;
			printf("beta:%f\n", angle);
			break;
		case 4:
			eyedistance -= 2.0;
			break;
		case 5:
			eyedistance += 2.0;
			break;
		}
	}

	void MainScene::SetAction(int action)
	{
		curAction = actionDatas[action];
		this->actionIndex = action;
		instancedNum = 1;
		frame = 0;
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

	void MainScene::SetSpeed(float speed)
	{
		this->speed = speed;
	}

	void MainScene::SetRotate(int bodyPart, float alpha, float beta, float gamma)
	{
		curAction.FDs[frame].partRotations[bodyPart].alpha = alpha;
		curAction.FDs[frame].partRotations[bodyPart].beta = beta;
		curAction.FDs[frame].partRotations[bodyPart].gamma = gamma;
	}

	//todo delete this func
	void MainScene::SetPosition(int axis, float position)
	{
		curAction.FDs[frame].position[axis] = position;
	}

	void MainScene::SetMtl(int partsNum, std::string material)
	{
		std::string mtlname;//material name

		if (material == "Matte") {
			for (int i = 0; i < mtls[partsNum].size(); i++) {
				mtlname = mtls[partsNum][i];
				glm::vec3 ks = glm::vec3(0.1, 0.1, 0.1);
				KSs[mtlname] = ks;
				glm::vec3 kd = glm::vec3(0.8, 0.8, 0.8);
				KDs[mtlname] = kd;
			}
		}
		else if (material == "Metal") {
			for (int i = 0; i < mtls[partsNum].size(); i++) {
				mtlname = mtls[partsNum][i];
				glm::vec3 ks = glm::vec3(2.0, 2.0, 2.0);
				KSs[mtlname] = ks;
				glm::vec3 kd = glm::vec3(0.8, 0.8, 0.8);
				KDs[mtlname] = kd;
			}
		}
		else if (material == "Dark") {
			for (int i = 0; i < mtls[partsNum].size(); i++) {
				mtlname = mtls[partsNum][i];
				glm::vec3 ks = glm::vec3(0.1, 0.1, 0.1);
				KSs[mtlname] = ks;
				glm::vec3 kd = glm::vec3(0.6, 0.6, 0.6);
				KDs[mtlname] = kd;
			}
		}
	}

	void MainScene::SetEffect(float num, int effect, bool isActive) {
		switch (effect) {
		case 0:
			enableBlur = isActive;
			blurStrength = num;
			break;
		case 1:
			enableQuan = isActive;
			quanStrength = num;
			break;
		}
	}

	void MainScene::SetTexture() {
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		glGenTextures(1, &depth_texture);
		glBindTexture(GL_TEXTURE_2D, depth_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Framebuffer not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void MainScene::SetEdit(bool isEdit,int mode) {
		this->isEdit = isEdit;
		if (mode == 0) {//edit cur action

		}
		else {// new action

		}
	}

	void MainScene::SetFrame(int frame) {
		this->frame = frame;
	}

	void MainScene::SetFrameData(JsonIO::FrameData frameData, int frame,bool isNewFD=0)
	{
		if (isNewFD) {
			curAction.FDs.insert(curAction.FDs.begin() + frame, frameData);
		}
		else {
			actionDatas[actionIndex] = curAction;
			JsonIO::SaveFrames("../../res/actions/action.json", actionDatas[actionIndex]);
		}
		this->frame = frame;
		curAction.FDs[frame] = frameData;
	}

	JsonIO::Action MainScene::GetAction()
	{
		return curAction;
	}

	JsonIO::FrameData MainScene::GetFrameData()
	{
		JsonIO::FrameData fd;
		fd.frame = frame;
		fd.isKeyFrame = curAction.FDs[frame].isKeyFrame;
		for (int i = 0; i < 3; ++i) {
			fd.position[i] = curAction.FDs[frame].position[i];
		}
		for (int i = 0; i < PARTSNUM-1/*without tree*/ ; i++) {
			fd.partRotations[i].alpha = curAction.FDs[frame].partRotations[i].alpha;
			fd.partRotations[i].beta = curAction.FDs[frame].partRotations[i].beta;
			fd.partRotations[i].gamma = curAction.FDs[frame].partRotations[i].gamma;
		}
		return fd;
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

		ShaderInfo shader[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/Post-Process.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/Post-Process.fp" },//fragment shader
			{ GL_NONE, NULL } };
		Post_Process = LoadShaders(shader); //讀取shader

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

		LoadAction();
		SetTexture();
		CreateScreenQuad();

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
		Load2Buffer("../../res/Parts/Tree.obj", 10);		// BackGround

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

	void MainScene::LoadAction()
	{
		JsonIO::Action actionData;
		if (JsonIO::LoadFrames("../../res/actions/idle.json", actionData)) {
			actionDatas.push_back(JsonIO::Action(actionData));
		}
		if (JsonIO::LoadFrames("../../res/actions/walk.json", actionData)) {
			actionDatas.push_back(JsonIO::Action(actionData));
		}
		if (JsonIO::LoadFrames("../../res/actions/sit_up.json", actionData)) {
			actionDatas.push_back(JsonIO::Action(actionData));
		}
		if (JsonIO::LoadFrames("../../res/actions/push_up.json", actionData)) {
			actionDatas.push_back(JsonIO::Action(actionData));
		}
		if (JsonIO::LoadFrames("../../res/actions/multiple.json", actionData)) {
			actionDatas.push_back(JsonIO::Action(actionData));
		}
		if (JsonIO::LoadFrames("../../res/actions/hopak_dance.json", actionData)) {
			actionDatas.push_back(JsonIO::Action(actionData));
		}
		if (JsonIO::LoadFrames("../../res/actions/apt.json", actionData)) {
			actionDatas.push_back(JsonIO::Action(actionData));
		}
		SetAction(Action::idle);
	}

	void MainScene::UpdateAction(double dt)
	{
		const JsonIO::Action& act = curAction;
		const size_t end = act.FDs.size();

		dt *= isEdit ? 0.0 : speed;//todo act.speed

		if (isActionChange) {
			isActionChange = false;
		}
		if (actionIndex == Action::multiple)
		{
			if (frame >= end - 1)
				instancedNum = 100;
		}
		else
			instancedNum = 1;
		if (instancedNum == 1) {
			HandleAction(act.FDs, frame, dt);
		}

		frame += dt;
		if (frame > end) {
			frame = 0.0;
		}
	}

	glm::mat4 MainScene::bodyRotateMatrix(int body)
	{
		glm::mat4 M = glm::mat4(1.0f);
		M = rotate(alphas[body], 1, 0, 0) * rotate(betas[body], 0, 1, 0) * rotate(gammas[body], 0, 0, 1);
		return M;
	}

	void MainScene::UpdateModel()
	{
		for (int i = 0; i < PARTSNUM; i++)
		{
			Models[i] = glm::mat4(1.0f);
		}

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

	void MainScene::HandleAction(const std::vector<JsonIO::FrameData>& frameDatas, double frame, double dt) {
		JsonIO::FrameData curFD = frameDatas[frame], perFD;
		if (frame == 0 || isEdit) {
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

	void MainScene::CreateScreenQuad()
	{
		GLfloat quadVertices[] = {
			-1.0f,  1.0f,  0.0f, 1.0f, // 左上
			-1.0f, -1.0f,  0.0f, 0.0f, // 左下
			1.0f, -1.0f,  1.0f, 0.0f, // 右下

			-1.0f,  1.0f,  0.0f, 1.0f, // 左上
			1.0f, -1.0f,  1.0f, 0.0f, // 右下
			1.0f,  1.0f,  1.0f, 1.0f  // 右上
		};

		glGenVertexArrays(1, &screenQuadVAO);
		glGenBuffers(1, &screenQuadVBO);
		glBindVertexArray(screenQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// texcoord attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
}