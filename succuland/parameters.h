#pragma once

#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#include "pgr.h"
#include <chrono>
#include <string>
#include <fstream>

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 800;
static const char* WINDOW_TITLE = "Succuland";
#define REFRESH_TIME unsigned int(float(1000) / Camera::refreshRate) // ms

auto timeBegin = std::chrono::steady_clock::now();
long frameCount = 0;

const float FAR_PLANE = 150.0f;
const float NEAR_PLANE = 0.01f;
const float CAMERA_ANGLE = 80.0f;
const uint32_t REFRESH_RATE = 60;

uint64_t SEED;

const glm::vec3 SUN_DIRECTION = { 2.0f, 2.0f, 3.0f };

//const float TERRAIN_SCALE = 1.0f;
const uint32_t TERRAIN_WIDTH = 500;
const uint32_t TERRAIN_LENGTH = 500;

const uint32_t CAMERA_UPPER_BOUNDARY = 80.0f;

const glm::vec3 NIGHT_SKY_COLOR = {0.0f, 0.0f, 0.1f};
const std::string DAY_SKYBOX_PATH = "textures/desertsky";

const std::string CACTUS_OBJ_PATH = "objects/Cactus/Cactus.obj";
const uint32_t CACTUS_COUNT = 150;
const float CACTUS_SCALE = 1.0f; 

const std::string ARROW_OBJ_PATH = "objects/arrow/Arrow5.obj";
const float ARROW_ELEVATION = 13.0f;
const float ARROW_RADIUS = 7.0f;
const float ARROW_SCALE = 0.2f;

const std::string BANNER_PATH = "textures/banner.png";

const std::string CLOUD_PARTICLE_PATH = "textures/cloud.png";

#endif