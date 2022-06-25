#pragma once

#include <fluent/os.h>
#include <glm/glm.hpp>
#include "constants.hpp"

static inline void
to_voxel_position( glm::ivec3& r, const glm::vec3 position )
{
	auto x = position[ 0 ];
	auto y = position[ 1 ];
	auto z = position[ 2 ];

	r[ 0 ] = int32_t( std::floor( x ) );
	r[ 1 ] = int32_t( std::floor( y ) );
	r[ 2 ] = int32_t( std::floor( z ) );
}

static inline void
to_chunk_position( glm::ivec3& r, const glm::vec3 position )
{
	int32_t x = position[ 0 ];
	int32_t y = position[ 1 ];
	int32_t z = position[ 2 ];

	r[ 0 ] = x < 0 ? ( ( ++x - CHUNK_SIZE ) / CHUNK_SIZE ) : ( x / CHUNK_SIZE );
	r[ 1 ] = y < 0 ? ( ( ++y - CHUNK_SIZE ) / CHUNK_SIZE ) : ( y / CHUNK_SIZE );
	r[ 2 ] = z < 0 ? ( ( ++z - CHUNK_SIZE ) / CHUNK_SIZE ) : ( z / CHUNK_SIZE );
}

static inline void
global_voxel_to_local( glm::ivec3& r, const glm::ivec3 position )
{
	r[ 0 ] = ( CHUNK_SIZE + ( position[ 0 ] % CHUNK_SIZE ) ) % CHUNK_SIZE;
	r[ 1 ] = ( CHUNK_SIZE + ( position[ 1 ] % CHUNK_SIZE ) ) % CHUNK_SIZE;
	r[ 2 ] = ( CHUNK_SIZE + ( position[ 2 ] % CHUNK_SIZE ) ) % CHUNK_SIZE;
}

static inline void
local_voxel_to_global( glm::ivec3&      r,
                       const glm::ivec3 chunk_pos,
                       const glm::ivec3 voxel_pos )
{
	r[ 0 ] = chunk_pos[ 0 ] * CHUNK_SIZE + voxel_pos[ 0 ];
	r[ 1 ] = chunk_pos[ 1 ] * CHUNK_SIZE + voxel_pos[ 1 ];
	r[ 2 ] = chunk_pos[ 2 ] * CHUNK_SIZE + voxel_pos[ 2 ];
}
