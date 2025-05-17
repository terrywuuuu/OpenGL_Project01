#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>
#include <Utilty/stb_image.h>

#include "./Water.h"
#include "./WaterTile.h"

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

glm::mat4 createTransformationMatrix(const glm::vec3& translation,
	float rx, float ry, float rz, float scaleSize) {
	glm::mat4 matrix = glm::mat4(1.0f);
	matrix *= translate(translation.x, translation.y, translation.z);
	matrix *= scale(scaleSize, scaleSize, scaleSize);
	return matrix;
}

float vertices[] = {
	-1.0f, -1.0f, 0.0f, // Triangle 1, Point 1
	 1.0f, -1.0f, 0.0f, // Triangle 1, Point 2
	-1.0f,  1.0f, 0.0f, // Triangle 1, Point 3

	-1.0f,  1.0f, 0.0f, // Triangle 2, Point 1
	 1.0f, -1.0f, 0.0f, // Triangle 2, Point 2
	 1.0f,  1.0f, 0.0f  // Triangle 2, Point 3
};

namespace CG
{
	auto Water::Initialize() -> bool
	{
		const float tileSize = WaterTile::TILE_SIZE;
		const float startX = -(GRIDSIZE / 2) * tileSize;
		const float startZ = -(GRIDSIZE / 2) * tileSize;

		for (int i = 0; i < GRIDSIZE; ++i) {
			for (int j = 0; j < GRIDSIZE; ++j) {
				int index = i * GRIDSIZE + j;
				float x = startX + j * tileSize; // calc waterTile x
				float z = startZ + i * tileSize; // calc waterTile z
				WaterTile waterTile(x, z, height);
				waterTiles.push_back(waterTile);
				glm::mat4 model = createTransformationMatrix(
					glm::vec3(waterTiles[index].getX(), waterTiles[index].getHeight(), waterTiles[index].getZ()),
					0.0f, 0.0f, 0.0f,
					WaterTile::TILE_SIZE
				);
				Models.push_back(model);
			}
		}
		return LoadScene();
	}

	auto Water::LoadScene() -> bool
	{
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		//VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		ShaderInfo shaders[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/Water.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/Water.fp" },//fragment shader
			{ GL_NONE, NULL } };

		program = LoadShaders(shaders); //弄shader

		glUseProgram(program); //uniform把计计玡ゲ斗use shader

		MatricesIdx = glGetUniformBlockIndex(program, "MatVP");
		ModelID = glGetUniformLocation(program, "Model");

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

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return true;
	}

	void Water::Render(float camX, float camY, float camZ, float aspect, GLenum mode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, mode);// mode = 0, fill

		glBindVertexArray(VAO);
		glUseProgram(program);//uniform把计计玡ゲ斗use shader

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


		for (int i = 0; i < GRIDSIZE; ++i) {
			for (int j = 0; j < GRIDSIZE; ++j) {
				int index = i * GRIDSIZE + j;
				glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Models[index][0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}

		glBindVertexArray(0);
		glFlush();
	}
}
