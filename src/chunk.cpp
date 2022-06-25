#include <algorithm>
#include <glm/gtc/noise.hpp>
#include "coordinates.hpp"
#include "chunk_manager.hpp"
#include "chunk.hpp"

static inline Voxel::Type
height_to_voxel( int32_t height, int32_t top )
{
	if ( height < 2 )
	{
		return Voxel::WATER;
	}
	else if ( height < 5 )
	{
		return Voxel::SAND;
	}
	else if ( height < 7 )
	{
		return Voxel::STONE;
	}
	else if ( height < top - 1 )
	{
		return Voxel::GROUND;
	}

	return Voxel::GRASS;
}

void
Chunk::init( const glm::vec3 pos, ChunkManager* manager, size_t last_access_frame )
{
	chunk_manager           = manager;
	this->last_access_frame = last_access_frame;

	position[ 0 ] = pos[ 0 ];
	position[ 1 ] = pos[ 1 ];
	position[ 2 ] = pos[ 2 ];

	for ( int32_t x = 0; x < CHUNK_SIZE; x++ )
	{
		for ( int32_t z = 0; z < CHUNK_SIZE; z++ )
		{
			float noise = ( perlin( glm::vec2( x + pos.x * CHUNK_SIZE,
			                                   z + pos.z * CHUNK_SIZE ) /
			                        float( CHUNK_SIZE * 2 ) ) +
			                1.0f ) /
			              2.0f;
			int32_t height =
			    std::clamp( int32_t( noise * CHUNK_SIZE ), 2, CHUNK_SIZE );
			for ( int32_t y = 0; y < height; y++ )
			{
				auto idx    = x + CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z;
				data[ idx ] = height_to_voxel( y, height );
			}
		}
	}
}

Voxel::Type
Chunk::safe_get_voxel( const glm::ivec3 block_position ) const
{
	if ( block_position.x < 0 || block_position.x >= CHUNK_SIZE ||
	     block_position.y < 0 || block_position.y >= CHUNK_SIZE ||
	     block_position.z < 0 || block_position.z >= CHUNK_SIZE )
	{
		return Voxel::AIR;
	}
	else
	{
		return get_voxel( block_position );
	}
}

Voxel::Type
Chunk::get_voxel( const glm::ivec3 block_position ) const
{
	size_t idx = block_position[ 0 ] +
	             CHUNK_SIZE * CHUNK_SIZE * block_position[ 1 ] +
	             CHUNK_SIZE * block_position[ 2 ];

	return data[ idx ];
}

void
Chunk::set_voxel( const glm::ivec3 block_position, Voxel::Type voxel )
{
	size_t idx = block_position[ 0 ] +
	             CHUNK_SIZE * CHUNK_SIZE * block_position[ 1 ] +
	             CHUNK_SIZE * block_position[ 2 ];

	data[ idx ] = voxel;
	modified    = true;
}

std::array<Voxel::Type, Face::COUNT>
Chunk::get_neighbors( const glm::ivec3 block_position ) const
{
	std::array<Voxel::Type, Face::COUNT> result;
	result.fill( Voxel::AIR );

	for ( uint32_t i = 0; i < Face::COUNT; i++ )
	{
		switch ( i )
		{
		case Face::FRONT:
		{
			if ( block_position.z == CHUNK_SIZE - 1 )
			{
				glm::ivec3 pos;
				local_voxel_to_global( pos, position, block_position );
				pos += glm::ivec3( 0, 0, 1 );

				result[ i ] = chunk_manager->get_voxel( pos );
			}
			else
			{
				result[ i ] =
				    get_voxel( block_position + glm::ivec3( 0, 0, 1 ) );
			}
			break;
		}
		case Face::BACK:
		{
			if ( block_position.z == 0 )
			{
				glm::ivec3 pos;
				local_voxel_to_global( pos, position, block_position );
				pos += glm::ivec3( 0, 0, -1 );
				result[ i ] = chunk_manager->get_voxel( pos );
			}
			else
			{
				result[ i ] =
				    get_voxel( block_position + glm::ivec3( 0, 0, -1 ) );
			}
			break;
		}
		case Face::LEFT:
		{
			if ( block_position.x == 0 )
			{
				glm::ivec3 pos;
				local_voxel_to_global( pos, position, block_position );
				pos += glm::ivec3( -1, 0, 0 );
				result[ i ] = chunk_manager->get_voxel( pos );
			}
			else
			{
				result[ i ] =
				    get_voxel( block_position + glm::ivec3( -1, 0, 0 ) );
			}
			break;
		}
		case Face::RIGHT:
		{
			if ( block_position.x == CHUNK_SIZE - 1 )
			{
				glm::ivec3 pos;
				local_voxel_to_global( pos, position, block_position );
				pos += glm::ivec3( 1, 0, 0 );
				result[ i ] = chunk_manager->get_voxel( pos );
			}
			else
			{
				result[ i ] =
				    get_voxel( block_position + glm::ivec3( 1, 0, 0 ) );
			}
			break;
		}
		case Face::BOTTOM:
		{
			if ( block_position.y == 0 )
			{
				glm::ivec3 pos;
				local_voxel_to_global( pos, position, block_position );
				pos += glm::ivec3( 0, -1, 0 );
				result[ i ] = chunk_manager->get_voxel( pos );
			}
			else
			{
				result[ i ] =
				    get_voxel( block_position + glm::ivec3( 0, -1, 0 ) );
			}
			break;
		}
		case Face::TOP:
		{
			if ( block_position.y == CHUNK_SIZE - 1 )
			{
				glm::ivec3 pos;
				local_voxel_to_global( pos, position, block_position );
				pos += glm::ivec3( 0, 1, 0 );
				result[ i ] = chunk_manager->get_voxel( pos );
			}
			else
			{
				result[ i ] =
				    get_voxel( block_position + glm::ivec3( 0, 1, 0 ) );
			}
			break;
		}
		default: break;
		}
	}

	return result;
}
