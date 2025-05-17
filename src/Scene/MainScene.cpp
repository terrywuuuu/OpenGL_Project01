#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>
#include <io.h>

#include "MainScene.h"

static glm::mat4 translate(float x, float y, float z)
{
	glm::vec4 t = glm::vec4(x, y, z, 1);//w = 1 ,?‡x,y,z=0?‚ä??½translate
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
		water = new Water();
		waterFrameBuffer = new WaterFrameBuffer();
		
		scene->Initialize();
		skyBox->Initialize();
		effect->Initialize();
		water->Initialize();

		//Initialize MusicPlayer
		musicPlayer = new MusicPlayer();
		return LoadScene();
	}

	void MainScene::Update(double dt)
	{
		UpdateAction(dt);
		UpdateModel();
	}

	void MainScene::GenerateWaterFrameBufferAndRender(float aspect, float width, float height) {
		// call mainScene->Render and set aspect
		glEnable(GL_CLIP_DISTANCE0);
		waterFrameBuffer->bindReflectionFrameBuffer();
		Render(aspect, glm::vec4(0, 1, 0, water->getHeight()), CamerMode::reflection);


		waterFrameBuffer->bindRefractionFrameBuffer();
		Render(aspect, glm::vec4(0, -1, 0, water->getHeight()), CamerMode::refraction);

		waterFrameBuffer->unbindCurrentFrameBuffer(width, height);

		glDisable(GL_CLIP_DISTANCE0);
		Render(aspect, glm::vec4(0, -1, 0, 5), CamerMode::normal);
	}
	//cameraMode: 0:normal 1:reflection
	void MainScene::Render(float aspect, glm::vec4 plane, CamerMode cameraMode)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		glClearColor(0.0, 0.0, 0.0, 1); //black screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, mode);// mode = 0, fill
		glDisable(GL_CULL_FACE);

		shadowTransforms.clear();
		camera.LookAt(LightPos, LightPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		shadowTransforms.push_back(camera.GetViewMatrix()); // +X
		camera.LookAt(LightPos, LightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		shadowTransforms.push_back(camera.GetViewMatrix()); // -X
		camera.LookAt(LightPos, LightPos + glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));
		shadowTransforms.push_back(camera.GetViewMatrix());  // +Y
		camera.LookAt(LightPos, LightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
		shadowTransforms.push_back(camera.GetViewMatrix());  // -Y
		camera.LookAt(LightPos, LightPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		shadowTransforms.push_back(camera.GetViewMatrix());  // +Z
		camera.LookAt(LightPos, LightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
		shadowTransforms.push_back(camera.GetViewMatrix());  // -Z
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glUseProgram(LightProgram);
		camera.SetAspect(1.0);
		camera.SetFov(90.0f);
		camera.SetClip(0.1, 200);


		float theta = glm::radians(eyeAngley);
		float phi = glm::radians(angle);

		float camX = eyedistance * cos(phi) * sin(theta);
		float camY = eyedistance * sin(phi);
		float camZ = eyedistance * cos(phi) * cos(theta);

		for (int i = 0; i < 6; i++) {
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				depthCubemap,
				0
			);

			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			glClear(GL_DEPTH_BUFFER_BIT);

			glUniformMatrix4fv(glGetUniformLocation(LightProgram, "shadowMatrix"), 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix() * shadowTransforms[i]));
			GLuint modelLoc = glGetUniformLocation(LightProgram, "Model");
			RenderMainScene(aspect, camX, camY, camZ, true, depthCubemap, modelLoc);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glClearColor(0.0, 0.0, 0.0, 1); //black screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, mode);// mode = 0, fill
		glDisable(GL_CULL_FACE);
		glUseProgram(program);
		glUniform1i(glGetUniformLocation(program, "enableToonShader"), enableToonShader);
		glBindVertexArray(lightVAO);

		glUniformMatrix4fv(PreViewId, 1, GL_FALSE, &PreView[0][0]);
		glUniformMatrix4fv(PreProjectionId, 1, GL_FALSE, &PreProjection[0][0]);

		if (cameraMode == CamerMode::normal)
		{
			camera.LookAt(
				glm::vec3(camX, camY, camZ),
				glm::vec3(0, 0, 0),
				glm::vec3(0, 1, 0)
			);
			camera.SetAspect(aspect);
		}
		else {
			float invertedPhi = -phi;

			camX = eyedistance * cos(invertedPhi) * sin(theta);
			camY = eyedistance * sin(invertedPhi);
			camZ = eyedistance * cos(invertedPhi) * cos(theta);

			float distance = camY - water->getHeight();
			float reflectedCamY = camY - 2 * distance;
			float targetY = 0;
			float reflectedTargetY = targetY - 2 * (targetY - water->getHeight());
			camera.LookAt(
				glm::vec3(camX, reflectedCamY, camZ),          // Ãè¹³«áªº¦ì¸m
				glm::vec3(0, reflectedTargetY, 0),              // Ãè¹³«áªº¥Ø¼Ð
				glm::vec3(0, 1, 0)                              // ¤W¤è¦Vºû«ù¤£ÅÜ
			);
		}

		PreView = camera.GetViewMatrix();
		PreProjection = camera.GetProjectionMatrix();

		//update data to UBO for MVP
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &camera.GetViewMatrix()[0][0]);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &camera.GetProjectionMatrix()[0][0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glUniform4f(PlaneID, plane.x, plane.y, plane.z, plane.w);

		// ---- ´è¬V¥ú·½ Cube ----
		glUniformMatrix4fv(ModelID, 1, GL_FALSE, glm::value_ptr(lightModel));
		glUniform3f(glGetUniformLocation(program, "vLightPosition"), LightPos.x, LightPos.y, LightPos.z);
		glUniform1f(glGetUniformLocation(program, "lightCube"), 1);
		glUniform1f(glGetUniformLocation(program, "isLightCube"), 1);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glUniform1f(glGetUniformLocation(program, "lightCube"), 0);
		glUniform1f(glGetUniformLocation(program, "isLightCube"), 0);
		// ------------------------------

		RenderMainScene(aspect, camX, camY, camZ, false, depthCubemap, ModelID);

		if (enableToonShader) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glUseProgram(OutlineProgram);

			glUniformMatrix4fv(glGetUniformLocation(OutlineProgram, "View"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
			glUniformMatrix4fv(glGetUniformLocation(OutlineProgram, "Projection"), 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));
			GLuint modelLoc = glGetUniformLocation(OutlineProgram, "Model");
			RenderMainScene(aspect, camX, camY, camZ, false, depthCubemap, modelLoc);
			glCullFace(GL_BACK);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Texture_Render();

		glFlush();

	}

	void MainScene::RenderMainScene(float aspect, float camX, float camY, float camZ, bool isDepth, GLuint depthCubemap, GLuint modelID) {
		glBindVertexArray(VAO);

		GLuint offset[3] = {0,0,0};//offset for vertices , uvs , normals
		for (int i = 0; i < PARTSNUM; i++)
		{
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &Models[i][0][0]);
			glUniformMatrix4fv(PreModelID, 1, GL_FALSE, &PreModels[i][0][0]);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0,				//location
				3,				//vec3
				GL_FLOAT,			//type
				GL_FALSE,			//not normalized
				0,				//strip
				(void*)offset[0]);//buffer offset
			//(location,vec3,type,?ºå?é»????é»žç??ç§»??buffer point)
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
			//(location,vec2,type,?ºå?é»????é»žç??ç§»??point)
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
			//(location,vec3,type,?ºå?é»????é»žç??ç§»??point)
			offset[2] += normals_size[i] * sizeof(glm::vec3);

			int vertexIDoffset = 0;//glVertexID's offset 
			std::string mtlname;//material name

			for (int j = 0; j < mtls[i].size(); j++)
			{
				mtlname = mtls[i][j];
				//find the material diffuse color in map:KDs by material name.
				if (!isDepth) {
					glUniform3fv(M_KdID, 1, &KDs[mtlname][0]);
					glUniform3fv(M_KsID, 1, &KSs[mtlname][0]);
				}
				//          (primitive   , glVertexID base , vertex count    )
				if (curInstancedNum == 1) {
					glDrawArrays(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3);
				}
				else {
					if (!isDepth) {
						if (i == PARTSNUM - 1) {
							glUniform1i(BackGround, 0);
						}
						else {
							glUniform1i(BackGround, 1);
						}
						glUniform1i(MultipleMode, multipleMode);
					}
					
					glDrawArraysInstanced(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3, curInstancedNum);
				}
				//we draw triangles by giving the glVertexID base and vertex count is face count*3
				vertexIDoffset += faces[i][j + 1] * 3;//glVertexID's base offset is face count*3
			}//end for loop for draw one part of the robot	

		}//end for loop for updating and drawing model

		glBindVertexArray(0);

		/*
		scene->Render(camX, camY, camZ, aspect, mode, LightProgram, isDepth, depthCubemap, LightPos, camera);*/
		if (!isDepth) {
			skyBox->Render(camX, camY, camZ, aspect, mode, enableEnvironmentMap);
		}
		water->Render(camX, camY, camZ, aspect, mode);
		/*
		if (effectTime["smoke"] != 0) {
			effect->renderEffects(true, camX, camY, camZ, aspect, mode, "smoke", effectTime["smoke"], 0);
			effectTime["smoke"]--;
		}*/
	}

	void MainScene::Texture_Render() {
		// use post process program
		glUseProgram(Post_Process); 
		glBindVertexArray(screenQuadVAO);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(Post_Process, "sceneTexture"), 3);
		glUniform2f(glGetUniformLocation(Post_Process, "texSize"), screenWidth, screenHeight);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, motionTexture);
		glUniform1i(glGetUniformLocation(Post_Process, "motionTexture"), 4);

		glUniform1f(glGetUniformLocation(Post_Process, "blurStrength"), blurStrength);
		glUniform1f(glGetUniformLocation(Post_Process, "quanStrength"), quanStrength);
		glUniform1f(glGetUniformLocation(Post_Process, "mosaicSize"), mosaicStrength);
		glUniform1i(glGetUniformLocation(Post_Process, "enableBlur"), enableBlur);
		glUniform1i(glGetUniformLocation(Post_Process, "enableQuan"), enableQuan);
		glUniform1i(glGetUniformLocation(Post_Process, "enableMosaic"), enableMosaic);
		glUniform1i(glGetUniformLocation(Post_Process, "enableMotionBlur"), enableMotionBlur);
		glUniform1i(glGetUniformLocation(Post_Process, "motionBlurStrength"), motionBlurStrength);

		// ´è¬V«Ì¹õ¥|Ãä§ÎÅã¥Üµ²ªG
		glDrawArrays(GL_TRIANGLES, 0, 6);  // ´è¬V¥|Ãä§Î
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
		//6: Arrow Up
		//7: Arrow Down
		//8: Arrow Left
		//9: Arrow Right
		//10: key "q" press
		//11: key "e" press
		switch (key)
		{
		case 0:
			eyeAngley -= 10;
			effect->setAngle("smoke", -10);
			break;
		case 1:
			eyeAngley += 10;
			effect->setAngle("smoke", 10);
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
		case 6:
			LightPos.y += 2.0;
			lightModel *= translate(0, 2, 0);
			printf("Light Position:%f, %f, %f\n", LightPos.x, LightPos.y, LightPos.z);
			break;
		case 7:
			LightPos.y -= 2.0;
			lightModel *= translate(0, -2, 0);
			printf("Light Position:%f, %f, %f\n", LightPos.x, LightPos.y, LightPos.z);
			break;
		case 8:
			LightPos.x -= 2.0;
			lightModel *= translate(-2, 0, 0);
			printf("Light Position:%f, %f, %f\n", LightPos.x, LightPos.y, LightPos.z);
			break;
		case 9:
			LightPos.x += 2.0;
			lightModel *= translate(2, 0, 0);
			printf("Light Position:%f, %f, %f\n", LightPos.x, LightPos.y, LightPos.z);
			break;
		case 10:
			LightPos.z -= 2.0;
			lightModel *= translate(0, 0, -2);
			printf("Light Position:%f, %f, %f\n", LightPos.x, LightPos.y, LightPos.z);
			break;
		case 11:
			LightPos.z += 2.0;
			lightModel *= translate(0, 0, 2);
			printf("Light Position:%f, %f, %f\n", LightPos.x, LightPos.y, LightPos.z);
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
		case 2:
			enableMosaic = isActive;
			mosaicStrength = num;
			break;
		case 3:
			enableMotionBlur = isActive;
			motionBlurStrength = num;
			break;
		case 4:
			enableEnvironmentMap = isActive;
			break;
		case 5:
			enableToonShader = isActive;
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

		// Motion Texture
		glGenTextures(1, &motionTexture);
		glBindTexture(GL_TEXTURE_2D, motionTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, screenWidth, screenHeight, 0, GL_RG, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, motionTexture, 0);

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

	void MainScene::setLightTexture() {
		glGenTextures(1, &depthCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		glGenFramebuffers(1, &depthMapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

/*		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Depth Framebuffer not complete!" << std::endl;
		}*/
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
		program = LoadShaders(shaders); 

		ShaderInfo shader[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/Post-Process.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/Post-Process.fp" },//fragment shader
			{ GL_NONE, NULL } };
		Post_Process = LoadShaders(shader);

		ShaderInfo Shader[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/Light.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/Light.fp" },//fragment shader
			{ GL_NONE, NULL } };
		LightProgram = LoadShaders(Shader);

		ShaderInfo Shaderss[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/Outline.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/Outline.fp" },//fragment shader
			{ GL_NONE, NULL } };
		OutlineProgram = LoadShaders(Shaderss);
		
//		effect->setProgram(Post_Process);
    
		glUseProgram(program);//uniform?ƒæ•¸?¸å€¼å?å¿…é??ˆuse shader

		MatricesIdx = glGetUniformBlockIndex(program, "MatVP");
		PreViewId = glGetUniformLocation(program, "PreView");
		PreProjectionId = glGetUniformLocation(program, "PreProjection");
		ModelID = glGetUniformLocation(program, "Model");
		PreModelID = glGetUniformLocation(program, "PreModel");
		M_KaID = glGetUniformLocation(program, "Material.Ka");
		M_KdID = glGetUniformLocation(program, "Material.Kd");
		M_KsID = glGetUniformLocation(program, "Material.Ks");
		BackGround = glGetUniformLocation(program, "isInstanced");
		MultipleMode = glGetUniformLocation(program, "MultipleMode");
		PlaneID = glGetUniformLocation(program, "plane");

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
		setLightCube();
		setLightTexture();

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

		// ? è??„éƒ¨ä»?
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

		// ?Ÿæ? VBO
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
			// è¤‡è£½?‚é?è³‡æ?
			glBindBuffer(GL_COPY_WRITE_BUFFER, VBO);
			glBindBuffer(GL_COPY_READ_BUFFER, VBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[0], vertices_size[i] * sizeof(glm::vec3));
			offset[0] += vertices_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(VBOs[i]); // ?‹æ”¾ VBO
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			// è¤‡è£½ UV è³‡æ?
			glBindBuffer(GL_COPY_WRITE_BUFFER, uVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, uVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[1], uvs_size[i] * sizeof(glm::vec2));
			offset[1] += uvs_size[i] * sizeof(glm::vec2);
			glInvalidateBufferData(uVBOs[i]); // ?‹æ”¾ VBO
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			// è¤‡è£½æ³•ç?è³‡æ?
			glBindBuffer(GL_COPY_WRITE_BUFFER, nVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, nVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[2], normals_size[i] * sizeof(glm::vec3));
			offset[2] += normals_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(nVBOs[i]); // ?‹æ”¾ VBO
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

				if (isFirstAppear) 
				{
					effectTime["smoke"] = 10.0f;
					isFirstAppear = false;
				}
			}
		}
		else if (!this->keepMultipleActive)
		{
			curInstancedNum = 1;
			isFirstAppear = true;
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
			PreModels[i] = Models[i];
		}
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
			// ¿Ã¹õ®y¼Ð  // UV
			-1.0f,  1.0f,  0.0f, 1.0f, 
			-1.0f, -1.0f,  0.0f, 0.0f, 
			1.0f, -1.0f,  1.0f, 0.0f, 
			
			-1.0f,  1.0f,  0.0f, 1.0f, 
			1.0f, -1.0f,  1.0f, 0.0f, 
			1.0f,  1.0f,  1.0f, 1.0f  
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

	void MainScene::setLightCube() {
		float vertices[] = {
			// positions       
			-2.5f, -2.5f, -2.5f, // 0
			2.5f, -2.5f, -2.5f, // 1
			2.5f,  2.5f, -2.5f, // 2
			-2.5f,  2.5f, -2.5f, // 3
			-2.5f, -2.5f,  2.5f, // 4
			2.5f, -2.5f,  2.5f, // 5
			2.5f,  2.5f,  2.5f, // 6
			-2.5f,  2.5f,  2.5f  // 7
		};

		unsigned int indices[] = {
			// back face
			0, 1, 2,
			2, 3, 0,
			// front face
			4, 5, 6,
			6, 7, 4,
			// left face
			0, 4, 7,
			7, 3, 0,
			// right face
			1, 5, 6,
			6, 2, 1,
			// bottom face
			0, 1, 5,
			5, 4, 0,
			// top face
			3, 2, 6,
			6, 7, 3
		};

		lightModel *= translate(LightPos.x, LightPos.y, LightPos.z);
		glGenVertexArrays(1, &lightVAO);
		glGenBuffers(1, &lightVBO);
		glGenBuffers(1, &EBO);
		glBindVertexArray(lightVAO);

		// ³»ÂI¸ê®Æ
		glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// ¯Á¤Þ¸ê®Æ
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// vertex attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
	}
}
