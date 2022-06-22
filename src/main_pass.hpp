#pragma once

#include <fluent/renderer.h>

void
register_main_pass( const ft_device*,
                    ft_render_graph*,
                    const char*    backbuffer_source_name,
                    enum ft_format color_format,
                    const ft_camera* );
void
free_main_pass_data( void );
