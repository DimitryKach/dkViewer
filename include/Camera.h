#pragma once
#include "utils.h"
#include "Eigen/Dense"
#include "GLFW/glfw3.h"

class Camera
{
public:
	float moveSpeed;
	float yaw;
	float pitch;
	float roll;
	float zoom;
	Eigen::Vector3f position;
	Eigen::Vector3f centerOfInterest;
	Eigen::Vector3f target;
	Eigen::Vector3f up;
	Eigen::Matrix4f projectionMtx;
	bool doRotate;
	bool doPan;
	bool doDolly;
	bool doZoom;
	bool active;
	float FOV;
	float ASPECT_RATIO;
	float NEAR;
	float FAR;
	float TIME_STATE_MULT;

	Eigen::Matrix4f getMtx();
	void zoomFwd(const float delta);
	void moveFwd(const float delta);
	void zoomBwd(const float delta);
	void moveBwd(const float delta);
	void moveLeft(const float delta);
	void moveRight(const float delta);
	void moveUp(const float delta);
	void moveDown(const float delta);
	void resetCOI();
	void rotate(float inYaw, float inPitch, float inRoll);
	void handleKeyInputs(int key);
	void handleMouseButtonInputs(int button, int action);
	void handleMouseMotion(double inX, double inY);
	void handleMouseScroll(double amount);
	void rotateTarget();
	void updateLastPos(const float& xpos, const float& ypos);
	void updateProjMtx();
	void setActionMul(float&& val);

	Camera()
	{
		position = Eigen::Vector3f(0.0f, 0.0f, 5.0f);
		target = position;
		centerOfInterest = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
		up = Eigen::Vector3f::UnitY();
		moveSpeed = 0.05f;
		yaw = 0;
		pitch = 0;
		roll = 0;
		doRotate = false;
		doPan = false;
		active = false;
		lastX = 0.0f;
		lastY = 0.0f;
		actionMul = 1.0f;
	}
	~Camera() = default;
private:
	float lastX;
	float lastY;
	float actionMul;
	Eigen::Matrix4f CalcProjectionMtx();
};

