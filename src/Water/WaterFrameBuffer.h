#pragma once

#include <array>
#include <string>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace CG
{
	class WaterFrameBuffer
	{
	public:
		WaterFrameBuffer();
		~WaterFrameBuffer();

		void bindReflectionFrameBuffer();
		void bindRefractionFrameBuffer();
		void unbindCurrentFrameBuffer(int screenWidth, int screenHeight);

		void cleanUp() {
			glDeleteFramebuffers(1, &reflectionFBO);
			glDeleteTextures(1, &reflectionTexture);
			glDeleteRenderbuffers(1, &reflectionDepthBuffer);

			glDeleteFramebuffers(1, &refractionFBO);
			glDeleteTextures(1, &refractionTexture);
			glDeleteTextures(1, &refractionDepthTexture);
		}
		GLuint getReflectionTexture() const {
			return reflectionTexture;
		}
		GLuint getRefractionTexture() const {
			return refractionTexture;
		}
		GLuint getRefractionDepthTexture() const {
			return refractionDepthTexture;
		}
	private:
		void initialiseReflectionFrameBuffer();
		void initialiseRefractionFrameBuffer();

		void bindFrameBuffer(unsigned int frameBuffer, int width, int height);
		GLuint createFrameBuffer();
		GLuint createTextureAttachment(int width, int height);
		GLuint createDepthTextureAttachment(int width, int height);
		GLuint createDepthBufferAttachment(int width, int height);
	private:
		static const int REFLECTION_WIDTH = 320;
		static const int REFLECTION_HEIGHT = 180;

		static const int REFRACTION_WIDTH = 1280;
		static const int REFRACTION_HEIGHT = 720;

		GLuint reflectionFBO;
		GLuint reflectionDepthBuffer;
		GLuint reflectionTexture;
		
		GLuint refractionFBO;
		GLuint refractionTexture;
		GLuint refractionDepthTexture;
	};
}