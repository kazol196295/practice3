//
//  basic_camera.h
//  test
//
//  Created by Nazirul Hasan on 10/9/23.
//  modified by Badiuzzaman on 3/11/24.
//  Modified by Assistant to support dynamic movement and rotation on 11/19/24.
//  Further modified to initially point the camera at the origin on 11/20/24.
//

#ifndef basic_camera_h
#define basic_camera_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class BasicCamera {
public:

    glm::vec3 Position;
    glm::vec3 Up;
    glm::vec3 Direction;
    glm::vec3 Right;

    float Yaw, Pitch, Roll;
    float Zoom, MouseSensitivity, MovementSpeed, RotationSpeed;

    BasicCamera(float posX = 0.0, float posY = 3.0, float posZ = 3.0,
        float lookAtX = 0.0, float lookAtY = 0.0, float lookAtZ = 0.0,
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        Position = glm::vec3(posX, posY, posZ);
        Up = upVector;

        // Set Yaw and Pitch so that the camera looks at the origin initially
        Yaw = -135.0f;   // Rotate 45 degrees left from the default -Z axis to point towards the origin
        Pitch = -45.0f;  // Rotate 45 degrees downward to look at the origin from above
        Roll = 0.0f;

        MovementSpeed = 1.0f;
        RotationSpeed = 30.0f;  // Degrees per second
        MouseSensitivity = 0.1f;
        Zoom = 45.0;

        // Update initial direction based on yaw and pitch values
        updateCameraVectors();
    }

    glm::mat4 createViewMatrix() {
        return glm::lookAt(Position, Position + Direction, Up);
    }

    // Update the camera direction based on the current Yaw, Pitch, and Roll
    void updateCameraVectors() {
        // Calculate the new direction vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Direction = glm::normalize(front);

        // Calculate the Right vector considering Roll
        Right = glm::normalize(glm::cross(Direction, glm::vec3(0.0f, 1.0f, 0.0f)));
        Right = glm::rotate(glm::mat4(1.0f), glm::radians(Roll), Direction) * glm::vec4(Right, 0.0f);

        // Recalculate the Up vector
        Up = glm::normalize(glm::cross(Right, Direction));
    }

    // Process keyboard movement
    void ProcessKeyboard(char direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == 'W')
            Position += Direction * velocity;  // Forward
        if (direction == 'S')
            Position -= Direction * velocity;  // Backward
        if (direction == 'A')
            Position -= Right * velocity;  // Left
        if (direction == 'D')
            Position += Right * velocity;  // Right
        if (direction == 'E')
            Position += Up * velocity;  // Up
        if (direction == 'R')
            Position -= Up * velocity;  // Down
    }

    // Process camera rotation based on Yaw, Pitch, and Roll
    void ProcessRotation(char axis, float deltaTime) {
        float angle = RotationSpeed * deltaTime;
        if (axis == 'P')  // Pitch up
            Pitch += angle;
        if (axis == 'N')  // Pitch down
            Pitch -= angle;
        if (axis == 'Y')  // Yaw left
            Yaw -= angle;
        if (axis == 'H')  // Yaw right
            Yaw += angle;
        if (axis == 'R')  // Roll clockwise
            Roll += angle;
        if (axis == 'L')  // Roll counter-clockwise
            Roll -= angle;

        // Limit Pitch to avoid gimbal lock
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }
};

#endif /* basic_camera_h */
#pragma once