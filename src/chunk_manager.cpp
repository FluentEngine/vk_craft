#include "coordinates.hpp"
#include "mesh_generator.hpp"
#include "chunk_manager.hpp"

void
ChunkManager::init( MeshGenerator* mesh_generator )
{
	this->mesh_generator     = mesh_generator;
	last_update_position     = glm::ivec3( 0, 0, 0 );
	world_changed_last_frame = false;
	frame_count              = 0;
}

bool
ChunkManager::need_update_chunks( const glm::ivec3& chunk_position ) const
{
	glm::ivec3 v = last_update_position - chunk_position;

	return ( v.x * v.x + v.z * v.z <
	         CHUNKS_IN_RENDER_DISTANCE * CHUNKS_IN_RENDER_DISTANCE ) ||
	       world_changed_last_frame;
}

void
ChunkManager::update_visible_chunks( const glm::vec3& position )
{
	if ( ( frame_count % 10 ) == 0 )
	{
		bool erase_something = false;
		for ( auto it = chunks.begin(); it != chunks.end(); )
		{
			if ( frame_count - it->second.last_access_frame > 10 )
			{
				it              = chunks.erase( it );
				erase_something = true;
			}
			else
			{
				it++;
			}
		}

		if ( erase_something )
		{
			chunks.rehash( 0 );
		}
	}

	mesh_generator->reset();

	glm::ivec3 chunk_position;
	to_chunk_position( chunk_position, position );

	if ( need_update_chunks( chunk_position ) == false )
	{
		return;
	}

	std::list<Chunk*> chunks_to_push;

	for ( int32_t z = -CHUNKS_IN_RENDER_DISTANCE; z < CHUNKS_IN_RENDER_DISTANCE;
	      z++ )
	{
		for ( int32_t x = -CHUNKS_IN_RENDER_DISTANCE;
		      x < CHUNKS_IN_RENDER_DISTANCE;
		      x++ )
		{
			glm::ivec3 spawn_position = chunk_position + glm::ivec3( x, 0, z );
			spawn_position.y          = -2;

			bool   new_chunk = chunks.find( spawn_position ) == chunks.cend();
			Chunk& chunk     = chunks[ spawn_position ];

			if ( new_chunk )
			{
				chunk.init( spawn_position, this, frame_count );
			}

			chunk.last_access_frame = frame_count;
			chunks_to_push.push_back( &chunk );
		}
	}

	// need this for face removing proper work
	for ( auto chunk : chunks_to_push )
	{
		mesh_generator->push_chunk( *chunk );
	}

	last_update_position     = chunk_position;
	world_changed_last_frame = false;
	frame_count++;
}

Voxel::Type
ChunkManager::get_voxel( const glm::ivec3& position ) const
{
	glm::ivec3 chunk_position;
	to_chunk_position( chunk_position, position );
	auto it = chunks.find( chunk_position );
	if ( it == chunks.cend() )
	{
		return Voxel::AIR;
	}
	else
	{
		glm::ivec3 local;
		global_voxel_to_local( local, position );
		it->second.last_access_frame = frame_count;
		return it->second.get_voxel( local );
	}
}

void
ChunkManager::ensure_neighbors( const glm::ivec3& position,
                                const glm::ivec3& chunk_position )
{
	glm::ivec3 neighbor_position;

	if ( position.x == 0 )
		neighbor_position = chunk_position - glm::ivec3( 1, 0, 0 );

	if ( position.x == CHUNK_SIDE - 1 )
		neighbor_position = chunk_position + glm::ivec3( 1, 0, 0 );

	if ( position.z == 0 )
		neighbor_position = chunk_position - glm::ivec3( 0, 0, 1 );

	if ( position.z == CHUNK_SIDE - 1 )
		neighbor_position = chunk_position + glm::ivec3( 0, 0, 1 );

	// TODO: WORLD HEIGHT > 1

	//	if ( position.y == 0 )
	//		neighbor_position = chunk_position - glm::ivec3( 0, 1, 0 );

	//	if ( position.y == CHUNK_SIDE - 1 )
	//		neighbor_position = chunk_position + glm::ivec3( 0, 1, 0 );

	auto it = chunks.find( neighbor_position );

	if ( it != chunks.end() )
	{
		it->second.modified          = true;
		it->second.last_access_frame = frame_count;
		mesh_generator->push_chunk( it->second );
	}
}

void
ChunkManager::set_voxel( const glm::ivec3& position, Voxel::Type voxel )
{
	glm::ivec3 chunk_position;
	to_chunk_position( chunk_position, position );
	auto       it = chunks.find( chunk_position );
	glm::ivec3 local;
	global_voxel_to_local( local, position );

	if ( it != chunks.cend() )
	{
		it->second.last_access_frame = frame_count;
		it->second.set_voxel( local, voxel );
		mesh_generator->push_chunk( it->second );
		if ( is_transparent( voxel ) )
		{
			ensure_neighbors( local, chunk_position );
		}
	}
	else if ( voxel != Voxel::AIR )
	{
		Chunk& chunk        = chunks[ chunk_position ];
		chunk.chunk_manager = this;
		chunk.data.fill( Voxel::AIR );
		chunk.set_voxel( local, voxel );
		chunk.last_access_frame = frame_count;
		mesh_generator->push_chunk( chunk );
	}
}
