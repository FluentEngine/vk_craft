#pragma once

#include <cstdint>
#include <array>
#include <fluent/os.h>
#include "constants.hpp"
#include "quad.hpp"
#include "voxel.hpp"

struct ChunkManager;

struct Chunk
{
	ChunkManager*                         chunk_manager;
	glm::ivec3                                  position;
	std::array<Voxel::Type, CHUNK_VOLUME> data;
	mutable bool                          modified = false;

	void
	init( const glm::vec3 position, ChunkManager* );

	Voxel::Type
	get_voxel( const glm::ivec3 ) const;
	void
	set_voxel( const glm::ivec3 block_position, Voxel::Type voxel );

	std::array<Voxel::Type, Face::COUNT>
	get_neighbors( const glm::ivec3 block_position ) const;
};
