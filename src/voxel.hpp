#pragma once

#include <cstdint>
#include <fluent/renderer.h>
#include <glm/glm.hpp>
#include "quad.hpp"

struct Voxel
{
	enum Type : uint16_t
	{
		AIR    = 0,
		GRASS  = 1,
		GROUND = 2,
		SAND   = 3,
		GLASS  = 4,
		WOOD   = 5,
		STONE  = 6,
		WATER  = 7,
		LEAVES = 8,
		COUNT
	};
};

void
init_voxel_data_storage();

void
get_uv( glm::vec2& r, Voxel::Type voxel, Face::Type face );

bool
is_transparent( Voxel::Type voxel );
