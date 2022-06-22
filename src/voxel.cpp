#include <array>
#include "constants.hpp"
#include "voxel.hpp"

struct VoxelData
{
	std::array<uint8_t, 6> sprites;

	VoxelData&
	set( Face::Type face, uint8_t id )
	{
		sprites[ static_cast<uint32_t>( face ) ] = id;
		return *this;
	}

	VoxelData&
	set_all( uint8_t id )
	{
		sprites.fill( id );
		return *this;
	}
};

struct VoxelDataStorage
{
	std::array<VoxelData, Voxel::COUNT> sprite_data;
	std::array<bool, Voxel::COUNT>      transparent;
} voxel_data_storage;

void
init_voxel_data_storage()
{
	auto& s = voxel_data_storage.sprite_data;
	auto& t = voxel_data_storage.transparent;

	s[ Voxel::AIR ].set_all( 0 );
	s[ Voxel::GROUND ].set_all( 3 );
	s[ Voxel::SAND ].set_all( 4 );
	s[ Voxel::GLASS ].set_all( 5 );
	s[ Voxel::STONE ].set_all( 8 );
	s[ Voxel::WATER ].set_all( 9 );
	s[ Voxel::GRASS ].set_all( 2 ).set( Face::BOTTOM, 3 ).set( Face::TOP, 1 );
	s[ Voxel::WOOD ].set_all( 6 ).set( Face::BOTTOM, 7 ).set( Face::TOP, 7 );
	s[ Voxel::LEAVES ].set_all( 10 );

	t.fill( false );
	t[ Voxel::LEAVES ] = true;
	t[ Voxel::GLASS ]  = true;
	t[ Voxel::AIR ]    = true;
}

void
get_uv( glm::vec2& r, Voxel::Type voxel, Face::Type face )
{
	uint8_t sprite = voxel_data_storage.sprite_data[ voxel ].sprites[ face ];

	float u = float( sprite % SPRITES_IN_SIDE ) / 16.0f;
	float v = float( sprite / SPRITES_IN_SIDE ) / 16.0f;
	r[ 0 ]  = u;
	r[ 1 ]  = v;
}

bool
is_transparent( Voxel::Type voxel )
{
	return voxel_data_storage.transparent[ voxel ];
}
