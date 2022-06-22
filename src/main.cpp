#include <fluent/os.h>
#include <fluent/renderer.h>
#include "main_pass.hpp"

#define FRAME_COUNT   2
#define WINDOW_WIDTH  1400
#define WINDOW_HEIGHT 900

struct frame_data
{
	struct ft_semaphore* present_semaphore;
	struct ft_semaphore* render_semaphore;
	struct ft_fence*     render_fence;

	struct ft_command_pool*   cmd_pool;
	struct ft_command_buffer* cmd;
	bool                      cmd_recorded;
};

static enum ft_renderer_api        renderer_api   = FT_RENDERER_API_VULKAN;
static struct ft_renderer_backend* backend        = NULL;
static struct ft_device*           device         = NULL;
static struct ft_queue*            graphics_queue = NULL;
static struct ft_swapchain*        swapchain      = NULL;
static struct frame_data           frames[ FRAME_COUNT ];
static uint32_t                    frame_index = 0;
static uint32_t                    image_index = 0;

struct ft_render_graph* graph;

static struct ft_camera            camera;
static struct ft_camera_controller camera_controller;

static void
init_renderer( void );

static void
shutdown_renderer( void );

static void
begin_frame( void );

static void
end_frame( void );

static void
on_init( void )
{
	float3 position  = { 0.0f, 0.0f, 3.0f };
	float3 direction = { 0.0f, 0.0f, -1.0f };
	float3 up        = { 0.0f, 1.0f, 0.0f };
	float  aspect    = ft_window_get_aspect( ft_get_app_window() );

	struct ft_camera_info camera_info = {};
	camera_info.fov                   = radians( 45.0f );
	camera_info.aspect                = aspect;
	camera_info.near                  = 0.1f;
	camera_info.far                   = 1500.0f;
	camera_info.speed                 = 10.0f;
	camera_info.sensitivity           = 0.12f;
	float3_dup( camera_info.position, position );
	float3_dup( camera_info.direction, direction );
	float3_dup( camera_info.up, up );

	ft_camera_init( &camera, &camera_info );
	ft_camera_controller_init( &camera_controller, &camera );

	init_renderer();

	ft_rg_create( device, &graph );
	register_main_pass( device, graph, "back", swapchain->format, &camera );
	ft_rg_set_backbuffer_source( graph, "back" );
	ft_rg_set_swapchain_dimensions( graph,
	                                swapchain->width,
	                                swapchain->height );
	ft_rg_build( graph );
}

static void
on_update( float delta_time )
{
	ft_camera_controller_update( &camera_controller, delta_time );

	begin_frame();

	struct ft_command_buffer* cmd = frames[ frame_index ].cmd;
	ft_begin_command_buffer( cmd );

	ft_rg_setup_attachments( graph, swapchain->images[ image_index ] );
	ft_rg_execute( cmd, graph );

	ft_end_command_buffer( cmd );

	end_frame();
}

static void
on_resize( uint32_t width, uint32_t height )
{
	ft_queue_wait_idle( graphics_queue );
	ft_resize_swapchain( device, swapchain, width, height );
	ft_rg_set_swapchain_dimensions( graph,
	                                swapchain->width,
	                                swapchain->height );
	ft_rg_build( graph );
}

static void
on_shutdown( void )
{
	ft_queue_wait_idle( graphics_queue );
	ft_rg_destroy( graph );
	free_main_pass_data();
	shutdown_renderer();
}

int
main( int argc, char** argv )
{
	struct ft_window_info window_info = {};
	window_info.title                 = "vk-craft";
	window_info.x                     = 100;
	window_info.y                     = 100;
	window_info.width                 = WINDOW_WIDTH;
	window_info.height                = WINDOW_HEIGHT;
	window_info.resizable             = 0;
	window_info.centered              = 1;
	window_info.fullscreen            = 0;
	window_info.grab_mouse            = 0;
	window_info.renderer_api          = renderer_api;

	struct ft_application_config config = {};
	config.argc                         = argc;
	config.argv                         = argv;
	config.window_info                  = window_info;
	config.log_level                    = FT_TRACE;
	config.on_init                      = on_init;
	config.on_update                    = on_update;
	config.on_resize                    = on_resize;
	config.on_shutdown                  = on_shutdown;

	ft_app_init( &config );
	ft_app_run();
	ft_app_shutdown();

	return EXIT_SUCCESS;
}

static void
init_renderer()
{
	struct ft_renderer_backend_info backend_info = {};
	backend_info.api                             = renderer_api;
	backend_info.wsi_info                        = ft_get_wsi_info();

	ft_create_renderer_backend( &backend_info, &backend );

	struct ft_device_info device_info = {};
	device_info.backend               = backend;
	ft_create_device( backend, &device_info, &device );

	struct ft_queue_info queue_info = {};
	queue_info.queue_type           = FT_QUEUE_TYPE_GRAPHICS,
	ft_create_queue( device, &queue_info, &graphics_queue );

	for ( uint32_t i = 0; i < FRAME_COUNT; i++ )
	{
		frames[ i ].cmd_recorded = 0;
		ft_create_semaphore( device, &frames[ i ].present_semaphore );
		ft_create_semaphore( device, &frames[ i ].render_semaphore );
		ft_create_fence( device, &frames[ i ].render_fence );

		struct ft_command_pool_info pool_info = {};
		pool_info.queue                       = graphics_queue,

		ft_create_command_pool( device, &pool_info, &frames[ i ].cmd_pool );
		ft_create_command_buffers( device,
		                           frames[ i ].cmd_pool,
		                           1,
		                           &frames[ i ].cmd );
	}

	struct ft_swapchain_info swapchain_info = {};
	swapchain_info.width =
	    ft_window_get_framebuffer_width( ft_get_app_window() );
	swapchain_info.height =
	    ft_window_get_framebuffer_height( ft_get_app_window() );
	swapchain_info.format          = FT_FORMAT_R8G8B8A8_SRGB;
	swapchain_info.min_image_count = FRAME_COUNT;
	swapchain_info.vsync           = 1;
	swapchain_info.queue           = graphics_queue;
	swapchain_info.wsi_info        = ft_get_wsi_info();
	ft_create_swapchain( device, &swapchain_info, &swapchain );
}

static void
shutdown_renderer()
{
	ft_queue_wait_idle( graphics_queue );

	ft_destroy_swapchain( device, swapchain );

	for ( uint32_t i = 0; i < FRAME_COUNT; i++ )
	{
		ft_destroy_command_buffers( device,
		                            frames[ i ].cmd_pool,
		                            1,
		                            &frames[ i ].cmd );
		ft_destroy_command_pool( device, frames[ i ].cmd_pool );
		ft_destroy_fence( device, frames[ i ].render_fence );
		ft_destroy_semaphore( device, frames[ i ].render_semaphore );
		ft_destroy_semaphore( device, frames[ i ].present_semaphore );
	}

	ft_destroy_queue( graphics_queue );
	ft_destroy_device( device );
	ft_destroy_renderer_backend( backend );
}

static void
begin_frame()
{
	if ( !frames[ frame_index ].cmd_recorded )
	{
		ft_wait_for_fences( device, 1, &frames[ frame_index ].render_fence );
		ft_reset_fences( device, 1, &frames[ frame_index ].render_fence );
		frames[ frame_index ].cmd_recorded = 1;
	}

	ft_acquire_next_image( device,
	                       swapchain,
	                       frames[ frame_index ].present_semaphore,
	                       NULL,
	                       &image_index );
}

static void
end_frame()
{
	struct ft_queue_submit_info submit_info = {};
	submit_info.wait_semaphore_count        = 1,
	submit_info.wait_semaphores      = &frames[ frame_index ].present_semaphore,
	submit_info.command_buffer_count = 1,
	submit_info.command_buffers      = &frames[ frame_index ].cmd,
	submit_info.signal_semaphore_count = 1,
	submit_info.signal_semaphores = &frames[ frame_index ].render_semaphore,
	submit_info.signal_fence      = frames[ frame_index ].render_fence,
	ft_queue_submit( graphics_queue, &submit_info );

	struct ft_queue_present_info present_info = {};
	present_info.wait_semaphore_count         = 1;
	present_info.wait_semaphores = &frames[ frame_index ].render_semaphore;
	present_info.swapchain       = swapchain;
	present_info.image_index     = image_index;
	ft_queue_present( graphics_queue, &present_info );

	frames[ frame_index ].cmd_recorded = 0;
	frame_index                        = ( frame_index + 1 ) % FRAME_COUNT;
}
