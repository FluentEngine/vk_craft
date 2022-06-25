#include <fluent/os.h>
#include <fluent/renderer.h>
#include "vertex.hpp"
#include "mesh_renderer.hpp"
#include "shader_main_vert.hpp"
#include "shader_main_frag.hpp"

void
MeshRenderer::create_ubo_buffer()
{
	struct ft_buffer_info info = {};
	info.descriptor_type       = FT_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	info.memory_usage          = FT_MEMORY_USAGE_CPU_TO_GPU;
	info.size                  = 2 * sizeof( float4x4 );

	ft_create_buffer( device, &info, &ubo_buffer );
}

void
MeshRenderer::create_atlas( enum ft_format color_format )
{
	struct ft_sampler_info sampler_info = {};
	sampler_info.min_filter             = FT_FILTER_NEAREST;
	sampler_info.mag_filter             = FT_FILTER_NEAREST;
	sampler_info.address_mode_u         = FT_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.address_mode_v         = FT_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.address_mode_w         = FT_SAMPLER_ADDRESS_MODE_REPEAT;

	ft_create_sampler( device, &sampler_info, &sampler );

	struct ft_image_info image_info = {};
	image_info.depth                = 1;
	image_info.sample_count         = 1;
	image_info.layer_count          = 1;
	image_info.mip_levels           = 1;
	image_info.format = ft_is_srgb( color_format ) ? FT_FORMAT_R8G8B8A8_SRGB
	                                               : FT_FORMAT_R8G8B8A8_UNORM;
	image_info.descriptor_type = FT_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

	void* image_data = ft_read_image_from_file( "atlas.png",
	                                            &image_info.width,
	                                            &image_info.height );

	size_t image_data_size = image_info.width * image_info.height * 4;

	ft_create_image( device, &image_info, &atlas );
	ft_upload_image( atlas, image_data_size, image_data );

	ft_free_image_data( image_data );
}

void
MeshRenderer::create_mesh_pipeline( enum ft_format color_format,
                                    enum ft_format depth_format )
{
	ft_shader_info shader_info = {};
	shader_info.vertex         = get_main_vert_shader( device->api );
	shader_info.fragment       = get_main_frag_shader( device->api );

	ft_shader* shader;
	ft_create_shader( device, &shader_info, &shader );

	ft_create_descriptor_set_layout( device, shader, &dsl );

	ft_descriptor_set_info set_info {};
	set_info.set                   = 0;
	set_info.descriptor_set_layout = dsl;
	ft_create_descriptor_set( device, &set_info, &set );

	ft_buffer_descriptor buffer_descriptor {};
	buffer_descriptor.buffer = ubo_buffer;
	buffer_descriptor.offset = 0;
	buffer_descriptor.range  = ubo_buffer->size;

	ft_sampler_descriptor sampler_descriptor {};
	sampler_descriptor.sampler = sampler;

	ft_image_descriptor image_descriptor {};
	image_descriptor.image          = atlas;
	image_descriptor.resource_state = FT_RESOURCE_STATE_SHADER_READ_ONLY;

	ft_descriptor_write descriptor_writes[ 3 ] {};
	descriptor_writes[ 0 ].descriptor_name     = "global_ubo";
	descriptor_writes[ 0 ].descriptor_count    = 1;
	descriptor_writes[ 0 ].buffer_descriptors  = &buffer_descriptor;
	descriptor_writes[ 1 ].descriptor_name     = "u_sampler";
	descriptor_writes[ 1 ].descriptor_count    = 1;
	descriptor_writes[ 1 ].sampler_descriptors = &sampler_descriptor;
	descriptor_writes[ 2 ].descriptor_name     = "u_atlas";
	descriptor_writes[ 2 ].descriptor_count    = 1;
	descriptor_writes[ 2 ].image_descriptors   = &image_descriptor;

	ft_update_descriptor_set( device, set, 3, descriptor_writes );

	ft_pipeline_info pipe_info           = {};
	pipe_info.type                       = FT_PIPELINE_TYPE_GRAPHICS;
	pipe_info.vertex_layout              = Vertex::get_vertex_layout();
	pipe_info.shader                     = shader;
	pipe_info.descriptor_set_layout      = dsl;
	pipe_info.topology                   = FT_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipe_info.rasterizer_info.cull_mode    = FT_CULL_MODE_BACK;
	pipe_info.rasterizer_info.front_face   = FT_FRONT_FACE_COUNTER_CLOCKWISE;
	pipe_info.rasterizer_info.polygon_mode = FT_POLYGON_MODE_FILL;
	pipe_info.depth_state_info.depth_test   = true;
	pipe_info.depth_state_info.depth_write  = true;
	pipe_info.depth_state_info.compare_op   = FT_COMPARE_OP_LESS;
	pipe_info.sample_count                  = 1;
	pipe_info.color_attachment_count        = 1;
	pipe_info.color_attachment_formats[ 0 ] = color_format;
	pipe_info.depth_stencil_format          = depth_format;
	ft_create_pipeline( device, &pipe_info, &pipeline );

	ft_destroy_shader( device, shader );
}

void
MeshRenderer::destroy_mesh_pipeline()
{
	ft_destroy_pipeline( device, pipeline );
	ft_destroy_descriptor_set( device, set );
	ft_destroy_descriptor_set_layout( device, dsl );
}

void
MeshRenderer::init( const struct ft_device* device,
                    enum ft_format          color_format,
                    enum ft_format          depth_format )
{
	this->device = device;

	create_ubo_buffer();
	create_atlas( color_format );
	create_mesh_pipeline( color_format, depth_format );
}

void
MeshRenderer::shutdown()
{
	destroy_mesh_pipeline();
	ft_destroy_image( device, atlas );
	ft_destroy_sampler( device, sampler );
	ft_destroy_buffer( device, ubo_buffer );
}

void
MeshRenderer::update( struct ft_command_buffer* cmd,
                      const struct ft_camera*   camera,
                      uint32_t                  width,
                      uint32_t                  height )
{
	struct ShaderData
	{
		float4x4 proj;
		float4x4 view;
	} shader_data;

	float4x4_dup( shader_data.proj, camera->projection );
	float4x4_dup( shader_data.view, camera->view );

	void* dst = ft_map_memory( device, ubo_buffer );
	memcpy( dst, &shader_data, sizeof( shader_data ) );
	ft_unmap_memory( device, ubo_buffer );

	ft_cmd_bind_pipeline( cmd, pipeline );
	ft_cmd_bind_descriptor_set( cmd, 0, set, pipeline );
	ft_cmd_set_viewport( cmd, 0, 0, width, height, 0.1f, 1.0f );
	ft_cmd_set_scissor( cmd, 0, 0, width, height );
}

void
MeshRenderer::render( struct ft_command_buffer* cmd, const Meshes& meshes )
{
	for ( const auto& mesh : meshes )
	{
		ft_cmd_draw_indexed( cmd,
		                     mesh.index_count,
		                     1,
		                     mesh.first_index,
		                     mesh.vertex_offset,
		                     0 );
	}
}
