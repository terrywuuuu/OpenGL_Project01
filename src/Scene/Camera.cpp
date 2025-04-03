#include "Camera.h"

namespace CG
{
    Camera::Camera()
    {
        projection = glm::mat4(1.0);
        view = glm::mat4(1.0);

        fov = 80.0f;
        aspect = 4.0f / 3.0f;
        clipNear = 0.01f;
        clipFar = 1000.0f;

        UpdateProjectionMatrix();
    }

    Camera::~Camera()
    {

    }

    void Camera::LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
    {
        view = glm::lookAt(eye, center, up);
    }

    void Camera::UpdateProjectionMatrix()
    {
        projection = glm::perspective(glm::radians(fov), aspect, clipNear, clipFar);
    }
}