#include "quad.hpp"
#include "mesh_generator.hpp"

void
MeshGenerator::ft_create_buffers()
{
	struct ft_buffer_info info = {};
	info.memory_usage          = FT_MEMORY_USAGE_CPU_TO_GPU;
	info.descriptor_type       = FT_DESCRIPTOR_TYPE_VERTEX_BUFFER;
	info.size                  = VERTEX_BUFFER_SIZE;
	ft_create_buffer( device, &info, &vertex_buffer.buffer );
	info.descriptor_type = FT_DESCRIPTOR_TYPE_INDEX_BUFFER;
	info.size            = INDEX_BUFFER_SIZE;
	ft_create_buffer( device, &info, &index_buffer.buffer );
	vertex_buffer.offset = 0;
	index_buffer.offset  = 0;

	ft_map_memory( device, vertex_buffer.buffer );
	ft_map_memory( device, index_buffer.buffer );
}

void
MeshGenerator::ft_destroy_buffers()
{
	ft_unmap_memory( device, vertex_buffer.buffer );
	ft_unmap_memory( device, index_buffer.buffer );
	ft_destroy_buffer( device, vertex_buffer.buffer );
	ft_destroy_buffer( device, index_buffer.buffer );
}

void
MeshGenerator::init( const struct ft_device* device )
{
	this->device = device;
	ft_create_buffers();
}

void
MeshGenerator::shutdown()
{
	ft_destroy_buffers();
}

static inline void
push_face( Vertices&         vertices,
           Voxel::Type       voxel,
           Face::Type        face,
           const glm::ivec3& chunk_position,
           const glm::ivec3& block_position )
{
	glm::vec3* face_data = nullptr;

	switch ( face )
	{
	case Face::FRONT:
	{
		face_data = front_face;
		break;
	}
	case Face::BACK:
	{
		face_data = back_face;
		break;
	}
	case Face::LEFT:
	{
		face_data = left_face;
		break;
	}
	case Face::RIGHT:
	{
		face_data = right_face;
		break;
	}
	case Face::BOTTOM:
	{
		face_data = bottom_face;
		break;
	}
	case Face::TOP:
	{
		face_data = top_face;
		break;
	}
	default: abort();
	}

	glm::vec2 uv;
	get_uv( uv, voxel, face );
	float f = static_cast<float>( face );

	glm::vec3 offset( chunk_position * CHUNK_SIDE + block_position );

	vertices.emplace_back( face_data[ 0 ] + offset, uv, f );
	vertices.emplace_back( face_data[ 1 ] + offset,
	                       glm::vec2( uv.x, uv.y + UV_SIZE ),
	                       f );
	vertices.emplace_back( face_data[ 2 ] + offset,
	                       glm::vec2( uv.x + UV_SIZE, uv.y + UV_SIZE ),
	                       f );
	vertices.emplace_back( face_data[ 3 ] + offset,
	                       glm::vec2( uv.x + UV_SIZE, uv.y ),
	                       f );
}

static inline void
push_indices( Indices& indices, Index& m )
{
	indices.insert( indices.end(),
	                { Index( Index( 0 ) + m ),
	                  Index( Index( 1 ) + m ),
	                  Index( Index( 2 ) + m ),
	                  Index( Index( 2 ) + m ),
	                  Index( Index( 3 ) + m ),
	                  Index( Index( 0 ) + m ) } );
	m += 4;
}

MeshGenerator::MeshData&
MeshGenerator::generate_mesh_data( const Chunk& chunk )
{
	if ( mesh_data_map.find( chunk.position ) != mesh_data_map.end() )
	{
		if ( chunk.modified == false )
		{
			return mesh_data_map[ chunk.position ];
		}
	}

	chunk.modified = false;

	MeshData& data = mesh_data_map[ chunk.position ];
	data.vertices.clear();
	data.indices.clear();

	Index m = 0;

	data.vertices.reserve( MAX_CHUNK_VERTEX_COUNT );
	data.indices.reserve( MAX_CHUNK_INDEX_COUNT );

	for ( int32_t z = 0; z < CHUNK_SIDE; z++ )
	{
		for ( int32_t x = 0; x < CHUNK_SIDE; x++ )
		{
			for ( int32_t y = 0; y < CHUNK_SIDE; y++ )
			{
				glm::ivec3 block_position( x, y, z );

				if ( chunk.get_voxel( block_position ) != Voxel::AIR )
				{
					auto neighbors = chunk.get_neighbors( block_position );

					for ( uint32_t i = 0; i < uint32_t( Face::COUNT ); i++ )
					{
						if ( is_transparent( neighbors[ i ] ) )
						{
							push_face( data.vertices,
							           chunk.get_voxel( block_position ),
							           Face::Type( i ),
							           chunk.position,
							           block_position );
							push_indices( data.indices, m );
						}
					}
				}
			}
		}
	}

	return data;
}

void
MeshGenerator::upload_mesh( const MeshData& data )
{
	const auto& vertices = data.vertices;
	const auto& indices  = data.indices;

	reset_if_need( vertices, indices );

	void* dst =
	    ( uint8_t* ) vertex_buffer.buffer->mapped_memory + vertex_buffer.offset;
	uint64_t v_size = vertices.size() * sizeof( Vertex );
	memcpy( dst, vertices.data(), v_size );
	dst = ( uint8_t* ) index_buffer.buffer->mapped_memory + index_buffer.offset;
	uint64_t i_size = indices.size() * sizeof( Index );
	memcpy( dst, indices.data(), i_size );

	Mesh mesh;

	mesh.vertex_offset = vertex_buffer.offset / sizeof( Vertex );
	mesh.first_index   = index_buffer.offset / sizeof( Index );
	mesh.index_count   = indices.size();

	meshes.push_back( std::move( mesh ) );

	vertex_buffer.offset += v_size;
	index_buffer.offset += i_size;
}

void
MeshGenerator::push_chunk( const Chunk& chunk )
{
	upload_mesh( generate_mesh_data( chunk ) );
}

void
MeshGenerator::pop_chunk()
{
	meshes.pop_front();
}

void
MeshGenerator::reset_if_need( const Vertices& vertices, const Indices& indices )
{
	if ( ( vertex_buffer.offset + vertices.size() * sizeof( Vertex ) >
	       VERTEX_BUFFER_SIZE ) ||
	     ( index_buffer.offset + indices.size() * sizeof( Index ) >
	       INDEX_BUFFER_SIZE ) )
	{
		FT_INFO( "Reset mesh generator buffer" );
		vertex_buffer.offset = 0;
		index_buffer.offset  = 0;
	}
}

void
MeshGenerator::bind_buffers( struct ft_command_buffer* cmd ) const
{
	ft_cmd_bind_vertex_buffer( cmd, vertex_buffer.buffer, 0 );
	ft_cmd_bind_index_buffer( cmd, index_buffer.buffer, 0, FT_INDEX_TYPE_U32 );
}
