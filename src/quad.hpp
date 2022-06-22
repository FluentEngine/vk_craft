#pragma once

#include <array>
#include <fluent/renderer.h>
#include <glm/glm.hpp>

struct Face
{
	enum Type : uint8_t
	{
		FRONT,
		BACK,
		LEFT,
		RIGHT,
		BOTTOM,
		TOP,
		COUNT
	};
};

static glm::vec3 front_face[] = {
    { 0.0f, 1.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
};

static glm::vec3 back_face[] = {
    { 1.0f, 1.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
};

static glm::vec3 left_face[] = {
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f },
};

static glm::vec3 right_face[] = {
    { 1.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
};

static glm::vec3 bottom_face[] = {
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 1.0f },
};

static glm::vec3 top_face[] = {
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f },
};
