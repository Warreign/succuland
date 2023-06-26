#include "camera.h"
#include "object.h"

int Camera::refreshRate = 120;

Camera * Camera::active = nullptr;

Camera::Camera(void) 
	: position(glm::vec3(0.0f)), direction(glm::vec3(0.0f)), up(glm::vec3(0.0f)),
	yaw(0.0f), pitch(0.0f), sensitivity(0.0f), speed(0.0f), freeMode(false),
	angle(0.0f), nearPlane(0.0f), farPlane(0.0f),
	elevation(0.0f), radius(0.0f), circling(false),
	lastActive(nullptr) 
{
}

Camera::Camera(glm::vec3 position, glm::vec3 direction, float nearPlane, float farPlane, float captureAngle)
	: position(position), direction(direction), up(glm::vec3(0.0f, 1.0f, 0.0f)),
	yaw(0.0f), pitch(0.0f), sensitivity(1.5f), speed(0.0f), freeMode(false),
	angle(glm::radians(captureAngle)), nearPlane(nearPlane), farPlane(farPlane),
	elevation(0.0f), radius(0.0f), circling(false),
	lastActive(nullptr), locked(true)
{
}
Camera::Camera(glm::vec3 position, glm::vec3 direction, float nearPlane, float farPlane, float captureAngle, float movementSpeed, float width, float length, float up, const Perlin* down)
	: position(position), direction(direction), up(glm::vec3(0.0f, 1.0f, 0.0f)),
	yaw(0.0f), pitch(0.0f), sensitivity(1.5f), speed(movementSpeed), freeMode(false),
	angle(glm::radians(captureAngle)), nearPlane(nearPlane), farPlane(farPlane),
	elevation(0.0f), radius(0.0f), circling(false),
	lastActive(nullptr), locked(false)
{
	circlingParameters(glm::vec3(0.0f, 0.0f, 0.0f), 20.0f, 40.0f);
	initBoundaries(width, length, up, down);
}


void Camera::checkBoundariesAndMove(const glm::vec3& newPos)
{
	bool leftright = (newPos.x < widthBoundary / 2 && newPos.x > -widthBoundary / 2);
	bool frontback = (newPos.z < lengthBoundary / 2 && newPos.z > -lengthBoundary / 2);
	bool down = (newPos.y > (*downBoundary)(newPos.x, newPos.z) + 0.1f);
	bool up = (newPos.y < upBoundary);
	if (leftright && frontback && up && down)
		position = newPos;
	else if (!up)
		Particle::createParticle("textures/cloud.png", position);
}

void Camera::initBoundaries(float width, float length, float up, const Perlin* down)
{
	this->widthBoundary = width;
	this->lengthBoundary = length;
	this->upBoundary = up;
	this->downBoundary = down;
}

void Camera::makeActive() {
	if (active != nullptr) 
	{
		if (active->freeMode) 
			active->toggleFreeMode();
		if (active->circling) 
			active->toggleCircling();
	}
	active = this;
}

/*
*	Free motion functions
*/

void Camera::freeModeMotionCallback(int x, int y) 
{
	float deltaYaw = float(GLUT_WIDTH / 2 - x) * 0.025 * active->sensitivity;
	float deltaPitch = float(GLUT_HEIGHT / 2 - y) * 0.025 * active->sensitivity;
	active->rotateView(deltaYaw, deltaPitch);
	glutWarpPointer(GLUT_WIDTH / 2, GLUT_HEIGHT / 2);
}

void Camera::toggleFreeMode() 
{
	if (freeMode == true) 
	{
		freeMode = false;
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		active = lastActive;
		glutPassiveMotionFunc(nullptr);
	}
	else 
	{ // freeMode == false
		makeActive();
		freeMode = true;
		glutSetCursor(GLUT_CURSOR_NONE);
		glutWarpPointer(GLUT_WIDTH / 2, GLUT_HEIGHT / 2);
		lastActive = active;
		glutPassiveMotionFunc(freeModeMotionCallback);
	}
}

void Camera::rotateView(float dyaw, float dpitch) 
{
	yaw = glm::mod((yaw + dyaw), 360.0f);
	pitch = glm::max(glm::min(pitch + dpitch, 90.0f), -90.0f);

	direction = glm::vec3(
		glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
		glm::sin(glm::radians(pitch)),
		glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
	);

	up = glm::vec3(
		-glm::sin(glm::radians(yaw)) * glm::sin(glm::radians(pitch)),
		 glm::cos(glm::radians(pitch)),
		-glm::cos(glm::radians(yaw)) * glm::sin(glm::radians(pitch))
	);
}

void Camera::moveLeft() 
{
	if (freeMode && !locked)
	{
		glm::vec3 newPos = position - (glm::normalize(glm::cross(direction, up)) * (speed / refreshRate));
		checkBoundariesAndMove(newPos);
	}
}

void Camera::moveRight() 
{
	if (freeMode && !locked)
	{
		glm::vec3 newPos = position + (glm::normalize(glm::cross(direction, up)) * (speed / refreshRate));
		checkBoundariesAndMove(newPos);
	}
}

void Camera::moveForward()
{
	if (freeMode && !locked)
	{
		glm::vec3 newPos = position + (glm::normalize(direction) * (speed / refreshRate));
		checkBoundariesAndMove(newPos);
	}
}

void Camera::moveBackward() 
{
	if (freeMode && !locked)
	{
		glm::vec3 newPos = position - (glm::normalize(direction) * (speed / refreshRate));
		checkBoundariesAndMove(newPos);
	}
}

/*
*	Circling functions
*/

void Camera::circleTimerCallback(int)
{
	float alpha = float(glutGet(GLUT_ELAPSED_TIME)) * 0.001;
	active->position = glm::vec3(glm::sin(alpha) * active->radius, active->elevation, glm::cos(alpha) * active->radius) + active->point;

	if (!active->freeMode) 
	{
		active->direction = glm::normalize(active->point - active->position);
		active->up = glm::normalize(glm::cross(glm::cross(glm::vec3(0.0f, -1.0f, 0.0f), active->direction), active->direction));
	}

	if (active->circling) 
	{
		glutTimerFunc(15, circleTimerCallback, 0);
	}
}


void Camera::circlingParameters(const glm::vec3& point, float elevation, float radius) 
{
	this->point = point;
	this->elevation = elevation;
	this->radius = radius;
}

void Camera::toggleCircling() 
{
	if (locked)
		return;

	if (circling == true) 
	{
		circling = false;
		active = lastActive;
	}
	else 
	{ // circling == false
		makeActive();
		circling = true;
		lastActive = active;
		glutTimerFunc(15, circleTimerCallback, 0);
	}
}

/*
*	View functions
*/

void Camera::updateMatrices() 
{
	this->view = glm::lookAt(position, position + direction, up);
	this->projection = glm::perspective(angle, float(GLUT_WIDTH) / float(GLUT_HEIGHT), nearPlane, farPlane);
}

void Camera::setProjectionParameters(float angle, float nearPlane, float farPlane)
{
	this->angle = glm::radians(angle);
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
}

const glm::mat4& Camera::viewMatrix() const 
{
	 return view;
}

const glm::mat4& Camera::projectMatrix() const 
{
	 return projection;
}


