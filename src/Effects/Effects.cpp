#include <Utilty/stb_image.h>
#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>

#include "Effects.h"

static glm::mat4 translate(float x, float y, float z)
{
	glm::vec4 t = glm::vec4(x, y, z, 1);
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

float smokePosition[] = {
	// 位置					// UV 座標
	 -25.0f,  0.0f,  0.0f, 0.0f, 0.0f, // 左下角
	 25.0f,  0.0f,  0.0f,  1.0f, 0.0f, // 右下角
	 25.0f, 75.0f,  0.0f,  1.0f, 1.0f, // 右上角
	 -25.0f, 75.0f,  0.0f,  0.0f, 1.0f  // 左上角
};

float fireworkPosition[] = { 30.0f, 30.0f, -80.0f };

// 繪製方形的index
unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0
};

namespace CG 
{
	auto Effects::Initialize() -> bool
	{
		EffectInforms.resize(effectsNum);
		return LoadTexture();
	}

	auto Effects::LoadTexture() -> bool
	{
		glCullFace(GL_BACK);
		std::string parentDir = "../../res/Effects/";
		std::string effectsTex[2] =
		{
			parentDir + "smoke.png",
			parentDir + "FireWork.png"
		};

		for (int i = 0; i < effectsNum; i++) {
			glGenTextures(1, &Effect_Texture[i]);
			glBindTexture(GL_TEXTURE_2D, Effect_Texture[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			int width, height, nrChannels;
			unsigned char* data = stbi_load(effectsTex[i].c_str(), &width, &height, &nrChannels, 4);
			if (data)
			{
				std::cout << "load effects texture: " << effectsTex[i] << std::endl;
				GLenum format = GL_RGBA;
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Failed to load texture: " << effectsTex[i] << std::endl;
			}

			for (int j = 0; j < effectCount[i]; j++) {
				EffectInforms[i].push_back({ glm::mat4(1.0), glm::vec3(0), glm::vec2(0), 1.0f, 20.0, true });
			}
		}

		effectInit();
		setupMesh();
		setProgram("../../res/shaders/Part_Effects.vp", "../../res/shaders/Part_Effects.fp", 0);
		return true;
	}

	void Effects::effectInit() {
		for (int i = 0; i < EffectInforms[1].size(); i++) {
			EffectInforms[1][i].Model = glm::mat4(1.0);
			EffectInforms[1][i].trans = glm::vec3(0);
			EffectInforms[1][i].alpha = 1.0f;
			EffectInforms[1][i].time = 100.0f;
		}
	}

	void Effects::setupMesh()
	{
		glGenVertexArrays(1, &sVAO);
		glGenBuffers(1, &sVBO);
		glGenBuffers(1, &sEBO);

		glGenVertexArrays(1, &fVAO);
		glGenBuffers(1, &fVBO);

		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, 1000 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

		// === Set sVAO instance attribute ===
		glBindVertexArray(sVAO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); 
		for (int i = 0; i < 4; i++) {
			glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
			glEnableVertexAttribArray(2 + i);
			glVertexAttribDivisor(2 + i, 1);
		}

		// === Set fVAO instance attribute ===
		glBindVertexArray(fVAO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		for (int i = 0; i < 4; i++) {
			glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
			glEnableVertexAttribArray(2 + i);
			glVertexAttribDivisor(2 + i, 1);
		}

		glBindVertexArray(0);
	}

	void Effects::setProgram(std::string vPath, std::string fPath, int index) {
		ShaderInfo shader[] = {
			{ GL_VERTEX_SHADER, vPath.c_str() },//vertex shader
			{ GL_FRAGMENT_SHADER, fPath.c_str() },//fragment shader
			{ GL_NONE, NULL } };

		program[index] = LoadShaders(shader); 
	}

	void Effects::setAngle(std::string effect, float angle) {
		if (effect == "smoke") {
			for (int i = 0; i < effectCount[0]; i++) {
				EffectInforms[0][i].Model *= rotate(angle, 0, 1, 0);
			}
		}
	}

	void Effects::renderEffects(bool enable, float camX, float camY, float camZ, float aspect, GLenum mode, std::string effects, float time, int index) 
	{
		std::mt19937 gen(rd());
		if (!enable) {
			for (int i = 0; i < effectsNum; i++) {
				for (int j = 0; j < effectCount[i]; j++) {
					EffectInforms[i][j].firstAppear = true;
				}
			}
			return;
		}

		GLuint Program = program[index];
		glUseProgram(Program);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPolygonMode(GL_FRONT_AND_BACK, mode);// mode = 0, fill

		camera.LookAt(
			glm::vec3(camX, camY, camZ),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
		);

		camera.SetAspect(aspect);

		glm::mat4 view = camera.GetViewMatrix();    // 拿目前主場景的 camera 設定
		glm::mat4 projection = camera.GetProjectionMatrix();

		GLuint viewLoc = glGetUniformLocation(Program, "view");
		GLuint projLoc = glGetUniformLocation(Program, "projection");
//		GLuint modelLoc = glGetUniformLocation(program, "model");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		if (effects == "smoke") {
			glBindVertexArray(sVAO);

			glBindBuffer(GL_ARRAY_BUFFER, sVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(smokePosition), smokePosition, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			updateSmoke(time);
			updateModel(0);
//			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(effects_Model[0]));
			glUniform1f(glGetUniformLocation(Program, "alpha"), EffectInforms[0][0].alpha);

			// 綁定 Texture 到 level 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Effect_Texture[0]); 
			glUniform1i(glGetUniformLocation(Program, "effectTexture"), 0); // Shader 紋理單元位置
			glUniform1i(glGetUniformLocation(Program, enableEffects[0].c_str()), enable);
			glUniform1i(glGetUniformLocation(Program, enableEffects[1].c_str()), false);

			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, EffectInforms[0].size());
		}
		else if (effects == "FireWork") {
			glBindVertexArray(fVAO);
			
			glBindBuffer(GL_ARRAY_BUFFER, fVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(fireworkPosition), fireworkPosition, GL_DYNAMIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glDisable(GL_CULL_FACE);
			glEnable(GL_POINT_SPRITE);
			glEnable(GL_PROGRAM_POINT_SIZE);
			updateFireWork(time);
			updateModel(1);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Effect_Texture[1]);
			glUniform1i(glGetUniformLocation(Program, "effectTexture"), 0);
			glUniform1f(glGetUniformLocation(Program, "alpha"), EffectInforms[1][0].alpha);
			glUniform1i(glGetUniformLocation(Program, enableEffects[1].c_str()), enable);
			glUniform1i(glGetUniformLocation(Program, enableEffects[0].c_str()), false);
			glDrawArraysInstanced(GL_POINTS, 0, 1, EffectInforms[1].size());

			glDisable(GL_PROGRAM_POINT_SIZE);
			glDisable(GL_POINT_SPRITE);
		}
		
		glDisable(GL_BLEND);
		glUseProgram(0);
	}

	void Effects::updateModel(int num) {
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);  // 綁定先前建好的 VBO	

		std::vector<glm::mat4> models;
		// 假設 effects_Model[num] 是 std::vector<glm::mat4>
		for (int i = 0; i < EffectInforms[num].size(); i++) {
			models.push_back(EffectInforms[num][i].Model);
		}
		glBufferSubData(GL_ARRAY_BUFFER, 0, EffectInforms[num].size() * sizeof(glm::mat4), models.data());
	}

	void Effects::updateSmoke(float Time)
	{
		EffectInforms[0][0].alpha -= 1.0f / EffectInforms[0][0].time;
		EffectInforms[0][0].trans.y += 1.0f / EffectInforms[0][0].time;

		if (EffectInforms[0][0].alpha < 0.0f || Time <= 1) {
			EffectInforms[0][0].trans = glm::vec3(0);
			EffectInforms[0][0].alpha = 1.0f;
		}

		for (int i = 0; i < EffectInforms[0].size(); i++) {
			EffectInforms[0][i].Model = translate(-250.0 + i * 50.0, EffectInforms[0][0].trans.y, 0);
		}
	}

	void Effects::updateFireWork(float Time) {
		EffectInforms[1][0].time--;

		if (EffectInforms[1][0].time <= 0) {
			effectInit(); 
			std::uniform_real_distribution<> distX(-50.0, 50.0);
			std::uniform_real_distribution<> distY(30.0, 80.0);
			fireworkPosition[0] = distX(gen);
			fireworkPosition[1] = distY(gen);

			for (int i = 0; i < EffectInforms[1].size(); ++i) {
				float num = i * 5.0;
				float angle = glm::radians(num); 
				EffectInforms[1][i].velocity.x = cos(angle) * (static_cast<float>(rand()) / RAND_MAX * 0.5f);  
				EffectInforms[1][i].velocity.y = sin(angle) * (static_cast<float>(rand()) / RAND_MAX * 0.5f);
				EffectInforms[1][i].alpha = 1.0f;
			}
		}

		for (int i = 0; i < EffectInforms[1].size(); ++i) {
			EffectInforms[1][i].trans.x += EffectInforms[1][i].velocity.x;
			EffectInforms[1][i].trans.y += EffectInforms[1][i].velocity.y;

			EffectInforms[1][i].velocity.y -= 0.005f;

			EffectInforms[1][i].alpha -= 0.005f;
			if (EffectInforms[1][i].alpha < 0.0f) EffectInforms[1][i].alpha = 0.0f;

			EffectInforms[1][i].Model = translate(EffectInforms[1][i].trans.x, EffectInforms[1][i].trans.y, 0);
		}
	}
}
