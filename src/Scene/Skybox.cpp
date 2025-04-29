#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>
#include <Utilty/stb_image.h>

#include "SkyBox.h"
#include "Camera.h"

const unsigned int width = 800;
const unsigned int height = 800;

float size = 100.0f;

float skyboxVertices[] =
{
	//   Coordinates
	-size, -size,  size,//        7--------6
	 size, -size,  size,//       /|       /|
	 size, -size, -size,//      4--------5 |
	-size, -size, -size,//      | |      | |
	-size,  size,  size,//      | 3------|-2
	 size,  size,  size,//      |/       |/
	 size,  size, -size,//      0--------1
	-size,  size, -size
};

unsigned int skyboxIndices[] =
{
	// Right (面朝 +x)
	2, 1, 5,
	5, 6, 2,
	// Left (面朝 -x)
	0, 3, 7,
	7, 4, 0,
	// Top (面朝 +y)
	4, 7, 6,
	6, 5, 4,
	// Bottom (面朝 -y)
	3, 0, 1,
	1, 2, 3,
	// Back (面朝 -z)
	3, 2, 6,
	6, 7, 3,
	// Front (面朝 +z)
	0, 4, 5,
	5, 1, 0
};

namespace CG
{
	auto SkyBox::Initialize() -> bool
	{
		return LoadScene();
	}

	auto SkyBox::LoadScene() -> bool
	{
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		ShaderInfo shaders[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/SkyBox_Material.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/SkyBox_Material.fp" },//fragment shader
			{ GL_NONE, NULL } };

		program = LoadShaders(shaders); //讀取shader

		glUseProgram(program);//uniform參數數值前必須先use shader

		MatricesIdx = glGetUniformBlockIndex(program, "MatVP");

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

	void SkyBox::Render(float camX, float camY, float camZ, float aspect, GLenum mode)
	{
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

		// Since the cubemap will always have a depth of 1.0, we need that equal sign so it doesn't get discarded
		glDepthFunc(GL_LEQUAL);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Switch back to the normal depth function
		glDepthFunc(GL_LESS);
	}

	void SkyBox::LoadModel()
	{
		std::string parentDir = "../../res/SkyBox/";
		std::string facesCubemap[6] =
		{
			parentDir + "px.png",
			parentDir + "nx.png",
			parentDir + "py.png",
			parentDir + "ny.png",
			parentDir + "pz.png",
			parentDir + "nz.png"
		};

		glGenTextures(1, &cubemapTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// These are very important to prevent seams
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// This might help with seams on some systems
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// Cycles through all the textures and attaches them to the cubemap object
		for (unsigned int i = 0; i < 6; i++)
		{
			int width, height, nrChannels;
			unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				std::cout << "load skybox texture: " << facesCubemap[i] << std::endl;
				stbi_set_flip_vertically_on_load(false);
				glTexImage2D
				(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					GL_RGBA,
					width,
					height,
					0,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					data
				);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
				stbi_image_free(data);
			}
		}
		//VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glUseProgram(program);

		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}
