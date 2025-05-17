#include "./WaterFrameBuffer.h"

namespace CG
{
	WaterFrameBuffer::WaterFrameBuffer() {
		initialiseReflectionFrameBuffer();
		initialiseRefractionFrameBuffer();
	}

    WaterFrameBuffer::~WaterFrameBuffer() {
        glDeleteFramebuffers(1, &reflectionFBO);
        glDeleteTextures(1, &reflectionTexture);
        glDeleteRenderbuffers(1, &reflectionDepthBuffer);

        glDeleteFramebuffers(1, &reflectionFBO);
        glDeleteTextures(1, &refractionTexture);
        glDeleteTextures(1, &refractionDepthTexture);
    }

	void WaterFrameBuffer::bindReflectionFrameBuffer() {
		bindFrameBuffer(reflectionFBO, REFLECTION_WIDTH, REFLECTION_HEIGHT);
	}

	void WaterFrameBuffer::bindRefractionFrameBuffer() {
		bindFrameBuffer(reflectionFBO, REFRACTION_WIDTH, REFRACTION_HEIGHT);
	}

	void WaterFrameBuffer::unbindCurrentFrameBuffer(int screenWidth, int screenHeight) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);
	}

	void WaterFrameBuffer::initialiseReflectionFrameBuffer() {
		reflectionFBO = createFrameBuffer();
		reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
		reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
		unbindCurrentFrameBuffer(1280, 720);
	}

	void WaterFrameBuffer::initialiseRefractionFrameBuffer() {
		refractionFBO = createFrameBuffer();
		refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
		refractionDepthTexture = createDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
		unbindCurrentFrameBuffer(1280, 720);
	}

	void WaterFrameBuffer::bindFrameBuffer(unsigned int frameBuffer, int width, int height) {
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, width, height);
	}

	GLuint WaterFrameBuffer::createFrameBuffer() {
		GLuint FBO;
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		return FBO;
	}

	GLuint WaterFrameBuffer::createTextureAttachment(int width, int height) {
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		return texture;
	}

	GLuint WaterFrameBuffer::createDepthTextureAttachment(int width, int height) {
		GLuint depthTexture;
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
		return depthTexture;
	}

	GLuint WaterFrameBuffer::createDepthBufferAttachment(int width, int height) {
		GLuint depthBuffer;
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
		return depthBuffer;
	}
}
