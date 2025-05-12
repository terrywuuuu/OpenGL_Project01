#include <Utilty/LoadShaders.h>
#include <Utilty/OBJLoader.hpp>
#include <Utilty/stb_image.h>

#include "Scene.h"
#include "Camera.h"

static glm::mat4 scale(float x, float y, float z)
{
	glm::vec4 c1 = glm::vec4(x, 0, 0, 0);
	glm::vec4 c2 = glm::vec4(0, y, 0, 0);
	glm::vec4 c3 = glm::vec4(0, 0, z, 0);
	glm::vec4 c4 = glm::vec4(0, 0, 0, 1);
	glm::mat4 M = glm::mat4(c1, c2, c3, c4);
	return M;
}

static glm::mat4 translate(float x, float y, float z)
{
	glm::vec4 t = glm::vec4(x, y, z, 1);//w = 1 ,?‡x,y,z=0?‚ä??½translate
	glm::vec4 c1 = glm::vec4(1, 0, 0, 0);
	glm::vec4 c2 = glm::vec4(0, 1, 0, 0);
	glm::vec4 c3 = glm::vec4(0, 0, 1, 0);
	glm::mat4 M = glm::mat4(c1, c2, c3, t);
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
	auto Scene::Initialize() -> bool
	{
		Models[0] *= scale(10, 10, 10);
//		Models[0] *= rotate(10, 1, 0, 0);
//		Models[0] *= translate(0, 0, -1);
		return LoadScene();
	}

	auto Scene::LoadScene() -> bool
	{
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		//VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		ShaderInfo shaders[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/Scene_Material.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/Scene_Material.fp" },//fragment shader
			{ GL_NONE, NULL } };
		program = LoadShaders(shaders); //Åª¨úshader

		glUseProgram(program);//uniform°Ñ¼Æ¼Æ­È«e¥²¶·¥ýuse shader

		MatricesIdx = glGetUniformBlockIndex(program, "MatVP");
		ModelID = glGetUniformLocation(program, "Model");
		M_KaID = glGetUniformLocation(program, "Material.Ka");
		M_KdID = glGetUniformLocation(program, "Material.Kd");
		M_KsID = glGetUniformLocation(program, "Material.Ks");
		IsInstanced = glGetUniformLocation(program, "isInstanced");
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

		return true;
	}

	void Scene::Render(float camX, float camY, float camZ, float aspect, GLenum mode, GLuint Program, bool isDepth, GLuint depthCubemap, glm::vec3 LightPos, Camera cam)
	{
		glPolygonMode(GL_FRONT_AND_BACK, mode);// mode = 0, fill
		GLuint modelLoc;

		glBindVertexArray(VAO);
		if (isDepth) {
			modelLoc = glGetUniformLocation(Program, "Model");
		}
		else {
			glUseProgram(program);//uniform°Ñ¼Æ¼Æ­È«e¥²¶·¥ýuse shader
			glUniform1f(glGetUniformLocation(program, "far_plane"), 200.0f);
			glUniform3f(glGetUniformLocation(program, "vLightPosition"), LightPos.x, LightPos.y, LightPos.z);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
			glUniform1i(glGetUniformLocation(program, "depthCubemap"), 0);
			modelLoc = ModelID;
		}

		camera = cam;
		
		if (!isDepth) {
			//update data to UBO for MVP
			glBindBuffer(GL_UNIFORM_BUFFER, UBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &camera.GetViewMatrix()[0][0]);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &camera.GetProjectionMatrix()[0][0]);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		GLuint offset[3] = {0,0,0};//offset for vertices , uvs , normals
		for (int i = 0; i < SCENESUM; i++)
		{
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &Models[i][0][0]);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0,				//location
				3,				//vec3
				GL_FLOAT,			//type
				GL_FALSE,			//not normalized
				0,				//strip
				(void*)offset[0]);//buffer offset
			//(location,vec3,type,©T©wÂI,³sÄòÂIªº°¾²¾¶q,buffer point)
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
			//(location,vec2,type,©T©wÂI,³sÄòÂIªº°¾²¾¶q,point)
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
			//(location,vec3,type,©T©wÂI,³sÄòÂIªº°¾²¾¶q,point)
			offset[2] += normals_size[i] * sizeof(glm::vec3);

			int vertexIDoffset = 0;//glVertexID's offset 
			std::string mtlname;//material name

			for (int j = 0; j < mtls[i].size(); j++)
			{
				mtlname = mtls[i][j];
				if (!isDepth) {
					//find the material diffuse color in map:KDs by material name.
					glUniform3fv(M_KdID, 1, &KDs[mtlname][0]);
					glUniform3fv(M_KsID, 1, &KSs[mtlname][0]);
					glUniform3fv(M_KaID, 1, &KAs[mtlname][0]);
				}
				//          (primitive   , glVertexID base , vertex count    )
				if (!isDepth && Textures[mtlname].hasTexture)
				{
					glBindTexture(GL_TEXTURE_2D, Textures[mtlname].texture);
				}
				if (instancedNum == 1) {
					glDrawArrays(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3);
				}
				else {
					if (!isDepth) {
						glUniform1i(IsInstanced, 1);
						glUniform1i(MultipleMode, multipleMode);
					}

					glDrawArraysInstanced(GL_TRIANGLES, vertexIDoffset, faces[i][j + 1] * 3, instancedNum);
				}
				//we draw triangles by giving the glVertexID base and vertex count is face count*3
				vertexIDoffset += faces[i][j + 1] * 3;//glVertexID's base offset is face count*3
			}//end for loop for draw one part of the robot	

		}//end for loop for updating and drawing model
		glUseProgram(0);
		glFlush();
	}

	void Scene::LoadModel()
	{
		std::vector<glm::vec3> Kds;
		std::vector<glm::vec3> Kas;
		std::vector<glm::vec3> Kss;
		std::vector<std::string> Materials; // mtl-name
		std::vector<std::string> textures;
		LoadMTL("../../res/Scene/Scene.mtl", Kds, Kas, Kss, Materials, textures);
		for (int i = 0; i < Materials.size(); i++)
		{
			std::string mtlname = Materials[i];
			Mtls.push_back(mtlname);
			KDs[mtlname] = Kds[i];
			KSs[mtlname] = Kss[i];
			KAs[mtlname] = Kas[i];
			Texture tex;
			Textures[mtlname] = tex;
			if (!textures[i].empty())
			{
				int width, height, channels;
				std::string textuePath = "../../res/Scene/" + textures[i];
				printf("Load Textures: %s\n", textuePath.c_str());
				unsigned char* data = stbi_load(textuePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
				
				if (data == nullptr) {
					std::cerr << "Error: Failed to load texture: " << textuePath << std::endl;
					continue;
				}
				glGenTextures(1, &Textures[mtlname].texture);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Textures[mtlname].texture);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				stbi_image_free(data);
				glBindTexture(GL_TEXTURE_2D, 0);

				GLuint tex0Uni = glGetUniformLocation(program, "tex0");
				glUseProgram(program);  // Use program before setting uniform
				glUniform1i(tex0Uni, 0);
			}
		}

		// ¥[¸ü¦U³¡¥ó
		Load2Buffer("../../res/Scene/Scene.obj", Type::Building);           // body

		GLuint totalSize[3] = { 0, 0, 0 };
		GLuint offset[3] = { 0, 0, 0 };
		for (int i = 0; i < SCENESUM; i++)
		{
			totalSize[0] += vertices_size[i] * sizeof(glm::vec3);
			totalSize[1] += uvs_size[i] * sizeof(glm::vec2);
			totalSize[2] += normals_size[i] * sizeof(glm::vec3);
		}

		// ¥Í¦¨ VBO
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &uVBO);
		glGenBuffers(1, &nVBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, totalSize[0], NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, uVBO);
		glBufferData(GL_ARRAY_BUFFER, totalSize[1], NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, nVBO);
		glBufferData(GL_ARRAY_BUFFER, totalSize[2], NULL, GL_STATIC_DRAW);

		for (int i = 0; i < SCENESUM; i++)
		{
			// ½Æ»s³»ÂI¸ê®Æ
			glBindBuffer(GL_COPY_WRITE_BUFFER, VBO);
			glBindBuffer(GL_COPY_READ_BUFFER, VBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[0], vertices_size[i] * sizeof(glm::vec3));
			offset[0] += vertices_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(VBOs[i]); // ÄÀ©ñ VBO
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			// ½Æ»s UV ¸ê®Æ
			glBindBuffer(GL_COPY_WRITE_BUFFER, uVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, uVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[1], uvs_size[i] * sizeof(glm::vec2));
			offset[1] += uvs_size[i] * sizeof(glm::vec2);
			glInvalidateBufferData(uVBOs[i]); // ÄÀ©ñ VBO
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			// ½Æ»sªk½u¸ê®Æ
			glBindBuffer(GL_COPY_WRITE_BUFFER, nVBO);
			glBindBuffer(GL_COPY_READ_BUFFER, nVBOs[i]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, offset[2], normals_size[i] * sizeof(glm::vec3));
			offset[2] += normals_size[i] * sizeof(glm::vec3);
			glInvalidateBufferData(nVBOs[i]); // ÄÀ©ñ VBO
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		}
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	}

	void Scene::Load2Buffer(const char* obj, int i)
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

	void Scene::SetInstance(int instancedNum, int multipleMode) {
		this->instancedNum = instancedNum;
		this->multipleMode = multipleMode;
	}
}
