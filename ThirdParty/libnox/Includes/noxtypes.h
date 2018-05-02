#ifndef NOXTYPES_H
#define NOXTYPES_H

namespace nf {
	constexpr int WORLD_HEIGHT = 128;
	constexpr int WORLD_WIDTH = 128;
	constexpr int WORLD_TILES_COUNT = WORLD_HEIGHT * WORLD_WIDTH;

	constexpr int REGION_WIDTH = 256;
	constexpr int REGION_HEIGHT = 256;
	constexpr int REGION_DEPTH = 128;
	constexpr int REGION_TILES_COUNT = REGION_WIDTH * REGION_HEIGHT * REGION_DEPTH;

	constexpr int CHUNK_SIZE = 64;
	constexpr int CHUNK_WIDTH = REGION_WIDTH / CHUNK_SIZE;
	constexpr int CHUNK_HEIGHT = REGION_HEIGHT / CHUNK_SIZE;
	constexpr int CHUNK_DEPTH = REGION_DEPTH / CHUNK_SIZE;
	constexpr int CHUNK_TOTAL = CHUNK_HEIGHT * CHUNK_WIDTH;
	constexpr int CHUNKS_TOTAL = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

	inline int chunk_idx(const int chunk_x, const int &chunk_y, const int &chunk_z) noexcept {
		return (chunk_z * CHUNK_HEIGHT * CHUNK_WIDTH) + (chunk_y * CHUNK_WIDTH) + chunk_x;
	}

	struct cube_t {
		int x, y, z, w, h, d;
		unsigned int tex;
	};

	struct floor_t {
		int x, y, z, w, h;
		unsigned int tex;
	};

	struct static_model_t {
		int idx, x, y, z;
	};

	struct dynamic_model_t {
		int idx;
		int entity_id;
		float x, y, z;
		float axis1, axis2, axis3, rot_angle;
		float tint_r, tint_g, tint_b;
	};

	struct dynamic_lightsource_t {
		float x, y, z, r, g, b, radius;
		int entity_id;
	};

	struct hud_info_t {
		int current_power;
		int max_power;
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		int cash;
	};

	struct water_t {
		float x, y, z, depth;
	};

	struct unit_list_settler_t {
		char name[254];
		char gender[8];
		char profession[254];
		char task[254];
		float health_percent;
		char hp[254];
		int id;
	};

	struct tooltip_info_t {
		char line1[254];
		char line2[254];
		char line3[254];
		char line4[254];
		char line5[254];
	};

	struct settler_job_t {
		char name[254];
		char profession[254];
		bool is_miner;
		bool is_lumberjack;
		bool is_farmer;
		int id;
	};
}

#endif