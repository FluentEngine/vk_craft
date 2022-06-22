#pragma once

#include <cstdint>

struct Mesh
{
	uint32_t index_count;
	uint32_t first_index;
	int32_t  vertex_offset;

	explicit Mesh() noexcept = default;

	explicit Mesh( uint32_t index_count,
	               uint32_t first_index,
	               int32_t  vertex_offset ) noexcept
	    : index_count( index_count )
	    , first_index( first_index )
	    , vertex_offset( vertex_offset )
	{
	}

	Mesh( Mesh const& ) = delete;
	Mesh&
	operator=( Mesh const& ) = delete;

	Mesh( Mesh&& other ) noexcept
	{
		index_count   = other.index_count;
		first_index   = other.first_index;
		vertex_offset = other.vertex_offset;

		other.first_index   = 0;
		other.index_count   = 0;
		other.vertex_offset = 0;
	}
};
