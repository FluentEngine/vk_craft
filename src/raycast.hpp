#pragma once

#include <optional>
#include <glm/glm.hpp>
#include "coordinates.hpp"
#include "chunk_manager.hpp"

struct RaycastResult
{
	glm::vec3 norm;
	glm::vec3 end;
};

std::optional<RaycastResult>
raycast( const ChunkManager& manager,
         const glm::vec3&    start_point,
         const glm::vec3&    direction,
         float               range )
{
	auto       n_direction = normalize( direction );
	auto       end_point   = start_point + n_direction * range;
	glm::ivec3 start_voxel;
	to_voxel_position( start_voxel, start_point );

	// +1, -1, or 0
	int step_x = ( n_direction.x > 0 ) ? 1 : ( ( n_direction.x < 0 ) ? -1 : 0 );
	int step_y = ( n_direction.y > 0 ) ? 1 : ( ( n_direction.y < 0 ) ? -1 : 0 );
	int step_z = ( n_direction.z > 0 ) ? 1 : ( ( n_direction.z < 0 ) ? -1 : 0 );

	float t_delta_x =
	    ( step_x != 0 )
	        ? fmin( step_x / ( end_point.x - start_point.x ), FLT_MAX )
	        : FLT_MAX;
	float t_delta_y =
	    ( step_y != 0 )
	        ? fmin( step_y / ( end_point.y - start_point.y ), FLT_MAX )
	        : FLT_MAX;
	float t_delta_z =
	    ( step_z != 0 )
	        ? fmin( step_z / ( end_point.z - start_point.z ), FLT_MAX )
	        : FLT_MAX;

	float t_max_x = ( step_x > 0.0f )
	                    ? t_delta_x * ( 1.0f - start_point.x + start_voxel.x )
	                    : t_delta_x * ( start_point.x - start_voxel.x );
	float t_max_y = ( step_y > 0.0f )
	                    ? t_delta_y * ( 1.0f - start_point.y + start_voxel.y )
	                    : t_delta_y * ( start_point.y - start_voxel.y );
	float t_max_z = ( step_z > 0.0f )
	                    ? t_delta_z * ( 1.0f - start_point.z + start_voxel.z )
	                    : t_delta_z * ( start_point.z - start_voxel.z );

	int t             = 0;
	int stepped_index = -1;

	while ( ++t < range * 3 )
	{
		if ( manager.get_voxel( start_voxel ) != Voxel::AIR )
		{
			RaycastResult res;
			res.end    = start_voxel;
			res.norm.x = res.norm.y = res.norm.z = 0.0f;
			if ( stepped_index == 0 )
				res.norm.x = -step_x;
			if ( stepped_index == 1 )
				res.norm.y = -step_y;
			if ( stepped_index == 2 )
				res.norm.z = -step_z;
			return res;
		}

		if ( t_max_x < t_max_y )
		{
			if ( t_max_x < t_max_z )
			{
				start_voxel.x += step_x;
				t_max_x += t_delta_x;
				stepped_index = 0;
			}
			else
			{
				start_voxel.z += step_z;
				t_max_z += t_delta_z;
				stepped_index = 2;
			}
		}
		else
		{
			if ( t_max_y < t_max_z )
			{
				start_voxel.y += step_y;
				t_max_y += t_delta_y;
				stepped_index = 1;
			}
			else
			{
				start_voxel.z += step_z;
				t_max_z += t_delta_z;
				stepped_index = 2;
			}
		}
		if ( t_max_x > 1 && t_max_y > 1 && t_max_z > 1 )
			break;
	}
	return {};
}
