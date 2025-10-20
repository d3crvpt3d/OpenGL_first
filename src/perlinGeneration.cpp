#include <array>
#include <cstdint>
#include <cmath>
#include <algorithm>

#include "PerlinNoise.hpp"
#include "chunkMap.h"

// Smoothstep für Interpolation
inline float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Trilineare Interpolation
inline float trilinear_interpolate(
    float c000, float c001, float c010, float c011,
    float c100, float c101, float c110, float c111,
    float tx, float ty, float tz)
{
    float c00 = c000 * (1 - tx) + c100 * tx;
    float c01 = c001 * (1 - tx) + c101 * tx;
    float c10 = c010 * (1 - tx) + c110 * tx;
    float c11 = c011 * (1 - tx) + c111 * tx;
    
    float c0 = c00 * (1 - ty) + c10 * ty;
    float c1 = c01 * (1 - ty) + c11 * ty;
    
    return c0 * (1 - tz) + c1 * tz;
}

// Block ID aus Höhe berechnen
inline int height_to_block_id(float terrain_height, float y) {
    float depth = terrain_height - y;
    
    float air_factor = 1.0f - smoothstep(-0.5f, 0.5f, depth);
    float grass_factor = smoothstep(-0.5f, 0.5f, depth) * (1.0f - smoothstep(0.5f, 1.5f, depth));
    float dirt_factor = smoothstep(0.5f, 1.5f, depth) * (1.0f - smoothstep(3.5f, 4.5f, depth));
    float stone_factor = smoothstep(3.5f, 4.5f, depth);
    
    float block_id = 0.0f * air_factor + 
                     1.0f * grass_factor + 
                     2.0f * dirt_factor + 
                     3.0f * stone_factor;
    
    return static_cast<int>(std::round(block_id));
}

void generate_chunk_with_caves(
    ChunkMap &map,
    int chunk_x,
    int chunk_y,
    int chunk_z,
    int octaves = 4,
    int sample_rate = 4)
{

	Chunk_t *chunk = map.at(chunk_x, chunk_y, chunk_z);

    constexpr int sample_size = (CHUNK_WDH / 4) + 1;
    
    const siv::PerlinNoise::seed_type seed = 0xABC152;
    const siv::PerlinNoise perlin{seed};

    // Höhen-Samples (2D) - [x][z] für XZ-Ebene
    float height_samples[sample_size][sample_size];
    
    // Cave-Samples (3D) - [x][y][z]
    float cave_samples[sample_size][sample_size][sample_size];
    
    // sampling
    for (int sx = 0; sx <= CHUNK_WDH; sx += sample_rate) {
        for (int sz = 0; sz <= CHUNK_WDH; sz += sample_rate) {

            // world coords für sample point
            float world_x = (chunk_x * CHUNK_WDH + sx) * 0.01f;
            float world_z = (chunk_z * CHUNK_WDH + sz) * 0.01f;
            
			//sample xz plane
            float height_01 = perlin.octave3D_01(world_x, 0.0f, world_z, octaves);

			//0 = meeresspiegel start
			const float mountain_peak = 96.0f; // Die maximale Höhe, die Berge erreichen können.
			const float valley_exponent = 2.5f; // Ein Wert > 1.0 drückt die Täler nach unten. Höher = flachere Täler.

			//potents function for smoothing vallies
			float remapped_height = powf(height_01, valley_exponent);
			height_samples[sx / sample_rate][sz / sample_rate] = remapped_height * mountain_peak;
			//0 = meeresspiegel end
            
            // 3D caves
            for (int sy = 0; sy <= CHUNK_WDH; sy += sample_rate) {
                float world_y = (chunk_y * CHUNK_WDH + sy) * 0.01f;
                
                float cave_noise = perlin.octave3D_01(
                    world_x * 3.0f, 
                    world_y * 3.0f, 
                    world_z * 3.0f, 
                    3
                );
                
                cave_samples[sx / sample_rate][sy / sample_rate][sz / sample_rate] = cave_noise;
            }
        }
    }
    
    // 2. Phase: Interpolation und Block-Platzierung
    for (int z = 0; z < CHUNK_WDH; z++) {
        for (int y = 0; y < CHUNK_WDH; y++) {
            for (int x = 0; x < CHUNK_WDH; x++) {

				//bilinear interpolation height
                int sx0 = (x / sample_rate) * sample_rate;
                int sz0 = (z / sample_rate) * sample_rate;
                int sx1 = std::min(sx0 + sample_rate, CHUNK_WDH);
                int sz1 = std::min(sz0 + sample_rate, CHUNK_WDH);
                
                float tx = float(x - sx0) / sample_rate;
                float tz = float(z - sz0) / sample_rate;
                
                // Hole 4 Eck-Höhenwerte aus height_samples[x][z]
                float h00 = height_samples[sx0 / sample_rate][sz0 / sample_rate];
                float h01 = height_samples[sx0 / sample_rate][sz1 / sample_rate];
                float h10 = height_samples[sx1 / sample_rate][sz0 / sample_rate];
                float h11 = height_samples[sx1 / sample_rate][sz1 / sample_rate];
                
                // Bilineare Interpolation
                float h0 = h00 * (1 - tx) + h10 * tx;
                float h1 = h01 * (1 - tx) + h11 * tx;
                float terrain_height = h0 * (1 - tz) + h1 * tz;
                
				//trilinear interpolation für caves
                int sy0 = (y / sample_rate) * sample_rate;
                int sy1 = std::min(sy0 + sample_rate, CHUNK_WDH);
                float ty = float(y - sy0) / sample_rate;
                
                // 8 corner points for cave_samples
                float c000 = cave_samples[sx0/sample_rate][sy0/sample_rate][sz0/sample_rate];
                float c001 = cave_samples[sx0/sample_rate][sy0/sample_rate][sz1/sample_rate];
                float c010 = cave_samples[sx0/sample_rate][sy1/sample_rate][sz0/sample_rate];
                float c011 = cave_samples[sx0/sample_rate][sy1/sample_rate][sz1/sample_rate];
                float c100 = cave_samples[sx1/sample_rate][sy0/sample_rate][sz0/sample_rate];
                float c101 = cave_samples[sx1/sample_rate][sy0/sample_rate][sz1/sample_rate];
                float c110 = cave_samples[sx1/sample_rate][sy1/sample_rate][sz0/sample_rate];
                float c111 = cave_samples[sx1/sample_rate][sy1/sample_rate][sz1/sample_rate];
                
                float cave_noise = trilinear_interpolate(
                    c000, c001, c010, c011,
                    c100, c101, c110, c111,
                    tx, ty, tz
                );
                
				//calc block id
				float world_y = chunk_y * CHUNK_WDH + y;
				int block_id = height_to_block_id(terrain_height, world_y);

				//prot survace new start

				// 1. Berechne die Tiefe des Blocks relativ zur Geländeoberfläche.
				float depth_from_surface = terrain_height - world_y;

				// 2. Definiere die Schutzzonen.
				const float surface_protection_depth = -10.0f; // Keine Höhlen in den obersten 10 Blöcken.
				const float full_cave_depth = 10.0f;          // Ab 25 Blöcken Tiefe beginnt die volle Höhlengeneration.

				// 3. Berechne einen Maskierungsfaktor (0.0 an der Oberfläche, 1.0 in der Tiefe).
				float cave_mask_factor = smoothstep(surface_protection_depth, full_cave_depth, depth_from_surface);

				// 4. Berechne den dynamischen Schwellenwert.
				// An der Oberfläche (factor=0) ist der Wert 1.1 (unmöglich zu erreichen).
				// In der Tiefe (factor=1) ist der Wert base_cave_threshold.
				float base_cave_threshold = 0.6f;
				float dynamic_cave_threshold = 1.1f - (1.1f - base_cave_threshold) * cave_mask_factor;

                // prot survace end

				// 5. Wende den neuen Schwellenwert an, um Höhlen auszuschneiden.
				if (cave_noise > dynamic_cave_threshold && block_id > 0) {
					block_id = 0;  // Block zu Luft machen.
				}
                
                chunk->blocks[z][y][x] = static_cast<uint16_t>(block_id);
            }
        }
    }
}
