#include <Utilty/stb_image.h>
#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>

#include "./Electricity.h"

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


float electricityVertices[] = {
	// 位置 (x, y, z)              // UV (s, t)
	-1, -1.0f, 0.0f,          0.0f, 0.0f,
	 1, -1.0f, 0.0f,          1.0f, 0.0f,
	 1,  1.0f, 0.0f,          1.0f, 1.0f,
	-1,  1.0f, 0.0f,          0.0f, 1.0f,
};

unsigned int electricityIndices[] = {
	0, 1, 2,
	2, 3, 0
};

namespace CG
{
	auto Electricity::Initialize() -> bool
	{
		Model *= translate(0, ElectricitySize,0);
		Model *= scale(ElectricitySize, ElectricitySize, ElectricitySize);
		return LoadScene();
	}

	auto Electricity::LoadScene() -> bool
	{
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		//VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		ShaderInfo shaders[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/Electricity.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/Electricity.fp" },//fragment shader
			{ GL_NONE, NULL } };

		program = LoadShaders(shaders); //讀取shader

		glUseProgram(program); //uniform參數數值前必須先use shader

		MatricesIdx = glGetUniformBlockIndex(program, "MatVP");
		ModelID = glGetUniformLocation(program, "Model");
		aspectID = glGetUniformLocation(program, "aspect");
		ElectricityTextureID = glGetUniformLocation(program, "ElectricityTexture");
		ElectricityBallTextureID = glGetUniformLocation(program, "ElectricityBallTexture");
		isBallID = glGetUniformLocation(program, "isBall");
		rowsID = glGetUniformLocation(program, "rows");
		colsID = glGetUniformLocation(program, "cols");
		nowFrameID = glGetUniformLocation(program, "nowFrame");
		totalFramesID = glGetUniformLocation(program, "totalFrames");
		instanceCountID = glGetUniformLocation(program, "instanceCount");

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

		std::string path = "../../res/Effects/spritesheet.png";
		int width, height, nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
		if (data)
		{
			std::cout << "load water texture: " << path << std::endl;
			stbi_set_flip_vertically_on_load(false);

			glGenTextures(1, &ElectricityTexture);
			glBindTexture(GL_TEXTURE_2D, ElectricityTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << path << std::endl;
			stbi_image_free(data);
		}

		path = "../../res/Effects/ElectricBall.png";
		data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
		if (data)
		{
			std::cout << "load water texture: " << path << std::endl;
			stbi_set_flip_vertically_on_load(false);

			glGenTextures(1, &ElectricityBallTexture);
			glBindTexture(GL_TEXTURE_2D, ElectricityBallTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << path << std::endl;
			stbi_image_free(data);
		}

		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		// VBO 綁定
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(electricityVertices), electricityVertices, GL_STATIC_DRAW);

		// EBO 綁定（用 index 畫）
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(electricityIndices), electricityIndices, GL_STATIC_DRAW);

		// position 屬性 (location = 0)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// texCoord 屬性 (location = 1)
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0); // unbind
		return true;
	}

	void Electricity::Render(float camX, float camY, float camZ, float aspect, GLenum mode)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPolygonMode(GL_FRONT_AND_BACK, mode);// mode = 0, fill

		glBindVertexArray(VAO);
		glUseProgram(program);//uniform參數數值前必須先use shader

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

		//bind sprite sheet texture
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, ElectricityTexture);
		glUniform1i(ElectricityTextureID, 9);

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, ElectricityBallTexture);
		glUniform1i(ElectricityBallTextureID, 10);


		// draw electricity

		float aspectTriangle = 95.0f / 253.0f;
		glUniform1f(aspectID, aspectTriangle);
		glUniform1i(rowsID, rows);
		glUniform1i(colsID, cols);
		glUniform1i(totalFramesID, totalFrames);
		glUniform1i(nowFrameID, (int)frame);
		glUniform1i(instanceCountID, instanceCount);
		glUniform1i(isBallID, 0);

		glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instanceCount);

		// draw electricity ball
		glm::mat4 faceCameraRotation = glm::mat4(1.0f);

		// 計算 camera 的方向向量
		glm::mat4 view = camera.GetViewMatrix();
		glm::vec3 camRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
		glm::vec3 camUp = glm::vec3(view[0][1], view[1][1], view[2][1]);
		glm::vec3 camFront = -glm::vec3(view[0][2], view[1][2], view[2][2]);

		// 組旋轉矩陣
		faceCameraRotation[0] = glm::vec4(camRight, 0.0f);  // X
		faceCameraRotation[1] = glm::vec4(camUp, 0.0f);  // Y
		faceCameraRotation[2] = glm::vec4(camFront, 0.0f);  // Z

		BallModel = glm::mat4(1.0);
		BallModel *= translate(0, 20, 0);
		BallModel *= faceCameraRotation;
		BallModel *= scale(ElectricityBallSize, ElectricityBallSize, ElectricityBallSize);

		aspectTriangle = 75.0f / 80.0f;
		glUniform1f(aspectID, aspectTriangle);

		glUniform1i(rowsID, ballRows);
		glUniform1i(colsID, ballCols);
		glUniform1i(totalFramesID, ballTotalFrames);
		glUniform1i(nowFrameID, (int)ballFrame);
		glUniform1i(isBallID, 1);

		glUniformMatrix4fv(ModelID, 1, GL_FALSE, &BallModel[0][0]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		glBindVertexArray(0);

		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glFlush();
	}

	void Electricity::Update(double dt) {
		frame += 1;
		if (frame > 6) {
			frame = 0;
		}
		ballFrame += 0.6;
		if (ballFrame > 6) {
			ballFrame = 0;
		}
	}
}
