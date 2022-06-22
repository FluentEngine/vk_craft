#pragma once

#include <fluent/renderer.h>
#include "mesh_generator.hpp"

class MeshRenderer
{
private:
	const struct ft_device*          device;
	struct ft_buffer*                ubo_buffer;
	struct ft_sampler*               sampler;
	struct ft_image*                 atlas;
	struct ft_descriptor_set_layout* dsl;
	struct ft_descriptor_set*        set;
	struct ft_pipeline*              pipeline;

	void
	create_ubo_buffer();

	void
	create_atlas( enum ft_format color_format );

	void
	create_mesh_pipeline( enum ft_format color_format,
	                      enum ft_format depth_format );
	void
	destroy_mesh_pipeline();

public:
	void
	init( const struct ft_device*,
	      enum ft_format color_format,
	      enum ft_format depth_format );

	void
	shutdown();

	void
	update( struct ft_command_buffer*,
	        const struct ft_camera*,
	        uint32_t width,
	        uint32_t height );

	void
	render( struct ft_command_buffer*, const Meshes& );
};
