#include <fluent/os.h>
#include <fluent/renderer.h>
#include "shader_ui_vert.hpp"
#include "shader_ui_frag.hpp"
#include "constants.hpp"
#include "voxel.hpp"
#include "ui_renderer.hpp"

struct ft_image*
UiRenderer::create_image_from_file( const std::string& filename )
{
	ft_image* image = nullptr;

	struct ft_image_info info = {};
	info.depth                = 1;
	info.sample_count         = 1;
	info.layer_count          = 1;
	info.mip_levels           = 1;
	info.format               = texture_format;
	info.descriptor_type      = FT_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

	void* data =
	    ft_read_image_from_file( filename.c_str(), &info.width, &info.height );
	size_t size = info.width * info.height * 4;

	ft_create_image( device, &info, &image );
	ft_upload_image( image, size, data );
	ft_free_image_data( data );

	return image;
}

struct ft_descriptor_set*
UiRenderer::create_ui_element_set( struct ft_sampler* sampler,
                                   struct ft_image*   image )
{
	struct ft_descriptor_set* set = nullptr;

	struct ft_descriptor_set_info set_info = {};
	set_info.set                           = 0;
	set_info.descriptor_set_layout         = dsl;
	ft_create_descriptor_set( device, &set_info, &set );

	struct ft_sampler_descriptor sampler_descriptor = {};
	sampler_descriptor.sampler                      = sampler;
	struct ft_image_descriptor image_descriptor     = {};
	image_descriptor.image                          = image;
	image_descriptor.resource_state = FT_RESOURCE_STATE_SHADER_READ_ONLY;

	struct ft_descriptor_write writes[ 2 ] = {};
	writes[ 0 ].descriptor_name            = "u_sampler";
	writes[ 0 ].descriptor_count           = 1;
	writes[ 0 ].sampler_descriptors        = &sampler_descriptor;
	writes[ 1 ].descriptor_name            = "u_atlas";
	writes[ 1 ].descriptor_count           = 1;
	writes[ 1 ].image_descriptors          = &image_descriptor;

	ft_update_descriptor_set( device, set, 2, writes );

	return set;
}

void
UiRenderer::init( const struct ft_device* device,
                  enum ft_format          color_format,
                  enum ft_format          depth_format )
{
	this->device   = device;
	texture_format = ft_is_srgb( color_format ) ? FT_FORMAT_R8G8B8A8_SRGB
	                                            : FT_FORMAT_R8G8B8A8_UNORM;

	struct ft_shader_info shader_info = {};
	shader_info.vertex                = get_ui_vert_shader( device->api );
	shader_info.fragment              = get_ui_frag_shader( device->api );

	struct ft_shader* shader;
	ft_create_shader( device, &shader_info, &shader );

	ft_create_descriptor_set_layout( device, shader, &dsl );

	struct ft_pipeline_info pipe_info       = {};
	pipe_info.type                          = FT_PIPELINE_TYPE_GRAPHICS;
	pipe_info.shader                        = shader;
	pipe_info.descriptor_set_layout         = dsl;
	pipe_info.sample_count                  = 1;
	pipe_info.color_attachment_count        = 1;
	pipe_info.color_attachment_formats[ 0 ] = color_format;
	pipe_info.depth_stencil_format          = depth_format;
	pipe_info.rasterizer_info.cull_mode     = FT_CULL_MODE_NONE;
	pipe_info.rasterizer_info.polygon_mode  = FT_POLYGON_MODE_FILL;
	pipe_info.rasterizer_info.front_face    = FT_FRONT_FACE_COUNTER_CLOCKWISE;
	pipe_info.topology = FT_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

	struct ft_vertex_layout& layout      = pipe_info.vertex_layout;
	layout.binding_info_count            = 1;
	layout.binding_infos[ 0 ].binding    = 0;
	layout.binding_infos[ 0 ].input_rate = FT_VERTEX_INPUT_RATE_VERTEX;
	layout.binding_infos[ 0 ].stride     = 4 * sizeof( float );
	layout.attribute_info_count          = 2;
	layout.attribute_infos[ 0 ].binding  = 0;
	layout.attribute_infos[ 0 ].location = 0;
	layout.attribute_infos[ 0 ].offset   = 0;
	layout.attribute_infos[ 0 ].format   = FT_FORMAT_R32G32_SFLOAT;
	layout.attribute_infos[ 1 ].binding  = 0;
	layout.attribute_infos[ 1 ].location = 1;
	layout.attribute_infos[ 1 ].offset   = 2 * sizeof( float );
	layout.attribute_infos[ 1 ].format   = FT_FORMAT_R32G32_SFLOAT;

	ft_create_pipeline( device, &pipe_info, &pipeline );

	ft_destroy_shader( device, shader );

	struct ft_sampler_info sampler_info = {};
	sampler_info.min_filter             = FT_FILTER_NEAREST;
	sampler_info.mag_filter             = FT_FILTER_NEAREST;
	sampler_info.mipmap_mode            = FT_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_info.address_mode_u         = FT_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.address_mode_v         = FT_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.address_mode_w         = FT_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	ft_create_sampler( device, &sampler_info, &sampler );

	// clang-format off
    float quad[] = {
       -0.5, -0.5, 0.0, 0.0,
        0.5, -0.5, 1.0, 0.0,
       -0.5,  0.5, 0.0, 1.0,
        0.5,  0.5, 1.0, 1.0,
    };
	// clang-format on

	struct ft_buffer_info buffer_info = {};
	buffer_info.descriptor_type       = FT_DESCRIPTOR_TYPE_VERTEX_BUFFER;
	buffer_info.memory_usage          = FT_MEMORY_USAGE_GPU_ONLY;
	buffer_info.size                  = sizeof( quad );

	ft_create_buffer( device, &buffer_info, &vertex_buffer );
	ft_upload_buffer( vertex_buffer, 0, buffer_info.size, quad );

	deffered_commands.push_back(
	    [ =, this ]()
	    {
		    ft_destroy_buffer( device, vertex_buffer );
		    ft_destroy_sampler( device, sampler );
		    ft_destroy_pipeline( device, pipeline );
		    ft_destroy_descriptor_set_layout( device, dsl );
	    } );

	create_toolbar();
	create_toolbar_held_item();
	create_toolbar_items();
	create_crosshair();
	const ft_window* window = ft_get_app_window();
	calculate_scale_offsets( ft_window_get_framebuffer_width( window ),
	                         ft_window_get_framebuffer_height( window ) );
}

void
UiRenderer::shutdown()
{
	for ( auto& cmd : deffered_commands ) { cmd(); }
}

void
UiRenderer::render( ft_command_buffer* cmd )
{
	ft_cmd_bind_pipeline( cmd, pipeline );

	for ( const auto& element : ui_elements )
	{
		ft_cmd_bind_descriptor_set( cmd, 0, element.set, pipeline );
		ft_cmd_bind_vertex_buffer( cmd, element.buffer, 0 );
		ft_cmd_push_constants( cmd,
		                       pipeline,
		                       0,
		                       sizeof( glm::vec4 ),
		                       &element.scale_offset );
		ft_cmd_draw( cmd, 4, 1, 0, 0 );
	}
}

void
UiRenderer::on_resize( uint32_t width, uint32_t height )
{
	calculate_scale_offsets( width, height );
}

void
UiRenderer::on_mouse_scroll_up()
{
	toolbar_held_item_position = std::min( 4, ++toolbar_held_item_position );

	auto& toolbar_held_item = ui_elements[ UiElement::TOOLBAR_HELD_ITEM ];

	toolbar_held_item.scale_offset.b =
	    toolbar_held_item_position * toolbar_held_item.scale_offset.r;
}

void
UiRenderer::on_mouse_scroll_down()
{
	toolbar_held_item_position = std::max( -4, --toolbar_held_item_position );

	auto& toolbar_held_item = ui_elements[ UiElement::TOOLBAR_HELD_ITEM ];

	toolbar_held_item.scale_offset.b =
	    toolbar_held_item_position * toolbar_held_item.scale_offset.r;
}

void
UiRenderer::calculate_scale_offsets( uint32_t width, uint32_t height )
{
	glm::vec2 scale( 1.0 );
	glm::vec2 offset( 0.0 );

	float aspect = static_cast<float>( width ) / static_cast<float>( height );

	auto& toolbar = ui_elements[ UiElement::TOOLBAR ];

	glm::vec2 toolbar_scale( 0.6, 0.06 * aspect );
	glm::vec2 toolbar_offset( 0.0, -0.9 );
	toolbar.scale_offset = glm::vec4( toolbar_scale, toolbar_offset );

	auto& toolbar_held_item = ui_elements[ UiElement::TOOLBAR_HELD_ITEM ];

	glm::vec2 toolbar_held_item_scale = toolbar_scale;
	toolbar_held_item_scale.x /= 9.0f;

	glm::vec2 toolbar_held_item_offset = toolbar_offset;
	toolbar_held_item_offset.x =
	    toolbar_held_item_position * toolbar_held_item_scale.x;

	toolbar_held_item.scale_offset =
	    glm::vec4( toolbar_held_item_scale, toolbar_held_item_offset );

	int32_t   start_pos          = -4.0f;
	glm::vec2 toolbar_item_scale = toolbar_held_item_scale;
	toolbar_item_scale *= 0.5;
	glm::vec2 toolbar_item_offset = toolbar_offset;

	for ( size_t item = UiElement::TOOLBAR_ITEM_0;
	      item <= UiElement::TOOLBAR_ITEM_8;
	      item++ )
	{
		toolbar_item_offset.x = start_pos * toolbar_held_item_scale.x;
		auto& toolbar_item    = ui_elements[ item ];
		toolbar_item.scale_offset =
		    glm::vec4( toolbar_item_scale, toolbar_item_offset );
		start_pos++;
	}

	glm::vec2 crosshair_scale( 0.02, 0.02 * aspect );
	glm::vec2 crosshair_offset( 0.0 );
	auto&     crosshair    = ui_elements[ UiElement::CROSSHAIR ];
	crosshair.scale_offset = glm::vec4( crosshair_scale, crosshair_offset );
}

void
UiRenderer::create_toolbar()
{
	auto& toolbar = ui_elements[ UiElement::TOOLBAR ];

	toolbar.buffer  = vertex_buffer;
	toolbar.texture = create_image_from_file( "toolbar.png" );
	toolbar.set     = create_ui_element_set( sampler, toolbar.texture );

	deffered_commands.push_back(
	    [ &, this ]()
	    {
		    ft_destroy_image( device, toolbar.texture );
		    ft_destroy_descriptor_set( device, toolbar.set );
	    } );
}

void
UiRenderer::create_toolbar_held_item()
{
	auto& toolbar_held_item = ui_elements[ UiElement::TOOLBAR_HELD_ITEM ];

	toolbar_held_item.buffer = vertex_buffer;
	toolbar_held_item.texture =
	    create_image_from_file( "toolbar_held_item.png" );
	toolbar_held_item.set =
	    create_ui_element_set( sampler, toolbar_held_item.texture );

	toolbar_held_item_position = -4;

	deffered_commands.push_back(
	    [ &, this ]()
	    {
		    ft_destroy_image( device, toolbar_held_item.texture );
		    ft_destroy_descriptor_set( device, toolbar_held_item.set );
	    } );
}

void
UiRenderer::create_toolbar_items()
{
	uint32_t voxel = 1;

	for ( uint32_t i = 0; i < 9; i++ )
	{
		toolbar_voxels[ i ] = Voxel::Type( voxel );
		voxel++;
		voxel = std::clamp( voxel,
		                    static_cast<uint32_t>( 1 ),
		                    uint32_t( Voxel::COUNT - 1 ) );
	}

	atlas = create_image_from_file( "atlas.png" );

	struct ft_descriptor_set* set = create_ui_element_set( sampler, atlas );

	size_t v = 0;
	for ( size_t item = UiElement::TOOLBAR_ITEM_0;
	      item <= UiElement::TOOLBAR_ITEM_8;
	      item++ )
	{
		auto& toolbar_item = ui_elements[ item ];

		glm::vec2 uv;
		get_uv( uv, toolbar_voxels[ v++ ], Face::FRONT );

		static constexpr float UV_OFFSET = 0.0001f;

		// clang-format off
        float quad[] = {
           -0.5, -0.5, uv.x + UV_OFFSET, uv.y + UV_SIZE - UV_OFFSET,
            0.5, -0.5, uv.x + UV_SIZE - UV_OFFSET, uv.y + UV_SIZE - UV_OFFSET,
           -0.5,  0.5, uv.x + UV_OFFSET, uv.y + UV_OFFSET,
            0.5,  0.5, uv.x + UV_SIZE - UV_OFFSET, uv.y + UV_OFFSET
        };
		// clang-format on

		struct ft_buffer_info buffer_info = {};
		buffer_info.descriptor_type       = FT_DESCRIPTOR_TYPE_VERTEX_BUFFER;
		buffer_info.memory_usage          = FT_MEMORY_USAGE_GPU_ONLY;
		buffer_info.size                  = sizeof( quad );

		ft_create_buffer( device, &buffer_info, &toolbar_item.buffer );
		ft_upload_buffer( toolbar_item.buffer, 0, buffer_info.size, quad );
		toolbar_item.texture = atlas;
		toolbar_item.set     = set;
	}

	deffered_commands.push_back(
	    [ &, set, this ]()
	    {
		    for ( size_t item = UiElement::TOOLBAR_ITEM_0;
		          item <= UiElement::TOOLBAR_ITEM_8;
		          item++ )
		    {
			    auto& toolbar_item = ui_elements[ item ];
			    ft_destroy_buffer( device, toolbar_item.buffer );
		    }
		    ft_destroy_descriptor_set( device, set );
		    ft_destroy_image( device, atlas );
	    } );
}

void
UiRenderer::create_crosshair()
{
	auto& crosshair = ui_elements[ UiElement::CROSSHAIR ];

	crosshair.buffer  = vertex_buffer;
	crosshair.texture = create_image_from_file( "crosshair.png" );
	crosshair.set     = create_ui_element_set( sampler, crosshair.texture );

	deffered_commands.push_back(
	    [ &, this ]()
	    {
		    ft_destroy_image( device, crosshair.texture );
		    ft_destroy_descriptor_set( device, crosshair.set );
	    } );
}

Voxel::Type
UiRenderer::get_selected_voxel() const
{
	size_t idx = toolbar_held_item_position + 4;
	return toolbar_voxels[ idx ];
}
