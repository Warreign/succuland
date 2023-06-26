
#include <iostream>
#include <stdexcept>

#include "pgr.h"
#include "data.h"
#include "shader.h"
#include "geometry.h"
#include "camera.h"
#include "object.h"
#include "properties.h"
#include "parameters.h"

#include <chrono>
#include <fstream>
#include <glm/ext.hpp>

namespace warreign 
{

bool keys[256];
bool skeys[256];
int menu;

// Shaders
Shader* commonShader;
Shader* lightSourceShader;
Shader* skyboxShader;
Shader* bannerShader;
Shader* particleShader;
LightingShader* lightingShader;

// Meshes
Mesh* cubeGeometry;
Mesh* skyboxGeometry;
Mesh* lightCubeGeometry;
Mesh* bannerGeometry;
Mesh* particleGeometry;
TerrainMesh* terrainMesh;
OBJMesh* cactusGeometry;
OBJMesh* arrowMesh;

// Material Properties
Material* brick;
Material* grass;
MaterialMap* sand;
MaterialMap* brickMap;

// Light Properties
PointLight* bulbProperties;
DirectionalLight* sunProperties;
SpotLight* flashlightProperties;

// Fog Properties
Fog* fog;

// Lights
LightObject* sun;
LightObject* flashlight;
std::vector<LightObject*> lights;

// Objects
Skybox* daySkybox;
Banner* banner;
Arrow* arrow;
std::vector<ObjectInstance*> cubes;
std::vector<ObjectInstance*> cacti;
std::vector<ObjectInstance*> objects;
std::vector<Particle*> particles;

// Cameras
Camera camera;
Camera staticCam1;
Camera staticCam2;

bool daytime = true;
bool flashOn = false;

void frameTimerCallback(int)
{
	glutPostRedisplay();
	glutTimerFunc(REFRESH_TIME, frameTimerCallback, 0);
}

/// Timer function to animate spinning arrow
void arrowAnimationTimerCallback(int)
{
	arrow->animationStep();
	glutTimerFunc(REFRESH_TIME, arrowAnimationTimerCallback, 0);
}

void displayCallback()
{
	lightingShader->resetLights();
	Camera& currentCamera = *Camera::active;
	currentCamera.updateMatrices();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearStencil(0);

	if (!daytime && !fog->isVisible)
		glClearColor(NIGHT_SKY_COLOR.r, NIGHT_SKY_COLOR.g, NIGHT_SKY_COLOR.b, 1.0f);
	else 
		glClearColor(fog->color.r, fog->color.g, fog->color.b, 1.0f);

	if (daytime) 
		sun->draw(currentCamera);

	for (auto& light : lights)
		light->draw(currentCamera);

	if (flashOn) 
	{
		flashlight->move(currentCamera.position, currentCamera.direction);
		flashlight->draw(currentCamera);
	}

	int idx = 1;
	for (const auto& c : cacti)
	{
		glStencilFunc(GL_ALWAYS, idx++, -1);
		c->draw(currentCamera);
	}
	glStencilFunc(GL_ALWAYS, 0, -1);

	for (const auto& o : objects)
		o->draw(currentCamera);

	if (daytime && !fog->isVisible) 
		daySkybox->draw(currentCamera);

	if (daytime)
		banner->draw(currentCamera);

	for (const auto& p : Particle::getParticles())
		p->draw(currentCamera);

	if (arrow->currentIdx > 0)
		arrow->draw(currentCamera);

	glutSwapBuffers();
}

void reshapeCallback(int width, int height)
{
	glViewport(0, 0, width, height);
}

void initShaders()
{
	lightingShader = new LightingShader("shaders/phong.vert", "shaders/phong.frag");
	commonShader = new Shader("shaders/standard.vert", "shaders/standard.frag");
	skyboxShader = new Shader("shaders/skybox.vert", "shaders/skybox.frag");
	lightSourceShader = new Shader("shaders/light.vert", "shaders/light.frag");
	bannerShader = new Shader("shaders/banner.vert", "shaders/banner.frag");
	particleShader = new Shader("shaders/particle.vert", "shaders/particle.frag");
}

/// <summary>
/// Function to generate cacti with random positions using seed
/// defined in "parameters.h" 
/// </summary>
/// <param name="count">Number of cacti</param>
/// <param name="width">Width of area around 0.0</param>
/// <param name="length">Length of area around 0.0</param>
/// <param name="perlin">Perlin noise function for height</param>
void genCacti(uint32_t count, const Perlin& perlin, uint32_t width, uint32_t length) 
{
	if (count > 255) {
		throw std::runtime_error("Exceeded maximum number of cacti");
	}
	srand(SEED);

	for (int i = 0; i < count; ++i)
	{
		float x = -(width / 2.0f) + (rand() % width);
		float z = -(length / 2.0f) + (rand() % length);
		glm::mat4 translate = glm::translate(glm::vec3(x, perlin(x,z), z));
		glm::mat4 rotate = glm::rotate( glm::radians(float(rand() % 360)), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 scale = glm::scale(glm::vec3(CACTUS_SCALE));
		cacti.push_back(new ObjectInstance(cactusGeometry, translate * rotate * scale));
	}
}

/// Initialize all of the objects and meshes
void initData()
{
	std::ifstream file("seed.txt");
	file >> SEED;
	std::cout << "SEED: " << SEED << std::endl;
	file.close();

	brick = new MaterialMap(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(0.8f), 256.0, "textures/wall.jpg");
	sand = new MaterialMap(glm::vec3(0.05f, 0.05f, 0.0f), glm::vec3(0.81f, 0.81f, 0.8f), glm::vec3(0.05f, 0.05f, 0.05f), 23.0f, "textures/sand/diffuse.jpg", "textures/sand/specular.png");

	skyboxGeometry = new Mesh(skyboxVertices, nullptr, 12, 8, skyboxShader, 0);
	lightCubeGeometry = new Mesh(vertices, indices, 12, 8, lightSourceShader, NORMAL_BIT);
	bannerGeometry = new Mesh(bannerVertices, nullptr, 2, 4, bannerShader, TEXTURE_BIT);
	arrowMesh = new OBJMesh(ARROW_OBJ_PATH, lightingShader);
	particleGeometry = new Mesh(particleSpriteVertices, nullptr, 2, 4, particleShader, TEXTURE_BIT);

	terrainMesh = new TerrainMesh(TERRAIN_WIDTH, TERRAIN_LENGTH, 0.0, SEED, lightingShader, sand, NORMAL_BIT | TEXTURE_BIT);
	objects.push_back(new ObjectInstance(terrainMesh, glm::scale(glm::vec3(1.0))));

	cactusGeometry = new OBJMesh(CACTUS_OBJ_PATH, lightingShader);
	genCacti(CACTUS_COUNT, terrainMesh->getPerlin(), TERRAIN_WIDTH, TERRAIN_LENGTH);

	bulbProperties = new PointLight(glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f));
	sunProperties = new DirectionalLight(glm::vec3(1.0f), glm::vec3(2.0f), glm::vec3(2.0f));
	flashlightProperties = new SpotLight(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f), glm::cos(glm::radians(120.0f)), 20.0);

	sun = new LightObject(sunProperties, lightingShader, SUN_DIRECTION);
	flashlight = new LightObject(lightCubeGeometry, flashlightProperties, lightingShader, camera.position, camera.direction);

	daySkybox = new Skybox(skyboxGeometry, DAY_SKYBOX_PATH);
	banner = new Banner(bannerGeometry, BANNER_PATH);
	arrow = new Arrow(arrowMesh, ARROW_ELEVATION, ARROW_RADIUS, glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(1, 0, 0)) * glm::scale(glm::vec3(ARROW_SCALE)));

	fog = new Fog(glm::vec3(0.6f), 0.025f, 4.0f);
	Shader::setFog(fog);

	Particle::init(particleGeometry);

	Camera::refreshRate = REFRESH_RATE;
	camera = Camera(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, -1.0f, 1.0f), NEAR_PLANE, FAR_PLANE, CAMERA_ANGLE, 50.0f, TERRAIN_WIDTH, TERRAIN_LENGTH, CAMERA_UPPER_BOUNDARY, &terrainMesh->getPerlin());
	camera.makeActive();

	float x = (rand() % TERRAIN_WIDTH) - TERRAIN_WIDTH / 2.0f;
	float y = 20.0f;
	float z = (rand() % TERRAIN_LENGTH) - TERRAIN_LENGTH / 2.0f;
	staticCam1 = Camera(glm::vec3(x, y, z), glm::vec3(-0.45, -0.15, 0.87), NEAR_PLANE, FAR_PLANE, CAMERA_ANGLE);
	x = (rand() % TERRAIN_WIDTH) - TERRAIN_WIDTH / 2.0f;
	y = 20.0f;
	z = (rand() % TERRAIN_LENGTH) - TERRAIN_LENGTH / 2.0f;
	staticCam2 = Camera(glm::vec3(x, y, z), glm::vec3(-1.0f, -0.5f, 1.0f), NEAR_PLANE, FAR_PLANE, CAMERA_ANGLE);
}

void menuCallback(int value)
{
	std::cout << "Button " << value << " was pressed" << std::endl;
	switch (value)
	{
	case 0:
		staticCam1.makeActive();
		break;
	case 1:
		staticCam2.makeActive();
		break;
	case 2:
		camera.makeActive();
		break;
	case 3:
		if (fog->isEnabled)
			fog->disable();
		else
			fog->enable();
		break;
	case 4:
		daytime = !daytime;
		break;
	}
	if (Camera::active->freeMode) {
		glutWarpPointer(GLUT_WIDTH / 2, GLUT_HEIGHT / 2);
	}
}

void initMenu()
{
	menu = glutCreateMenu(menuCallback);
	glutSetMenu(menu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutAddMenuEntry("Static Camera #1", 0);
	glutAddMenuEntry("Static Camera #2", 1);
	glutAddMenuEntry("Dynamic Camera", 2);
	glutAddMenuEntry("Toggle fog", 3);
	glutAddMenuEntry("Toggle time", 4);
}

/// Timer callback to check input from keyboard
void keysTimerCallback(int)
{
	if (keys['w']) 
	{
		Camera::active->moveForward();
	}
	if (keys['s']) 
	{
		Camera::active->moveBackward();
	}
	if (keys['a']) 
	{
		Camera::active->moveLeft();
	}
	if (keys['d']) 
	{
		Camera::active->moveRight();
	}
	glutTimerFunc(REFRESH_TIME, keysTimerCallback, 0);
}


void specialCallback(int key, int x, int y)
{
	skeys[key] = true;
	switch (key) 
	{
	case GLUT_KEY_F1:
		Camera::active->toggleFreeMode();
		break;
	case GLUT_KEY_F2:
		Camera::active->toggleCircling();
		break;
	case GLUT_KEY_F3:
		flashOn = !flashOn;
		break;
	case GLUT_KEY_F4:
		daytime = !daytime;
		break;
	case GLUT_KEY_F5:
		lights.emplace_back(new LightObject(lightCubeGeometry, bulbProperties, lightingShader, glm::translate(Camera::active->position) * glm::scale(glm::vec3(0.2f))));
		break;
	case GLUT_KEY_F11:
		glutFullScreenToggle();
		break;
	case GLUT_KEY_F7:
		if (Camera::refreshRate == 60)
			Camera::refreshRate = 120;
		else
			Camera::refreshRate = 60;
		break;
	}
}

void specialUpCallback(int key, int x, int y)
{
	skeys[key] = false;
}

void keyboardCallback(unsigned char key, int x, int y)
{
	switch (key) 
	{
	case 27:
		glutLeaveMainLoop();
		break;
	}
	keys[key] = true;
}

void keyboardUpCallback(unsigned char key, int x, int y)
{
	keys[key] = false;
}

void mouseCallback(int button, int state, int x, int y)
{
	if (state != GLUT_DOWN)
		return;

	GLuint idx;
	glReadPixels(x, glutGet(GLUT_WINDOW_HEIGHT) - y - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &idx);

	if (idx > 0) 
	{
		cacti[idx - 1]->addChild(new LightObject(lightCubeGeometry, flashlightProperties, lightingShader, glm::translate(glm::vec3(0.0f, 30.0f, 0.0f)), glm::vec3(0.0f, -1.0f, 0.0f)));
		arrow->target = cacti[idx - 1]->position;
		if (arrow->currentIdx == 0)
			glutTimerFunc(REFRESH_TIME, arrowAnimationTimerCallback, 0);
		arrow->currentIdx = idx;
	}
}

void initApp()
{
	glutInitContextVersion(4, 4);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow(WINDOW_TITLE);

	glutIgnoreKeyRepeat(true);
	glutSpecialFunc(specialCallback);
	glutKeyboardFunc(keyboardCallback);
	glutKeyboardUpFunc(keyboardUpCallback);

	glutMouseFunc(mouseCallback);

	glutDisplayFunc(displayCallback);
	glutReshapeFunc(reshapeCallback);

	glutTimerFunc(REFRESH_TIME, frameTimerCallback, 0);
	glutTimerFunc(REFRESH_TIME, keysTimerCallback, 0);

	if (!pgr::initialize(4, 4, pgr::DEBUG_OFF)) 
	{
		throw std::runtime_error("pgr init error");
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

/// Delete dynamically allocated objects
void cleanup()
{
	delete lightingShader;
	delete lightSourceShader;
	delete commonShader;
	delete skyboxShader;
	delete bannerShader;
	delete particleShader;

	delete bulbProperties;
	delete sunProperties;
	delete flashlightProperties;

	delete flashlight;
	delete sun;
	for (auto& l : lights)
		delete l;

	delete fog;

	delete cubeGeometry;
	delete skyboxGeometry;
	delete lightCubeGeometry;
	delete terrainMesh;
	delete bannerGeometry;
	delete particleGeometry;

	delete grass;
	delete sand;
	delete brick;
	delete brickMap;

	delete daySkybox;
	for (auto& c : cacti)
		delete c;
	for (auto& o : objects)
		delete o;
}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	try 
	{
		warreign::initApp();
		warreign::initShaders();
		warreign::initData();
		warreign::initMenu();
		glutMainLoop();
		warreign::cleanup();
	}
	catch (std::exception& e) 
	{
		pgr::dieWithError(e.what());
	}

	return EXIT_SUCCESS;
}
