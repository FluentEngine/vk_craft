#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <cstdint>
#include <unordered_map>
#include <list>
#include <vector>
#include <fluent/renderer.h>
#include <glm/gtx/hash.hpp>
#include "constants.hpp"
#include "chunk.hpp"
#include "vertex.hpp"
#include "mesh.hpp"

using Index    = uint32_t;
using Vertices = std::vector<Vertex>;
using Indices  = std::vector<Index>;
using Meshes   = std::list<Mesh>;

class MeshGenerator
{
private:
	static constexpr uint64_t MAX_CHUNK_VERTEX_COUNT = ( CHUNK_VOLUME * 24 );
	static constexpr uint64_t MAX_CHUNK_INDEX_COUNT  = ( CHUNK_VOLUME * 36 );
	static constexpr uint64_t MAX_CHUNK_COUNT =
	    ( 4 * ( CHUNKS_IN_RENDER_DISTANCE * CHUNKS_IN_RENDER_DISTANCE ) );

	static constexpr uint64_t VERTEX_BUFFER_SIZE =
	    ( MAX_CHUNK_COUNT * MAX_CHUNK_VERTEX_COUNT * sizeof( Vertex ) ) / 8;
	static constexpr uint64_t INDEX_BUFFER_SIZE =
	    ( MAX_CHUNK_COUNT * MAX_CHUNK_INDEX_COUNT * sizeof( Index ) ) / 8;

	struct MeshBuffer
	{
		struct ft_buffer* buffer;
		uint64_t          offset;
	};

	struct MeshData
	{
		Vertices vertices;
		Indices  indices;
		size_t   last_access_frame;
	};

	const struct ft_device* device = nullptr;

	MeshBuffer vertex_buffer;
	MeshBuffer index_buffer;

	std::unordered_map<glm::vec3, MeshData> mesh_data_map;
	Meshes                                  meshes;

	size_t frame_count;

	void
	create_buffers();
	void
	destroy_buffers();

	MeshData&
	generate_mesh_data( const Chunk& );

	void
	upload_mesh( const MeshData& );

	void
	reset_if_need( const Vertices&, const Indices& );

public:
	void
	init( const struct ft_device* );

	void
	shutdown();

	void
	push_chunk( const Chunk& );

	void
	pop_chunk();

	void
	reset()
	{
		if ( ( frame_count % 10 ) == 0 )
		{
			bool erase_something = false;
			for ( auto it = mesh_data_map.begin(); it != mesh_data_map.end(); )
			{
				if ( frame_count - it->second.last_access_frame > 10 )
				{
					it              = mesh_data_map.erase( it );
					erase_something = true;
				}
				else
				{
					it++;
				}
			}

			if ( erase_something )
			{
				mesh_data_map.rehash( 0 );
			}
		}
		vertex_buffer.offset = 0;
		index_buffer.offset  = 0;
		meshes.clear();
		frame_count++;
	}

	void
	bind_buffers( struct ft_command_buffer* ) const;

	const Meshes&
	get_meshes() const
	{
		return meshes;
	}
};
