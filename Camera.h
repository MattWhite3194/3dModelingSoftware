#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>

// Default camera values
const float YAW = -90.0f;
const float PITCH = -45.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.2f;
const float ZOOM = 1.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 ZoomPosition;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity = 2.0f;
    float PanSpeed = 0.001f;
    float Zoom = 5.0f;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, 0.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        Position = position;
        ZoomPosition = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(ZoomPosition, Position + Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessMousePanning(float xoffset, float yoffset) {
        Position += (Right * -(PanSpeed * Zoom * xoffset));
        Position += (Up * -(PanSpeed * Zoom * yoffset));
        updateCameraVectors();
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    void SetFocus(glm::vec3 position, float zoom) {
        Position = position;
        Zoom = zoom;
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        float baseSpeed = 0.4f; // tweak this constant
        float zoomSpeed = baseSpeed * glm::length(Front);
        Zoom += zoomSpeed * -yoffset;
        if (Zoom < 0.01) {
            Zoom = 0.01;
        }
        updateCameraVectors();
    }

    void GetMouseRay(int mouseX, int mouseY, int viewportWidth, int viewportHeight, const glm::mat4& projection, glm::vec3& outDir, glm::vec3& outOrigin ) {
        float x = (2.0f * mouseX) / viewportWidth - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / viewportHeight;

        glm::vec4 rayClip(x, y, -1.0f, 1.0f);

        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

        outDir = glm::normalize(
            glm::vec3(glm::inverse(GetViewMatrix()) * rayEye)
        );
        outOrigin = ZoomPosition;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Pitch)) * cos(glm::radians(-Yaw));
        front.y = cos(glm::radians(Pitch)) * sin(glm::radians(-Yaw));
        front.z = sin(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
        ZoomPosition = Position - Front * Zoom;
    }
};
#endif