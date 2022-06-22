#pragma once

#include <fluent/renderer.h>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 tex;
	float     face;

	Vertex() = default;
	Vertex( const glm::vec3 position, const glm::vec2 texture, float face )
	    : face( face )
	    , pos( position )
	    , tex( texture )
	{
	}

	static inline struct ft_vertex_layout
	get_vertex_layout()
	{
		struct ft_vertex_layout layout       = {};
		layout.binding_info_count            = 1;
		layout.binding_infos[ 0 ].binding    = 0;
		layout.binding_infos[ 0 ].input_rate = FT_VERTEX_INPUT_RATE_VERTEX;
		layout.binding_infos[ 0 ].stride     = sizeof( Vertex );
		layout.attribute_info_count          = 3;
		layout.attribute_infos[ 0 ].binding  = 0;
		layout.attribute_infos[ 0 ].format   = FT_FORMAT_R32G32B32_SFLOAT;
		layout.attribute_infos[ 0 ].location = 0;
		layout.attribute_infos[ 0 ].offset   = 0;
		layout.attribute_infos[ 1 ].binding  = 0;
		layout.attribute_infos[ 1 ].format   = FT_FORMAT_R32G32_SFLOAT;
		layout.attribute_infos[ 1 ].location = 1;
		layout.attribute_infos[ 1 ].offset   = 3 * sizeof( float );
		layout.attribute_infos[ 2 ].binding  = 0;
		layout.attribute_infos[ 2 ].format   = FT_FORMAT_R32_SFLOAT;
		layout.attribute_infos[ 2 ].location = 2;
		layout.attribute_infos[ 2 ].offset   = 5 * sizeof( float );

		return layout;
	}
};
