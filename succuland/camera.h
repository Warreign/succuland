#pragma once

#ifndef _CAMERA_H
#define _CAMERA_H

#include "pgr.h"
#include "perlin.h"

#include <iostream>
#include <glm/ext.hpp>


/// Macros to simplify getting width and height of the window
#define GLUT_WIDTH glutGet(GLUT_WINDOW_WIDTH)
#define GLUT_HEIGHT glutGet(GLUT_WINDOW_HEIGHT)

class Camera 
{
protected:
	/// Passive motion callback function to update camera parameters while in free moving mode
	static void freeModeMotionCallback(int x, int y);
	/// Timer callback function to update camera parameters while in spinning mode
	static void circleTimerCallback(int);

	/// Camera to return to after finished free mode or spinning mode
	Camera* lastActive;

	// Projection parameters
	float angle;
	float farPlane;
	float nearPlane;

	// Free mode parameters
	float yaw;
	float pitch;
	float sensitivity;
	float speed;

	// Circling parameters
	bool circling;
	glm::vec3 point;
	float elevation;
	float radius;

	/// Boolean value for whether the camera can use free or spinning mode
	bool locked;

	// View matrices
	glm::mat4 view;
	glm::mat4 projection;

	// Boundaries
	float widthBoundary;
	float lengthBoundary;
	float upBoundary;
	const Perlin* downBoundary;

	/// <summary>
	/// Check if the camera can move to new position and if so, move it there
	/// </summary>
	/// <param name="newPosition">position to check</param>
	void checkBoundariesAndMove(const glm::vec3& newPosition);

	/// <summary>
	/// Initialize boundaries for camera
	/// </summary>
	/// <param name="width">Width around 0.0</param>
	/// <param name="length">Height around 0.0</param>
	/// <param name="up">Upper boundary</param>
	/// <param name="down">Lower boundary perlin noise function</param>
	void initBoundaries(float width, float length, float up, const Perlin* down);

public:
	static int refreshRate;
	bool freeMode;

	/// Up vector
	glm::vec3 up;
	/// Direction camera is facing
	glm::vec3 direction;
	/// Position of the camera
	glm::vec3 position;

	/// Currently active camera
	static Camera* active;

	Camera();
	/// Initialize static camera
	Camera(glm::vec3 position, glm::vec3 direction, float nearPlane, float farPlane, float captureAngle);
	/// Initialize dynamic camera with bounds
	Camera(glm::vec3 position, glm::vec3 direction, float nearPlane, float farPlane, float captureAngle, float movementSpeed, float width, float length, float up, const Perlin* down);

	/// Make current camera active
	void makeActive();

	/// Change projection parameters
	void setProjectionParameters(float angle, float nearPlane, float farPlane);

	/// Rotate camera using 2 delta angles, roll not supported
	void rotateView(float dyaw, float dpitch);

	/// Toggle free movement mode
	void toggleFreeMode();
	void moveLeft();
	void moveRight();
	void moveForward();
	void moveBackward();
	
	/// <summary>
	/// Change the parameters of how the camera is going to spin
	/// </summary>
	/// <param name="point">Point of focus</param>
	/// <param name="elevation">Elevation above the point</param>
	/// <param name="radius">Radius of spinning</param>
	void circlingParameters(const glm::vec3& point, float elevation, float radius);
	void toggleCircling();

	/// Update view and projection matrices with current camera parameters
	void updateMatrices();

	const glm::mat4& viewMatrix() const;
	const glm::mat4& projectMatrix() const;
};

#endif