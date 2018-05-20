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

	/*
	* Set the base path prefix for game loading; this needs to be relative to where the engine you are using puts
	* content. It maps world_defs and rex folders as children.
	*/
	void set_game_def_path(const char * base_path);

	/*
	* Loads the planet from disk and gets it ready to use.
	*/
	void setup_planet();

	/*
	* Saves the planet to disk.
	*/
	void serialize_planet();

	/*
	* Sets up the raws if they haven't already been loaded.
	*/
	void setup_raws();

	/*
	* Loads the current game state from disk.
	*/
	void load_game();

	/*
	 * Loads the materials map
	 */
	void get_materials_map(size_t &size, material_map_t *& mat_ptr);

	/*
	* Initializes the in-memory structures handling chunks.
	*/
	void chunks_init();

	/*
	* Orders the chunking engine to rebuild any dirty chunks.
	*/
	void chunks_update();

	/*
	* Retrieve a list of chunks that have been updated, and need to be re-rendered on the client side.
	*/
	void chunks_update_list_dirty(size_t &size, int *& dirty_ptr);

	/*
	* Converts x/y/z world coordinates into a chunk index.
	*/
	void chunk_world_coordinates(const int &idx, int &x, int &y, int &z);

	/*
	* Gets a list of floor tiles in a chunk.
	*/
	void chunk_floors(const int &chunk_idx, const int &chunk_z, size_t &size, floor_t *& floor_ptr);

	/*
	* Gets a list of cube tiles in a chunk.
	*/
	void chunk_cubes(const int &chunk_idx, const int &chunk_z, size_t &size, cube_t *& cube_ptr);

	/*
	* Gets a list of models in a chunk.
	*/
	void chunk_models(const int &chunk_idx, size_t &size, static_model_t *& model_ptr);

	void chunk_veg(const int &chunk_idx, size_t &size, veg_t *& veg_ptr);

	void chunk_design_mode(const int &chunk_idx, const int &chunk_z, size_t &size, floor_t *& floor_ptr);

	/*
	* Gets the current camera position
	*/
	void get_camera_position(float &x, float &y, float &z, float &zoom, bool &perspective, int &mode);

	/*
	* Instructs the camera to zoom in.
	*/
	void camera_zoom_in();

	/*
	* Instructs the camera to zoom out.
	*/
	void camera_zoom_out();

	/*
	* Moves the camera. The x/y/z are DELTA positions, added to the current position.
	*/
	void camera_move(const int &x, const int &y, const int &z);

	/*
	* Cycles through available camera modes.
	*/
	void toggle_camera_mode();

	/*
	* Toggles perspective / ortho
	*/
	void toggle_camera_perspective();

	/*
	* Gets a list of voxel models to render.
	*/
	void voxel_render_list(size_t &size, dynamic_model_t *& model_ptr);

	/*
	* Gets a list of lightsources to render.
	*/
	void lightsource_list(size_t &size, dynamic_lightsource_t *& light_ptr);

	/*
	* Gets the current info for the HUD.
	*/
	hud_info_t get_hud_info();

	/*
	* Gets a list of water cubes to render;
	*/
	void water_cubes(size_t &size, water_t *& water_ptr);

	/*
	* Retrieves the current pause mode.
	*/
	int get_pause_mode();

	/*
	* Sets the current pause mode.
	*/
	void set_pause_mode(int mode);

	/*
	* Runs all systems after an engine tick occurs.
	*/
	void on_tick(const double duration_ms);

	/*
	* Does water need re-rendering?
	*/
	extern bool water_dirty;

	/*
	* Can the world be loaded, or is a new one required?
	*/
	bool is_world_loadable();

	/*
	* Randomly generates the sub-title for the menu screen.
	*/
	const char * get_game_subtitle();

	/* 
	* Gets the list of settlers for the Units menu.
	*/
	void get_unit_list_settlers(size_t &size, unit_list_settler_t *& settler_ptr);

	/*
	* Gets the list of natives for the Units menu.
	*/
	void get_unit_list_natives(size_t &size, unit_list_settler_t *& settler_ptr);

	/*
	* Gets the list of wildlife for the Units menu.
	*/
	void get_unit_list_wildlife(size_t &size, unit_list_settler_t *& settler_ptr);

	/*
	* Zooms to a current settler position
	*/
	void zoom_settler(int id);

	/*
	* Orders the camera to follow a settler
	*/
	void follow_settler(int id);

	/*
	* Tells the game where the mouse is currently pointing, in world coordinates.
	*/
	void set_world_pos_from_mouse(int x, int y, int z);

	/*
	* Gets the tooltip for the current tile.
	*/
	tooltip_info_t get_tooltip_info();

	/*
	* Gets the current game mode.
	*/
	void get_game_mode(int &major, int &minor);

	/*
	* Sets the current game mode.
	*/
	void set_game_mode(int major, int minor);

	/*
	* Gets the current job list for the UI.
	*/
	void get_settler_job_list(size_t &size, settler_job_t *& job_ptr);

	/*
	* Settler of ID is now a miner.
	*/
	void make_miner(int id);

	/*
	* Settler of ID is now a farmer.
	*/
	void make_farmer(int id);

	/*
	* Settler of ID is now a lumberjack.
	*/
	void make_lumberjack(int id);

	/*
	* Settler of ID is no longer a miner.
	*/
	void fire_miner(int id);

	/*
	* Settler of ID is no longer a lumberjack.
	*/
	void fire_lumberjack(int id);

	/*
	* Settler of ID is no longer a farmer.
	*/
	void fire_farmer(int id);

	/*
	* Settler of ID is now a hunter.
	*/
	void make_hunter(int id);

	/*
	* Settler of ID is no longer a hunter.
	*/
	void fire_hunter(int id);
	
	/*
	* Gets the cursor list for rendering.
	*/
	void cursor_list(size_t &size, cube_t *& cube_ptr);

	/*
	* Set the current world tile to be guarded.
	*/
	void guardmode_set();

	/*
	* Set the current world tile to be unguarded.
	*/
	void guardmode_clear();

	/*
	* Set the current world tile to be chopped.
	*/
	void lumberjack_set();

	/*
	* Set the current world tile to not be chopped.
	*/
	void lumberjack_clear();	

	/*
	* Gets a list of buildings that can be constructed with on-hand materials.
	*/
	void available_buildings(size_t &size, buildable_building_t *& build_ptr);

	/*
	* Sets the currently selected building type (for design-build)
	*/
	void set_selected_building(int list_index);

	/*
	* Orders the creation of the current building.
	*/
	void place_selected_building();

	tooltip_info_t get_farm_yield();
	void harvest_set();
	void harvest_clear();

	void plantable_seeds(size_t &size, plantable_seed_t *& seed_ptr);
	void set_selected_seed(int list_index);
	void plant_set();
	void plant_clear();

	void set_mining_mode(int mode);
	int get_mining_mode();
	void mine_set();
	void mine_clear();

	void set_architecture_mode(int mode);
	int get_architecture_mode();
	void architecture_set();
	void architecture_clear();
	int get_available_block_count();
	int get_required_block_count();

	void workflow_menu(size_t &queue_size, queued_work_t *& queued_work_ptr, size_t &available_size, available_work_t *& available_work_ptr, size_t &standing_order_size, active_standing_order_t *& standing_order_ptr);
	void workflow_remove_from_queue(int index);
	void workflow_enqueue(int index);
	void workflow_add_so(int index);
	void workflow_remove_so(int index);
}

#endif