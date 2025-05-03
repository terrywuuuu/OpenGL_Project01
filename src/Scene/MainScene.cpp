#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>
#include <io.h>

#include "MainScene.h"

static glm::mat4 translate(float x, float y, float z)
{
	glm::vec4 t = glm::vec4(x, y, z, 1);//w = 1 ,?áx,y,z=0?Ç‰??Ωtranslate
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
		//Initialize Scene, SkyBox
		scene = new Scene();
		skyBox = new SkyBox();
		effect = new Effects();
		
		scene->Initialize();
		skyBox->Initialize();
		effect->Initialize();

		//Initialize MusicPlayer
		musicPlayer = new MusicPlayer();
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
		glUseProgram(program);

		float theta = glm::radians(eyeAngley); // Â∑¶Âè≥
		float phi = glm::radians(angle);   // ‰∏ä‰?

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
			//(location,vec3,type,?∫Â?Èª????ÈªûÁ??èÁßª??buffer point)
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
			//(location,vec2,type,?∫Â?Èª????ÈªûÁ??èÁßª??point)
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
			//(location,vec3,type,?∫Â?Èª????ÈªûÁ??èÁßª??point)
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
				if (curInstancedNum == 1) {
					glDrawArrays(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3);
				}
				else {
					if (i == PARTSNUM - 1) {
						glUniform1i(BackGround, 0);
					}
					else {
						glUniform1i(BackGround, 1);
					}

					glUniform1i(MultipleMode, multipleMode);
					glDrawArraysInstanced(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3, curInstancedNum);
				}
				//we draw triangles by giving the glVertexID base and vertex count is face count*3
				vertexIDoffset += faces[i][j + 1] * 3;//glVertexID's base offset is face count*3
			}//end for loop for draw one part of the robot	

		}//end for loop for updating and drawing model

		scene->Render(camX, camY, camZ, aspect, mode);
		skyBox->Render(camX, camY, camZ, aspect, mode);

		if (effectTime["smoke"] != 0) {
			effect->renderEffects(true, camX, camY, camZ, aspect, mode, "smoke", effectTime["smoke"]);
			effectTime["smoke"]--;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Texture_Render();

		glFlush();
	}

	void MainScene::Texture_Render() {
		// È°ØÁ§∫Ê∏≤Ê?ÁµêÊ?‰∏¶Ê??®Ê®°Á≥?
		glUseProgram(Post_Process);  // ‰ΩøÁî®?¶‰???program
		glBindVertexArray(screenQuadVAO);  // Á∂ÅÂ??õÈ?ÂΩ?VAO
		glClear(GL_COLOR_BUFFER_BIT); // ?ôË£°?™Ê? colorÔºå‰?Ê∏?depth
		glDisable(GL_DEPTH_TEST); // ?úÊ?Ê∑±Â∫¶Ê∏¨Ë©¶

		// ?≥È? FBO Ê∏≤Ê?ÁµêÊ??ÑÁ??ÜÂ?Á¥ãÁ?Â§ßÂ?
		glActiveTexture(GL_TEXTURE0);  // ÊøÄÊ¥ªÁ??ÜÂñÆ??
		glBindTexture(GL_TEXTURE_2D, texture);  // Á∂ÅÂ??¥ÊôØÊ∏≤Ê??ÑÁ???
		glUniform1i(glGetUniformLocation(Post_Process, "useMVP"), false);
		glUniform1i(glGetUniformLocation(Post_Process, "sceneTexture"), 0);  // ?≥È?Á¥ãÁ???shader
		glUniform2f(glGetUniformLocation(Post_Process, "texSize"), screenWidth, screenHeight);  // ?≥È?Á¥ãÁ?Â§ßÂ???shader
		glUniform1f(glGetUniformLocation(Post_Process, "blurStrength"), blurStrength);
		glUniform1f(glGetUniformLocation(Post_Process, "quanStrength"), quanStrength);
		glUniform1i(glGetUniformLocation(Post_Process, "enableBlur"), enableBlur);
		glUniform1i(glGetUniformLocation(Post_Process, "enableQuan"), enableQuan);

		// ¥Ë¨V´Ãπı•|√‰ßŒ≈„•‹µ≤™G
		glDrawArrays(GL_TRIANGLES, 0, 6);  // ¥Ë¨V•|√‰ßŒ*/
		glEnable(GL_DEPTH_TEST);
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
			printf("eyedistance:%f\n", eyedistance);
			break;

		case 5:
			eyedistance += 2.0;
			printf("eyedistance:%f\n", eyedistance);
			break;
		}
	}

	void MainScene::PlayMusic() {
		musicPlayer->Play("../../res/Music/" + curAction.musicName);
		if (isEdit || curAction.name == "idle") {
			musicPlayer->Stop();
		}
		else {
			if (curAction.name == "multiple")
			{
				musicPlayer->SetLooping(false);
			}
			else {
				musicPlayer->SetLooping(true);
			}
		}
	}

	void MainScene::SetAction(int action)
	{
		curAction = actionDatas[action];
		this->actionIndex = action;

		if (curAction.name == "multiple" || !this->keepMultipleActive)
		{
			curInstancedNum = 1;
		}
		scene->SetInstance(curInstancedNum, multipleMode);

		frame = 0.0;
		PlayMusic();
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

	void MainScene::SetMultipleNumber(int num) {
		this->instancedNum = num;
	}

	void MainScene::SetkeepMultipleActive(bool keepMultipleActive) {
		this->keepMultipleActive = keepMultipleActive;
	}

	void MainScene::SetMultipleMode(int multipleMode) {
		this->multipleMode = multipleMode;
	}

	void MainScene::SetEdit(bool isEdit) {
		this->isEdit = isEdit;
		PlayMusic();
	}

	void MainScene::SetFrame(int frame) {
		this->frame = frame;
	}

	void MainScene::SetSpeed(float speed)
	{
		curAction.speed = speed;
	}

	void MainScene::SetNewFrameData(JsonIO::FrameData frameData, int frame, bool isAdd)
	{
		if (isAdd) {
			curAction.FDs.insert(curAction.FDs.begin() + frame, frameData);
			this->frame = frame + 1;
		}
		else {
			if (curAction.FDs.size() <= 1) {
				std::cout << "Cannot delete the last frame!" << std::endl;
				return;
			}
			curAction.FDs.erase(curAction.FDs.begin() + frame);
			this->frame = this->frame <= 0 ? 0 : this->frame - 1;
		}

		for (int i = this->frame; i < curAction.FDs.size(); ++i) {
			curAction.FDs[i].frame = i;
		}
	}

	void MainScene::SetCurFrameData(JsonIO::FrameData curFD, int frame)
	{
		this->curAction.FDs[frame] = curFD;
	}

	void MainScene::SaveAction(std::string fileName) {
		curAction.name = fileName;
		JsonIO::SaveAction("../../res/actions/" + fileName, curAction);
		LoadAction();
	}

	JsonIO::Action MainScene::GetAction()
	{
		return curAction;
	}

	JsonIO::FrameData MainScene::GetFrameData()
	{
		return curAction.FDs[frame];
	}

	std::vector<std::string> MainScene::GetActionNames()
	{
		std::vector<std::string> actionNames;
		for (int i = 0; i < actionDatas.size(); i++)
		{
			actionNames.push_back(actionDatas[i].name);
		}
		return actionNames;
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
		program = LoadShaders(shaders); //ËÆÄ?ñshader

		ShaderInfo shader[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/Post-Process.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/Post-Process.fp" },//fragment shader
			{ GL_NONE, NULL } };
		Post_Process = LoadShaders(shader); //ËÆÄ?ñshader

		effect->setProgram(Post_Process);
    
		glUseProgram(program);//uniform?ÉÊï∏?∏ÂÄºÂ?ÂøÖÈ??àuse shader

		MatricesIdx = glGetUniformBlockIndex(program, "MatVP");
		ModelID = glGetUniformLocation(program, "Model");
		M_KaID = glGetUniformLocation(program, "Material.Ka");
		M_KdID = glGetUniformLocation(program, "Material.Kd");
		M_KsID = glGetUniformLocation(program, "Material.Ks");
		BackGround = glGetUniformLocation(program, "isInstanced");
		MultipleMode = glGetUniformLocation(program, "MultipleMode");

		// Camera matrix

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

		effectTime["smoke"] = 0;

		return true;
	}

	void MainScene::LoadModel()
	{
		std::vector<glm::vec3> Kds;
		std::vector<glm::vec3> Kas;
		std::vector<glm::vec3> Kss;
		std::vector<std::string> Materials; // mtl-name
		std::vector<std::string> texture;
		LoadMTL("../../res/Parts/material.mtl", Kds, Kas, Kss, Materials, texture);
		for (int i = 0; i < Materials.size(); i++)
		{
			std::string mtlname = Materials[i];
			KDs[mtlname] = Kds[i];
		}

		// ?†Ë??ÑÈÉ®‰ª?
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

		// ?üÊ? VBO
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
			// Ë§áË£Ω?ÇÈ?Ë≥áÊ?
			glBindBuffer(GL_COPY_WRITE_BUFFER, VBO);
			glBindBuffer(GL_COPY_READ_BUFFER, VBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[0], vertices_size[i] * sizeof(glm::vec3));
			offset[0] += vertices_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(VBOs[i]); // ?ãÊîæ VBO
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			// Ë§áË£Ω UV Ë≥áÊ?
			glBindBuffer(GL_COPY_WRITE_BUFFER, uVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, uVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[1], uvs_size[i] * sizeof(glm::vec2));
			offset[1] += uvs_size[i] * sizeof(glm::vec2);
			glInvalidateBufferData(uVBOs[i]); // ?ãÊîæ VBO
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			// Ë§áË£ΩÊ≥ïÁ?Ë≥áÊ?
			glBindBuffer(GL_COPY_WRITE_BUFFER, nVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, nVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[2], normals_size[i] * sizeof(glm::vec3));
			offset[2] += normals_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(nVBOs[i]); // ?ãÊîæ VBO
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
		const std::string actionsDir = "../../res/actions/";
		const std::string pattern = actionsDir + "*.json"; // find all json files
		actionDatas.clear();

		struct _finddata_t fd;
		intptr_t handle = _findfirst(pattern.c_str(), &fd);
		if (handle != -1) {
			do {
				JsonIO::Action actionData;
				std::string filePath = actionsDir + fd.name;
				if (JsonIO::LoadAction(filePath, actionData)) {
					if (actionData.name == "idle") // let idle be first action
						actionDatas.insert(actionDatas.begin(), actionData);
					else
						actionDatas.push_back(actionData);
				}
			} while (_findnext(handle, &fd) == 0);
			_findclose(handle);
		}
		SetAction(0);
	}

	void MainScene::UpdateAction(double dt)
	{
		const size_t end = curAction.FDs.size();

		dt *= isEdit ? 0.0 : curAction.speed;

		if (curAction.name == "multiple")
		{
			if (frame >= end - 1)
			{
				curInstancedNum = instancedNum;
				effectTime["smoke"] = 5.0f;
			}
		}
		else
			if (!this->keepMultipleActive)
			{
				curInstancedNum = 1;
			}
		scene->SetInstance(curInstancedNum, multipleMode);
		if (isEdit || curInstancedNum == 1 || (curInstancedNum != 1 && curAction.name != "multiple")) {
			HandleAction(curAction.FDs, frame, dt);
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

		Translation[Body::body] = translate(position[Axis::x], 18.5f + position[Axis::y], position[Axis::z]);
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
		JsonIO::FrameData curFD = curAction.FDs[frame], perFD;
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
			-1.0f,  1.0f,  0.0f, 1.0f, // Â∑¶‰?
			-1.0f, -1.0f,  0.0f, 0.0f, // Â∑¶‰?
			1.0f, -1.0f,  1.0f, 0.0f, // ?≥‰?

			-1.0f,  1.0f,  0.0f, 1.0f, // Â∑¶‰?
			1.0f, -1.0f,  1.0f, 0.0f, // ?≥‰?
			1.0f,  1.0f,  1.0f, 1.0f  // ?≥‰?
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
