#include "Camera.h"
#include <iostream>

void Camera::zoomFwd(const float delta)
{
    position -= Eigen::Vector3f::UnitZ() * moveSpeed * delta;
    if (position.z() <= 0.0f)
        position.z() = 0.0f;
}

void Camera::moveFwd(const float delta)
{
    centerOfInterest -= target * moveSpeed * delta;
}

void Camera::zoomBwd(const float delta)
{
    position += Eigen::Vector3f::UnitZ() * moveSpeed * delta;
}

void Camera::moveBwd(const float delta)
{
    centerOfInterest += target * moveSpeed * delta;
}

void Camera::moveLeft(const float delta)
{
    Eigen::Vector3f sideDir = up.cross(target);
    centerOfInterest -= sideDir * moveSpeed * delta;
}

void Camera::moveRight(const float delta)
{
    Eigen::Vector3f sideDir = up.cross(target);
    centerOfInterest += sideDir * moveSpeed * delta;
}

void Camera::moveUp(const float delta)
{
    centerOfInterest += up * moveSpeed * delta;
}

void Camera::moveDown(const float delta)
{
    centerOfInterest -= up * moveSpeed * delta;
}

void Camera::resetCOI()
{
    centerOfInterest = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
}

void Camera::handleKeyInputs(int key)
{
    // TODO: switch between Maya-like camera and FPS. Then we use WASD keys.
    switch (key) {
    case GLFW_KEY_W:
        moveFwd(0.5f);
        break;
    case GLFW_KEY_S:
        moveBwd(0.5f);
        break;
    case GLFW_KEY_A:
        moveLeft(0.5f);
        break;
    case GLFW_KEY_D:
        moveRight(0.5f);
        break;
    case GLFW_KEY_F:
        resetCOI();
        break;
    }
}

void Camera::handleMouseButtonInputs(int button, int action)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        active = true;
        doRotate = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        active = false;
        doRotate = false;
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        active = true;
        doPan = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        active = false;
        doPan = false;
    }

}

Eigen::Matrix4f Camera::getMtx()
{
    Eigen::Matrix4f outMtx;
    outMtx.setIdentity();

    Eigen::Vector3f N = target;
    N.normalize();

    Eigen::Vector3f UpNorm = up;
    UpNorm.normalize();

    Eigen::Vector3f U;
    U = UpNorm.cross(N);
    U.normalize();

    Eigen::Vector3f V = N.cross(U);

    Eigen::Matrix4f coiMtx;
    coiMtx.setIdentity();
    coiMtx.block<3, 1>(0, 3) = -centerOfInterest;

    outMtx.setIdentity();
    outMtx.block<1, 3>(0, 0) = U;
    outMtx.block<1, 3>(1, 0) = V;
    outMtx.block<1, 3>(2, 0) = N;
    outMtx.block<3, 1>(0, 3) = - position;
    outMtx = outMtx * coiMtx;
    return outMtx;
}

void Camera::handleMouseMotion(double inX, double inY)
{
    float dX = (float)(inX - (double)lastX);
    float dY = (float)(inY - (double)lastY);
    lastX = (float)inX;
    lastY = (float)inY;

    if (doRotate)
    {
        pitch += -dY * moveSpeed * 10.0f;
        yaw += -dX * moveSpeed * 10.0f;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        rotateTarget();
    }
    else if (doPan)
    {
        dX = dX * 0.05f;
        dY = dY * 0.05f;
        if (dX < 0.0f)
            moveRight(-dX);
        else if (dX > 0.0f)
            moveLeft(dX);
        if (dY < 0.0f)
            moveDown(-dY);
        else if (dY > 0.0f)
            moveUp(dY);
    }
}

void Camera::handleMouseScroll(double amount)
{
    if (amount > 0.0)
        zoomFwd((float)amount);
    else if (amount < 0.0)
        zoomBwd(-(float)amount);
}

void Camera::rotateTarget()
{
    Eigen::Vector3f _up= Eigen::Vector3f::UnitY();
    Eigen::Vector3f _side= Eigen::Vector3f::UnitX();
    // Apply yaw
    Eigen::Matrix3f rY = Eigen::AngleAxisf(ToRadian(yaw), Eigen::Vector3f::UnitY()).matrix();
    _side = rY * _side;
    Eigen::Vector3f _target = _side.cross(_up);

    // Apply pitch
    Eigen::Matrix3f rX = Eigen::AngleAxisf(ToRadian(pitch), _side).matrix();
    _target = rX * _target;
    _up = rX * _up;

    target = _target;
    up = _up;
}

void Camera::updateLastPos(const float& xpos, const float& ypos)
{
    lastX = xpos;
    lastY = ypos;
}

void Camera::updateProjMtx()
{
    projectionMtx = CalcProjectionMtx();
}

Eigen::Matrix4f Camera::CalcProjectionMtx()
{
    float tanHalfFOV = tanf(ToRadian(FOV / 2.0f));
    float d = 1.0f / tanHalfFOV;
    float A = (-FAR - NEAR) / (FAR - NEAR);
    float B = -(2 * FAR * NEAR) / (FAR - NEAR);

    Eigen::Matrix4f pm;
    pm <<
        d / ASPECT_RATIO, 0.0f, 0.0f, 0.0f,
        0.0f, d, 0.0f, 0.0f,
        0.0f, 0.0f, A, B,
        0.0f, 0.0f, -1.0f, 0.0f;

    return pm;
}