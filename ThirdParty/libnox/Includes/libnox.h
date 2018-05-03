#ifndef LIBNOX_H
#define LIBNOX_H

#include "noxtypes.h"

/*
 * Public facing header for LibNOX - Nox Futura as a service.
 * Because we are trying to support Unreal and similar services, we need to provide a very low-tech
 * interface.
 */

namespace nf {

	/*
	* Test function that returns a string with the current version number.
	*/
	const char * get_version();

	void set_game_def_path(const char * base_path);
	void setup_planet();
	void serialize_planet();
	void setup_raws();
	void load_game();
	void chunks_init();
	void chunks_update();
	void chunks_update_list_dirty(size_t &size, int *& dirty_ptr);
	void chunk_world_coordinates(const int &idx, int &x, int &y, int &z);
	void chunk_floors(const int &chunk_idx, const int &chunk_z, size_t &size, floor_t *& floor_ptr);
	void chunk_cubes(const int &chunk_idx, const int &chunk_z, size_t &size, cube_t *& cube_ptr);
	void chunk_models(const int &chunk_idx, size_t &size, static_model_t *& model_ptr);
	void get_camera_position(float &x, float &y, float &z, float &zoom, bool &perspective, int &mode);
	void camera_zoom_in();
	void camera_zoom_out();
	void camera_move(const int &x, const int &y, const int &z);
	void toggle_camera_mode();
	void toggle_camera_perspective();
	void voxel_render_list(size_t &size, dynamic_model_t *& model_ptr);
	void lightsource_list(size_t &size, dynamic_lightsource_t *& light_ptr);
	hud_info_t get_hud_info();
	void water_cubes(size_t &size, water_t *& water_ptr);
	int get_pause_mode();
	void set_pause_mode(int mode);
	void on_tick(const double duration_ms);
	extern bool water_dirty;
	bool is_world_loadable();
	const char * get_game_subtitle();
	void get_unit_list_settlers(size_t &size, unit_list_settler_t *& settler_ptr);
	void get_unit_list_natives(size_t &size, unit_list_settler_t *& settler_ptr);
	void get_unit_list_wildlife(size_t &size, unit_list_settler_t *& settler_ptr);
	void zoom_settler(int id);
	void follow_settler(int id);
	void set_world_pos_from_mouse(int x, int y, int z);
	tooltip_info_t get_tooltip_info();
	void get_game_mode(int &major, int &minor);
	void set_game_mode(int major, int minor);
	void get_settler_job_list(size_t &size, settler_job_t *& job_ptr);
	void make_miner(int id);
	void make_farmer(int id);
	void make_lumberjack(int id);
	void fire_miner(int id);
	void fire_lumberjack(int id);
	void fire_farmer(int id);
	void cursor_list(size_t &size, cube_t *& cube_ptr);
	void guardmode_set();
	void guardmode_clear();
}

#endif