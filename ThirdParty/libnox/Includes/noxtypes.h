#ifndef NOXTYPES_H
#define NOXTYPES_H

namespace nf {

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

	struct veg_t {
		int plant, lifecycle, x, y, z;
	};

	struct dynamic_model_t {
		int idx;
		int entity_id;
		float x, y, z;
		float axis1, axis2, axis3, rot_angle;
		float tint_r, tint_g, tint_b;
		float x_scale, y_scale, z_scale;
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
		char tooltip_data[2048];
	};

	struct settler_job_t {
		char name[254];
		char profession[254];
		bool is_miner;
		bool is_lumberjack;
		bool is_farmer;
		bool is_hunter;
		int id;
	};

	struct buildable_building_t {
		char tag[254];
		char displayName[254];
	};

	struct plantable_seed_t {
		int number;
		char name[254];
		char grows_into[254];
	};

	struct queued_work_t {
		int qty;
		char reaction_def[254];
		char reaction_name[254];
	};

	struct available_work_t {
		char reaction_def[254];
		char reaction_name[254];
		char building_name[254];
		char inputs[1024];
		char outputs[1024];
	};

	struct active_standing_order_t {
		char item_name[254];
		int min_qty;
		char reaction_name[254];
		char item_tag[254];
	};
}

#endif