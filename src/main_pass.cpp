#include <fluent/os.h>
#include "raycast.hpp"
#include "voxel.hpp"
#include "mesh_generator.hpp"
#include "mesh_renderer.hpp"
#include "ui_renderer.hpp"
#include "main_pass.hpp"

struct MainPassData
{
	enum ft_format          color_format;
	const struct ft_camera* camera;
	MeshGenerator           mesh_generator;
	MeshRenderer            mesh_renderer;
	ChunkManager            chunk_manager;
	UiRenderer              ui_renderer;
	Voxel::Type             current_voxel;
	uint32_t                viewport_width;
	uint32_t                viewport_height;
};

static MainPassData* main_pass_data;

static bool
main_pass_clear_color( uint32_t idx, ft_color_clear_value* v )
{
	switch ( idx )
	{
	case 0:
	{
		v[ 0 ][ 0 ] = 0.5f;
		v[ 0 ][ 1 ] = 0.6f;
		v[ 0 ][ 2 ] = 0.7f;
		v[ 0 ][ 3 ] = 1.0f;
		return true;
	}
	default:
	{
		return false;
	}
	}
}

static bool
main_pass_clear_depth_stencil( ft_depth_stencil_clear_value* v )
{
	v->depth   = 1.0f;
	v->stencil = 0;
	return true;
}

static void
main_pass_create( const struct ft_device* device, void* user_data )
{
	auto* data = static_cast<MainPassData*>( user_data );

	const ft_window* window = ft_get_app_window();

	uint32_t viewport_width  = ft_window_get_framebuffer_width( window );
	uint32_t viewport_height = ft_window_get_framebuffer_height( window );

	data->viewport_width  = viewport_width;
	data->viewport_height = viewport_height;
	data->current_voxel   = data->ui_renderer.get_selected_voxel();
	data->ui_renderer.on_resize( viewport_width, viewport_height );

	ft_window_show_cursor( false );
}

static void
main_pass_destroy( const struct ft_device* device, void* user_data )
{
	auto* data = static_cast<MainPassData*>( user_data );
}

static void
main_pass_execute( const struct ft_device*   device,
                   struct ft_command_buffer* cmd,
                   void*                     user_data )
{
	auto* data = static_cast<MainPassData*>( user_data );

	glm::vec3 camera_position( data->camera->position[ 0 ],
	                           data->camera->position[ 1 ],
	                           data->camera->position[ 2 ] );

	glm::vec3 camera_direction( data->camera->direction[ 0 ],
	                            data->camera->direction[ 1 ],
	                            data->camera->direction[ 2 ] );

	data->chunk_manager.update_visible_chunks( camera_position );

	data->mesh_generator.bind_buffers( cmd );
	data->mesh_renderer.update( cmd,
	                            data->camera,
	                            data->viewport_width,
	                            data->viewport_height );
	data->mesh_renderer.render( cmd, data->mesh_generator.get_meshes() );
	data->ui_renderer.render( cmd );

	if ( ft_get_mouse_wheel() > 0 )
	{
		data->ui_renderer.on_mouse_scroll_up();
		data->current_voxel = data->ui_renderer.get_selected_voxel();
	}
	else if ( ft_get_mouse_wheel() < 0 )
	{
		data->ui_renderer.on_mouse_scroll_down();
		data->current_voxel = data->ui_renderer.get_selected_voxel();
	}

	bool lmb = ft_is_button_pressed( FT_BUTTON_LEFT );
	bool rmb = ft_is_button_pressed( FT_BUTTON_RIGHT );

	if ( ft_is_key_pressed( FT_KEY_ESCAPE ) )
	{
		ft_window_show_cursor( true );
	}

	if ( lmb )
	{
		ft_window_show_cursor( false );
	}

	if ( lmb || rmb )
	{
		auto res = raycast( data->chunk_manager,
		                    camera_position,
		                    camera_direction,
		                    10.0f );

		if ( res.has_value() )
		{
			if ( lmb )
			{
				data->chunk_manager.set_voxel( res->end, Voxel::AIR );
			}

			if ( rmb )
			{
				data->chunk_manager.set_voxel( res->end + res->norm,
				                               data->current_voxel );
			}
		}
	}
}

void
register_main_pass( const ft_device* device,
                    ft_render_graph* graph,
                    const char*      backbuffer_source_name,
                    enum ft_format   color_format,
                    const ft_camera* camera )
{
	const ft_window* window = ft_get_app_window();

	uint32_t viewport_width  = ft_window_get_framebuffer_width( window );
	uint32_t viewport_height = ft_window_get_framebuffer_height( window );

	ft_image_info back = {};

	ft_image_info depth   = {};
	depth.width           = viewport_width;
	depth.height          = viewport_height;
	depth.depth           = 1;
	depth.sample_count    = 1;
	depth.layer_count     = 1;
	depth.format          = FT_FORMAT_D32_SFLOAT;
	depth.mip_levels      = 1;
	depth.descriptor_type = FT_DESCRIPTOR_TYPE_DEPTH_STENCIL_ATTACHMENT;

	struct MainPassData* data = new MainPassData;
	data->viewport_width      = viewport_width;
	data->viewport_height     = viewport_height;
	data->color_format        = color_format;
	data->camera              = camera;

	init_voxel_data_storage();
	data->mesh_generator.init( device );
	data->mesh_renderer.init( device,
	                          data->color_format,
	                          FT_FORMAT_D32_SFLOAT );
	data->chunk_manager.init( &data->mesh_generator );
	data->ui_renderer.init( device, color_format, FT_FORMAT_D32_SFLOAT );

	ft_render_pass* pass;
	ft_rg_add_pass( graph, "main", &pass );
	ft_rg_add_color_output( pass, backbuffer_source_name, &back );
	ft_rg_add_depth_stencil_output( pass, "depth", &depth );
	ft_rg_set_get_clear_color( pass, main_pass_clear_color );
	ft_rg_set_get_clear_depth_stencil( pass, main_pass_clear_depth_stencil );
	ft_rg_set_user_data( pass, data );
	ft_rg_set_pass_create_callback( pass, main_pass_create );
	ft_rg_set_pass_destroy_callback( pass, main_pass_destroy );
	ft_rg_set_pass_execute_callback( pass, main_pass_execute );

	main_pass_data = data;
}

void
free_main_pass_data()
{
	main_pass_data->ui_renderer.shutdown();
	main_pass_data->mesh_renderer.shutdown();
	main_pass_data->mesh_generator.shutdown();
	delete main_pass_data;
}
