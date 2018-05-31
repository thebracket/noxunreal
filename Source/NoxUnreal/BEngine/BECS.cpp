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
		auto EntityTurn = RunInitiative();
		
		/*
		WE ARE PORTING... removing when done

		wildlife_population::run(ms);
			// fluids
			explosives::run(ms);
			doors::run(ms);
			gravity::run(ms);
			distance_map::run(ms);
			world::run(ms);
			if (day_elapsed) sentient_ai_system::run(ms);
			corpse_system::run(ms);
			mining_system::run(ms);
			architecture_system::run(ms);
			stockpile_system::run(ms);
			power::run(ms);
			workflow_system::run(ms);
			ai_status_effects::run(ms);
			ai_stuck::run(ms);
			ai_visibility_scan::run(ms);
			ai_new_arrival::run(ms);
			ai_scheduler::run(ms);
			ai_leisure_time::run(ms);
			ai_sleep_time::run(ms);
			ai_work_time::run(ms);
			ai_work_lumberjack::run(ms);
			ai_mining::run(ms);
			ai_guard::run(ms);
			ai_harvest::run(ms);
			ai_farm_plant::run(ms);
			ai_farm_fertilize::run(ms);
			ai_farm_clear::run(ms);
			ai_farm_fixsoil::run(ms);
			ai_farm_water::run(ms);
			ai_farm_weed::run(ms);
			ai_building::run(ms);
			ai_workorder::run(ms);
			ai_architect::run(ms);
			ai_hunt::run(ms);
			ai_butcher::run(ms);
			ai_work_stockpiles::run(ms);
			ai_deconstruction::run(ms);
			ai_leisure_eat::run(ms);
			ai_leisure_drink::run(ms);
			ai_idle_time::run(ms);
			movement::run(ms);
			triggers::run(ms);
			settler_ranged_attack::run(ms);
			settler_melee_attack::run(ms);
			sentient_attacks::run(ms);
			creature_attacks::run(ms);
			turret_attacks::run(ms);
			damage_system::run(ms);
			kill_system::run(ms);
			if (hour_elapsed) healing_system::run(ms);
			topology::run(ms);
			visibility::run(ms);
			vegetation::run(ms);
			if (day_elapsed) item_wear::run(ms);
		}
		inventory_system::run(ms);
		*/
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
				auto newId = region->CreateSettler(spawn_point.x, spawn_point.y, spawn_point.z, &rng, rng.RollDice(1, 3) - 1);
				//particles::block_destruction_effect(spawn_point.x, spawn_point.y, spawn_point.z, 1.0f, 1.0f, 1.0f, particles::PARTICLE_SMOKE);
				dm->OnCompositeAdded.Broadcast(newId);
			}
		}
	}
}

inline void calculate_initiative(initiative_t &ai, game_stats_t &stats, RandomNumberGenerator * rng) {
	ai.initiative = FMath::Max(1, rng->RollDice(1, 12) - stat_modifier(stats.dexterity) + ai.initiative_modifier);
}

TArray<int> UBECS::RunInitiative() {
	TArray<int> my_turn;
	ecs.Each<initiative_t>([this, &my_turn](const int &e, initiative_t &i) {
		--i.initiative;
		if (i.initiative + i.initiative_modifier < 1) {
			// It's my turn
			my_turn.Emplace(e);
			auto stats = ecs.GetComponent<game_stats_t>(e);
			if (stats) {
				calculate_initiative(i, *stats, &rng);
			}
			else {
				i.initiative = 10;
			}
			i.initiative_modifier = 0;
		}
	});
	return my_turn;
}