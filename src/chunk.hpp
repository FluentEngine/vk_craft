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
	glm::ivec3                            position;
	std::array<Voxel::Type, CHUNK_SIZE_CUBED> data;
	mutable bool                          modified = false;
	mutable size_t                        last_access_frame;

	void
	init( const glm::vec3 position, ChunkManager*, size_t );

	Voxel::Type
	safe_get_voxel( const glm::ivec3 block_position ) const;

	Voxel::Type
	get_voxel( const glm::ivec3 ) const;
	void
	set_voxel( const glm::ivec3 block_position, Voxel::Type voxel );

	std::array<Voxel::Type, Face::COUNT>
	get_neighbors( const glm::ivec3 block_position ) const;
};
