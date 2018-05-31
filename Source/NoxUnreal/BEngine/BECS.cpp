// (c) 2016 - Present, Bracket Productions

#include "BECS.h"
#include "../Public/NoxGameInstance.h"
#include "../Planet/NDisplayManager.h"

UNPlanet * planet;
UNRegion * region;
ANDisplayManager * dm;

void UBECS::LinkMasters(UNPlanet * p, UNRegion * r) {
	planet = p;
	region = r;
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

void UBECS::LinkDM(ANDisplayManager * ndm) {
	dm = ndm;
}

void UBECS::SetPauseMode(const int &p) {
	pause_mode = p;
	dm->OnPauseChange.Broadcast(p);
}

void UBECS::GameTick() {
	// Ticker
	major_tick = false;
	slow_tick = false;
	hour_elapsed = false;
	day_elapsed = false;

	if (pause_mode != 0) {
		major_tick = true;
		++slow_ticker_count;
		if (slow_ticker_count > 9) {
			slow_tick = true;
			slow_ticker_count = 0;
		}

		if (pause_mode == 1) SetPauseMode(0);
	}

	// Things to do...
	if (major_tick) {
		RunCalendar();
		if (day_elapsed) RunSettlerSpawner();
	}
}

void UBECS::RunCalendar() {
	auto hour = Calendar->hour;
	auto day = Calendar->day;
	Calendar->next_minute();
	hour_elapsed = false;
	day_elapsed = false;
	if (Calendar->hour != hour) hour_elapsed = true;
	if (Calendar->day != day) day_elapsed = true;
	dm->OnCalendarChange.Broadcast(Calendar->get_date_time());
}

void UBECS::RunHunger() {
	ecs.Each<hunger_t>([](const int &e, hunger_t &h)
	{
		--h.hunger_clock;
		if (h.hunger_clock < 60) h.is_hungry = true;
		if (h.hunger_clock < -(24 * 60 * 3)) h.is_starving = true;
	});

	ecs.Each<thirst_t>([](const int &e, thirst_t &h)
	{
		--h.thirst_clock;
		if (h.thirst_clock < 60) h.is_thirsty = true;
		if (h.thirst_clock < -(24 * 60 * 3)) h.is_dehydrating = true;
	});
}

void UBECS::RunSettlerSpawner() {
	using namespace nfu;

	// New arrivals
	if (day_elapsed && planet->RemainingSettlers > 0) {
		++planet->MigrantCounter;
		if (planet->MigrantCounter > 14 && !planet->StrictBeamdown) { // Every 2 weeks
			const int crash_x = REGION_WIDTH / 2;
			const int crash_y = REGION_HEIGHT / 2;
			const int crash_z = region->GroundZ(crash_x, crash_y);

			const TArray<position_t> settler_arrival_points{
				{ crash_x - 2, crash_y - 2, crash_z + 1 },
			{ crash_x - 1, crash_y - 2, crash_z + 1 },
			{ crash_x, crash_y - 2, crash_z + 1 },
			{ crash_x + 1, crash_y - 2, crash_z + 1 },
			{ crash_x - 3, crash_y, crash_z + 1 },
			{ crash_x - 2, crash_y, crash_z + 1 },
			{ crash_x - 1, crash_y, crash_z + 1 },
			{ crash_x, crash_y, crash_z + 1 },
			{ crash_x + 1, crash_y, crash_z + 1 }
			};


			planet->MigrantCounter = 0;
			const int new_settler_count = FMath::Min(planet->RemainingSettlers, rng.RollDice(1, 6));
			planet->RemainingSettlers -= new_settler_count;

			for (auto i = 0; i < new_settler_count; ++i) {
				const position_t spawn_point = settler_arrival_points[i % settler_arrival_points.Num()];
				region->CreateSettler(spawn_point.x, spawn_point.y, spawn_point.z, &rng, rng.RollDice(1, 3) - 1);
				//particles::block_destruction_effect(spawn_point.x, spawn_point.y, spawn_point.z, 1.0f, 1.0f, 1.0f, particles::PARTICLE_SMOKE);
			}
		}
	}
}