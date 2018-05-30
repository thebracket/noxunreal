// (c) 2016 - Present, Bracket Productions

#include "BECS.h"

void UBECS::LinkMasters() {
	ecs.Each<world_position_t, calendar_t, designations_t, logger_t, camera_options_t, mining_designations_t, farming_designations_t, building_designations_t, architecture_designations_t>(
		[this](int &entity, world_position_t &pos, calendar_t &cal, designations_t &design,
			logger_t &log, camera_options_t &camera_prefs, mining_designations_t &mining,
			farming_designations_t &farming, building_designations_t &building,
			architecture_designations_t &arch)
	{
		CameraEntity = entity;
		RegionX = pos.world_x;
		RegionY = pos.world_y;
		CameraPosition = &pos;
		Calendar = &cal;
		Designations = &design;
		//logger = &log;
		Camera = &camera_prefs;
		MiningDesignations = &mining;
		FarmingDesignations = &farming;
		BuildingDesignations = &building;
		ArchitectureDesignations = &arch;
	});
}


