#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace CG
{
    class Camera
    {
    public:
        Camera();
        ~Camera();

        void LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

        auto GetProjectionMatrix() -> const glm::mat4
        {
            return projection;
        }

        auto GetViewMatrix() -> const glm::mat4
        {
            return view;
        }

        void SetFov(float degree)
        {
            fov = degree;

            UpdateProjectionMatrix();
        }

        void SetAspect(float ratio)
        {
            aspect = ratio;

            UpdateProjectionMatrix();
        }

        void SetClip(float near, float far)
        {
            clipNear = near;
            clipFar = far;

            UpdateProjectionMatrix();
        }

    private:
        void UpdateProjectionMatrix();

    private:
        glm::mat4 projection;
        glm::mat4 view;

        float fov;
        float aspect;
        float clipNear;
        float clipFar;
    };
}
