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

// 繪製方形的index
unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0
};

namespace CG 
{
	auto Effects::Initialize() -> bool
	{
		return LoadTexture();
	}

	auto Effects::LoadTexture() -> bool
	{
		glCullFace(GL_BACK);
		std::string parentDir = "../../res/Effects/";
		std::string effectsTex[1] =
		{
			parentDir + "smoke.png"
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

			std::vector<glm::mat4> E;
			for (int j = 0; j < 10; j++) {
				glm::mat4 M = glm::mat4(1.0);
				E.push_back(M);
			}
			EffectInforms.push_back({ E, glm::vec3(0), 1.0f, 15.0, true });
		}

		setupMesh();
		setProgram("../../res/shaders/Part_Effects.vp", "../../res/shaders/Part_Effects.fp", 0);
		return true;
	}

	void Effects::setupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(smokePosition), smokePosition, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &modelVBO);
		glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
		glBufferData(GL_ARRAY_BUFFER, 10 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);  // 分配空間，但先不寫資料

		for (int i = 0; i < 4; i++) {
			glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
			glEnableVertexAttribArray(2 + i);
			glVertexAttribDivisor(2 + i, 1); // 一個 instance 更新一次
		}
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
			for (int i = 0; i < EffectInforms[0].effect_Model.size(); i++) {
				EffectInforms[0].effect_Model[i] *= rotate(angle, 0, 1, 0);
			}
		}
	}

	void Effects::renderEffects(bool enable, float camX, float camY, float camZ, float aspect, GLenum mode, std::string effects, float time, int index) 
	{
		if (!enable) {
			for (int i = 0; i < effectsNum; i++) {
				EffectInforms[i].firstAppear = true;
			}
			return;
		}

		GLuint Program = program[index];
		glUseProgram(Program);
		// 綁定 VAO
		glBindVertexArray(VAO);
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
			updateSmoke(time);
			updateModel(0);
//			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(effects_Model[0]));
			glUniform1f(glGetUniformLocation(Program, "alpha"), EffectInforms[0].alpha);

			// 綁定 Texture 到 level 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Effect_Texture[0]); // 假設使用煙霧特效的 Texture
			glUniform1i(glGetUniformLocation(Program, "effectTexture"), 0); // 告訴 Shader 紋理單元位置
			glUniform1i(glGetUniformLocation(Program, enableEffects[0].c_str()), enable);

			// 繪製矩形（使用索引繪製）
//			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // 繪製兩個三角形形成的矩形
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 10);
		}
		
		glDisable(GL_BLEND);
		glUseProgram(0);
	}

	void Effects::updateModel(int num) {
		glBindBuffer(GL_ARRAY_BUFFER, modelVBO);  // 綁定先前建好的 VBO

		// 假設 effects_Model[num] 是 std::vector<glm::mat4>
		glBufferSubData(GL_ARRAY_BUFFER, 0, EffectInforms[num].effect_Model.size() * sizeof(glm::mat4), EffectInforms[num].effect_Model.data());
	}

	void Effects::updateSmoke(float Time)
	{
		EffectInforms[0].alpha -= 1.0f / EffectInforms[0].time;
		EffectInforms[0].trans.y += 1.0f / EffectInforms[0].time;

		if (EffectInforms[0].alpha < 0.0f || Time <= 1) {
			EffectInforms[0].trans = glm::vec3(0);
			EffectInforms[0].alpha = 1.0f;
		}

		if (EffectInforms[0].firstAppear) {
			for (int i = 0; i < EffectInforms[0].effect_Model.size(); i++) {
				EffectInforms[0].effect_Model[i] *= translate(-250.0 + i * 50.0, EffectInforms[0].trans.y, 0);
			}

			EffectInforms[0].firstAppear = false;
		}
	}
}
