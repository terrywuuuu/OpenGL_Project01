#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>
#include <Utilty/stb_image.h>

#include "SkyBox.h"
#include "Camera.h"

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

float size = 1.0f;

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
	// Right (­±´Â +x)
	2, 1, 5,
	5, 6, 2,
	// Left (­±´Â -x)
	0, 3, 7,
	7, 4, 0,
	// Top (­±´Â +y)
	4, 7, 6,
	6, 5, 4,
	// Bottom (­±´Â -y)
	3, 0, 1,
	1, 2, 3,
	// Back (­±´Â -z)
	3, 2, 6,
	6, 7, 3,
	// Front (­±´Â +z)
	0, 4, 5,
	5, 1, 0
};

namespace CG
{
	auto SkyBox::Initialize() -> bool
	{
		Model = glm::mat4(1.0);
		Model *= translate(0, -50, 0);
		Model *= scale(20, 20, 20);
		return LoadScene();
	}

	auto SkyBox::LoadScene() -> bool
	{
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		ShaderInfo shaders[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/SkyBox.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/SkyBox.fp" },//fragment shader
			{ GL_NONE, NULL } };

		program = LoadShaders(shaders); //Åª¨úshader

		ShaderInfo Shaders[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/Ball.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/Ball.fp" },//fragment shader
			{ GL_NONE, NULL } };

		Ball_program = LoadShaders(Shaders); //Åª¨úshader

		glUseProgram(program);//uniform°Ñ¼Æ¼Æ­È«e¥²¶·¥ýuse shader

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

	void SkyBox::Render(float camX, float camY, float camZ, float aspect, GLenum mode, bool environmentMap, GLuint envCubemap, bool isEnv)
	{
		glPolygonMode(GL_FRONT_AND_BACK, mode);// mode = 0, fill

		glBindVertexArray(VAO);
		glUseProgram(program);//uniform°Ñ¼Æ¼Æ­È«e¥²¶·¥ýuse shader

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
		
		if (environmentMap && !isEnv) {
			glUseProgram(Ball_program);
			glBindVertexArray(BallVAO);

			glUniform3f(glGetUniformLocation(Ball_program, "cameraPos"), camX, camY, camZ);
			glUniformMatrix4fv(ProID, 1, GL_FALSE, &camera.GetProjectionMatrix()[0][0]);
			glUniformMatrix4fv(ViewID, 1, GL_FALSE, &camera.GetViewMatrix()[0][0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
			glUniform1i(glGetUniformLocation(Ball_program, "environmentMap"), 0);

			GLuint offset[3] = { 0,0,0 };//offset for vertices , uvs , normals
			for (int i = 0; i < 1; i++)
			{
				glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);

				glBindBuffer(GL_ARRAY_BUFFER, BallVBO);
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
				glBindBuffer(GL_ARRAY_BUFFER, BalluVBO);
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
				glBindBuffer(GL_ARRAY_BUFFER, BallnVBO);
				glVertexAttribPointer(2,
					3,
					GL_FLOAT,
					GL_FALSE,
					0,
					(void*)offset[2]);
				//(location,vec3,type,?ºå?é»????é»žç??ç§»??point)
				offset[2] += normals_size[i] * sizeof(glm::vec3);

				int vertexIDoffset = 0;//glVertexID's offset 

				for (int j = 0; j < mtls[i].size(); j++)
				{
					glDrawArrays(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3);
					//we draw triangles by giving the glVertexID base and vertex count is face count*3
					vertexIDoffset += faces[i][j + 1] * 3;//glVertexID's base offset is face count*3
				}
			}//end for loop for draw one part of the robot	
		}
	}//end for loop for updating and drawing model

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

		glGenVertexArrays(1, &BallVAO);
		glBindVertexArray(BallVAO);
		glUseProgram(Ball_program);

		ModelID = glGetUniformLocation(Ball_program, "model");
		ProID = glGetUniformLocation(Ball_program, "projection");
		ViewID = glGetUniformLocation(Ball_program, "view");

		Load2Buffer("../../res/Parts/Ball.obj", 0);

		GLuint totalSize[3] = { 0, 0, 0 };
		GLuint offset[3] = { 0, 0, 0 };
		for (int i = 0; i < 1; i++)
		{
			totalSize[0] += vertices_size[i] * sizeof(glm::vec3);
			totalSize[1] += uvs_size[i] * sizeof(glm::vec2);
			totalSize[2] += normals_size[i] * sizeof(glm::vec3);
		}

		glGenBuffers(1, &BallVBO);
		glGenBuffers(1, &BalluVBO);
		glGenBuffers(1, &BallnVBO);

		glBindBuffer(GL_ARRAY_BUFFER, BallVBO);
		glBufferData(GL_ARRAY_BUFFER, totalSize[0], NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, BalluVBO);
		glBufferData(GL_ARRAY_BUFFER, totalSize[1], NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, BallnVBO);
		glBufferData(GL_ARRAY_BUFFER, totalSize[2], NULL, GL_STATIC_DRAW);

		for (int i = 0; i < 1; i++)
		{
			glBindBuffer(GL_COPY_WRITE_BUFFER, BallVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, VBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[0], vertices_size[i] * sizeof(glm::vec3));
			offset[0] += vertices_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(VBOs[i]);
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			glBindBuffer(GL_COPY_WRITE_BUFFER, BalluVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, uVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[1], uvs_size[i] * sizeof(glm::vec2));
			offset[1] += uvs_size[i] * sizeof(glm::vec2);
			glInvalidateBufferData(uVBOs[i]);
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			glBindBuffer(GL_COPY_WRITE_BUFFER, BallnVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, nVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[2], normals_size[i] * sizeof(glm::vec3));
			offset[2] += normals_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(nVBOs[i]);
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		}
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		glEnableVertexAttribArray(0);
	}

	void SkyBox::Load2Buffer(const char* obj, int i)
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
}
