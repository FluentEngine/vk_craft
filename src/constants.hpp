#pragma once

#include <cstdint>

// chunk
static constexpr int32_t CHUNK_SIDE   = 16;
static constexpr int32_t CHUNK_AREA   = CHUNK_SIDE * CHUNK_SIDE;
static constexpr int32_t CHUNK_VOLUME = CHUNK_AREA * CHUNK_SIDE;

// chunk manager
static constexpr int32_t RENDER_DISTANCE           = 14 * CHUNK_SIDE;
static constexpr int32_t CHUNKS_IN_RENDER_DISTANCE = 14;

// uv
static constexpr int32_t SPRITES_IN_SIDE = 16;
static constexpr float   UV_SIZE         = 1.0f / 16.0f;
