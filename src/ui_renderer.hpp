#pragma once

#include <list>
#include <functional>
#include <string>
#include <fluent/renderer.h>
#include "voxel.hpp"

class UiRenderer
{
	struct UiElement
	{
		glm::vec4                 scale_offset;
		struct ft_buffer*         buffer;
		struct ft_image*          texture;
		struct ft_descriptor_set* set;

		enum Type : uint32_t
		{
			TOOLBAR           = 0,
			TOOLBAR_HELD_ITEM = 1,
			TOOLBAR_ITEM_0    = 2,
			TOOLBAR_ITEM_1    = 3,
			TOOLBAR_ITEM_2    = 4,
			TOOLBAR_ITEM_3    = 5,
			TOOLBAR_ITEM_4    = 6,
			TOOLBAR_ITEM_5    = 7,
			TOOLBAR_ITEM_6    = 8,
			TOOLBAR_ITEM_7    = 9,
			TOOLBAR_ITEM_8    = 10,
			CROSSHAIR         = 11,
			COUNT
		};
	};

private:
	const struct ft_device* device;

	struct ft_pipeline*              pipeline;
	struct ft_descriptor_set_layout* dsl;
	struct ft_sampler*               sampler;
	struct ft_buffer*                vertex_buffer;
	struct ft_image*                 atlas;
	enum ft_format                   texture_format;

	UiElement ui_elements[ UiElement::COUNT ];

	std::list<std::function<void()>> deffered_commands;

	// elements stuff
	int32_t     toolbar_held_item_position;
	Voxel::Type toolbar_voxels[ 9 ];

	struct ft_image*
	create_image_from_file( const std::string& );

	struct ft_descriptor_set*
	create_ui_element_set( struct ft_sampler*, struct ft_image* );

	void
	calculate_scale_offsets( uint32_t width, uint32_t height );

	void
	create_toolbar();
	void
	create_toolbar_held_item();
	void
	create_toolbar_items();
	void
	create_crosshair();

public:
	void
	init( const struct ft_device*,
	      enum ft_format color_format,
	      enum ft_format depth_format );

	void
	shutdown();

	void
	render( struct ft_command_buffer* );

	void
	on_resize( uint32_t width, uint32_t height );
	void
	on_mouse_scroll_up();
	void
	on_mouse_scroll_down();

	Voxel::Type
	get_selected_voxel() const;
};
