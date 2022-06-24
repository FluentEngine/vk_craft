#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <unordered_map>
#include <list>
#include <glm/gtx/hash.hpp>
#include "chunk.hpp"

class MeshGenerator;

class ChunkManager
{
private:
	MeshGenerator* mesh_generator;

	glm::ivec3 last_update_position;
	bool       world_changed_last_frame;

	std::unordered_map<glm::ivec3, Chunk> chunks;

	size_t frame_count;

	bool
	need_update_chunks( const glm::ivec3& ) const;

	void
	ensure_neighbors( const glm::ivec3& position,
	                  const glm::ivec3& chunk_position );

public:
	void
	init( MeshGenerator* );

	void
	update_visible_chunks( const glm::vec3& position );

	Voxel::Type
	get_voxel( const glm::ivec3& ) const;

	void
	set_voxel( const glm::ivec3& position, Voxel::Type voxel );
};
