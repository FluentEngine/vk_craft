#include <future>
#include "quad.hpp"
#include "mesh_generator.hpp"
#include "coordinates.hpp"
#include "chunk_manager.hpp"

void
MeshGenerator::create_buffers()
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
MeshGenerator::destroy_buffers()
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
	create_buffers();
}

void
MeshGenerator::shutdown()
{
	destroy_buffers();
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
			MeshData& data         = mesh_data_map[ chunk.position ];
			data.last_access_frame = frame_count;
			return data;
		}
	}

	chunk.modified = false;

	MeshData& data         = mesh_data_map[ chunk.position ];
	data.last_access_frame = frame_count;
	data.vertices.clear();
	data.indices.clear();

	Index m = 0;

	data.vertices.reserve( 4000 );
	data.indices.reserve( 4000 * 6 );

	// TODO: refactor
	// TODO: optimize

	struct VoxelFace
	{
		Voxel::Type voxel;
		Face::Type  face;
	};

	int        i, j, k, l, w, h, u, v, n;
	Face::Type face;

	int32_t x[ 3 ]  = { 0, 0, 0 };
	int32_t q[ 3 ]  = { 0, 0, 0 };
	int32_t du[ 3 ] = { 0, 0, 0 };
	int32_t dv[ 3 ] = { 0, 0, 0 };

	VoxelFace mask[ CHUNK_SIZE_SQUARED ];
	memset( mask, 0, sizeof( mask ) );

	VoxelFace voxel_face0, voxel_face1;

	for ( bool back_face = true, b = false; b != back_face;
	      back_face = back_face && b, b = !b )
	{
		for ( int d = 0; d < 3; d++ )
		{
			u = ( d + 1 ) % 3;
			v = ( d + 2 ) % 3;

			x[ 0 ] = 0;
			x[ 1 ] = 0;
			x[ 2 ] = 0;

			q[ 0 ] = 0;
			q[ 1 ] = 0;
			q[ 2 ] = 0;
			q[ d ] = 1;

			if ( d == 0 )
			{
				face = back_face ? Face::LEFT : Face::RIGHT;
			}
			else if ( d == 1 )
			{
				face = back_face ? Face::BOTTOM : Face::TOP;
			}
			else if ( d == 2 )
			{
				face = back_face ? Face::BACK : Face::FRONT;
			}

			for ( x[ d ] = -1; x[ d ] < CHUNK_SIZE; )
			{
				n = 0;

				for ( x[ v ] = 0; x[ v ] < CHUNK_SIZE; x[ v ]++ )
				{
					for ( x[ u ] = 0; x[ u ] < CHUNK_SIZE; x[ u ]++ )
					{
						voxel_face0 =
						    ( x[ d ] >= 0 )
						        ? VoxelFace { chunk.get_voxel(
						                          { x[ 0 ], x[ 1 ], x[ 2 ] } ),
						                      face }
						        : VoxelFace { Voxel::AIR };
						voxel_face1 =
						    ( x[ d ] < CHUNK_SIZE - 1 )
						        ? VoxelFace { chunk.get_voxel(
						                          { x[ 0 ] + q[ 0 ],
						                            x[ 1 ] + q[ 1 ],
						                            x[ 2 ] + q[ 2 ] } ),
						                      face }
						        : VoxelFace { Voxel::AIR };

						mask[ n++ ] =
						    ( ( voxel_face0.voxel != Voxel::AIR &&
						        voxel_face1.voxel != Voxel::AIR &&
						        voxel_face0.voxel == voxel_face1.voxel ) )
						        ? VoxelFace { Voxel::AIR }
						    : back_face ? voxel_face1
						                : voxel_face0;
					}
				}

				x[ d ]++;

				n = 0;

				for ( j = 0; j < CHUNK_SIZE; j++ )
				{
					for ( i = 0; i < CHUNK_SIZE; )
					{
						if ( mask[ n ].voxel != Voxel::AIR )
						{
							for ( w = 1; i + w < CHUNK_SIZE &&
							             mask[ n + w ].voxel != Voxel::AIR &&
							             mask[ n + w ].voxel == mask[ n ].voxel;
							      w++ )
							{
							}

							bool done = false;

							for ( h = 1; j + h < CHUNK_SIZE; h++ )
							{
								for ( k = 0; k < w; k++ )
								{
									if ( mask[ n + k + h * CHUNK_SIZE ].voxel ==
									         Voxel::AIR ||
									     !( mask[ n + k + h * CHUNK_SIZE ]
									            .voxel == mask[ n ].voxel ) )
									{
										done = true;
										break;
									}
								}

								if ( done )
								{
									break;
								}
							}

							x[ u ] = i;
							x[ v ] = j;

							du[ 0 ] = 0;
							du[ 1 ] = 0;
							du[ 2 ] = 0;
							du[ u ] = w;

							dv[ 0 ] = 0;
							dv[ 1 ] = 0;
							dv[ 2 ] = 0;
							dv[ v ] = h;

							glm::vec2 uv;
							get_uv( uv, mask[ n ].voxel, mask[ n ].face );

							Vertex v0( glm::ivec3( x[ 0 ], x[ 1 ], x[ 2 ] ) +
							               chunk.position * CHUNK_SIZE,
							           uv,
							           mask[ n ].face );

							Vertex v1( glm::ivec3( x[ 0 ] + du[ 0 ],
							                       x[ 1 ] + du[ 1 ],
							                       x[ 2 ] + du[ 2 ] ) +
							               chunk.position * CHUNK_SIZE,
							           uv,
							           mask[ n ].face );

							Vertex v2(
							    glm::ivec3( x[ 0 ] + du[ 0 ] + dv[ 0 ],
							                x[ 1 ] + du[ 1 ] + dv[ 1 ],
							                x[ 2 ] + du[ 2 ] + dv[ 2 ] ) +
							        chunk.position * CHUNK_SIZE,
							    uv,
							    mask[ n ].face );

							Vertex v3( glm::ivec3( x[ 0 ] + dv[ 0 ],
							                       x[ 1 ] + dv[ 1 ],
							                       x[ 2 ] + dv[ 2 ] ) +
							               chunk.position * CHUNK_SIZE,
							           uv,
							           mask[ n ].face );

							switch ( mask[ n ].face )
							{
							case Face::BOTTOM:
							case Face::BACK:
							case Face::LEFT:
							{
								data.vertices.insert( data.vertices.end(),
								                      { v3, v2, v1, v0 } );
								break;
							}
							case Face::TOP:
							case Face::RIGHT:
							case Face::FRONT:
							{
								data.vertices.insert( data.vertices.end(),
								                      { v0, v1, v2, v3 } );
								break;
							}
							}

							push_indices( data.indices, m );

							for ( l = 0; l < h; ++l )
							{
								for ( k = 0; k < w; ++k )
								{
									mask[ n + k + l * CHUNK_SIZE ].voxel =
									    Voxel::AIR;
								}
							}

							i += w;
							n += w;
						}
						else
						{
							i++;
							n++;
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
MeshGenerator::push_chunks( const std::list<Chunk*>& chunks )
{
	for ( const Chunk* chunk : chunks )
	{
		upload_mesh( generate_mesh_data( *chunk ) );
	}
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
