// (c) 2016 - Present, Bracket Productions

#include "BECS.h"
#include "../Public/NoxGameInstance.h"
#include "../Planet/NDisplayManager.h"
#include "../Raws/NRaws.h"

UNPlanet * planet;
UNRegion * region;
NRaws * raws;
ANDisplayManager * dm;

void UBECS::LinkMasters(UNPlanet * p, UNRegion * r, NRaws * raw) {
	planet = p;
	region = r;
	raws = raw;

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
	using namespace nfu;

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
		// Clean up
		MoveRequests.Empty();
		DamageRequests.Empty();

		// Run the Logic!
		RunCalendar();
		if (day_elapsed) RunSettlerSpawner();
		RunInitiative(MyTurn);
		for (auto &id : MyTurn) {
			RunAI(id);
		}
		ProcessMovement();
		
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

void UBECS::RunInitiative(TArray<int> &my_turn) {
	my_turn.Empty();
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
}

inline bool is_stuck_or_invalid(const position_t &pos) {
	using namespace nfu;
	const int map_index = region->mapidx(pos.x, pos.y, pos.z);
	return ((map_index < 0 || map_index >(REGION_HEIGHT*REGION_WIDTH*REGION_DEPTH)) || testbit(regiondefs::tile_flags::SOLID, region->TileFlags[map_index]));
}

void UBECS::RunAI(const int &entity) {
	auto pos = ecs.GetComponent<position_t>(entity);
	auto settler = ecs.GetComponent<settler_ai_t>(entity);
	auto turret = ecs.GetComponent<turret_t>(entity);

	/* Pre-Run Status Effects */
	auto health = ecs.GetComponent<health_t>(entity);
	if (health) {
		if (health->unconscious) return; // No turn for you!
		if (health->stunned_counter > 0) {
			--health->stunned_counter;
			return; // No turn for you!
		}
	}

	auto hunger = ecs.GetComponent<hunger_t>(entity);
	if (hunger && hunger->is_starving) {
		DamageRequests.Emplace(damage_request_t{ entity, 1, pos->x, pos->y, pos->z, "Starvation" });
	}
	auto thirst = ecs.GetComponent<thirst_t>(entity);
	if (thirst && thirst->is_dehydrating) {
		DamageRequests.Emplace(damage_request_t{ entity, 1, pos->x, pos->y, pos->z, "Dehydration" });
	}

	/* Pre-Run Stuck Check */
	// TODO: auto sentient = ecs.GetComponent<sentient_ai_t>(entity);
	// TODO: auto grazer = ecs.GetComponent<grazer_ai>(entity);
	if (settler && pos && is_stuck_or_invalid(*pos)) {
		// Activate emergency teleport
		ecs.Each<position_t, construct_provides_sleep_t>([&entity, &pos, this](const int &bedId, position_t &bedpos, construct_provides_sleep_t &bed) {
			MoveRequests.Emplace(move_request_t{ entity, pos->x, pos->y, pos->z, bedpos.x, bedpos.y, bedpos.z });
		});
		return; // No turn for you!
	}

	if (settler) {
		// Settler behavior tree

		// TODO: Visibility scan - am I in immediate danger?
		
		// New Arrival
		auto new_arrival = ecs.GetComponent<ai_settler_new_arrival_t>(entity);
		if (new_arrival) {
			SettlerNewArrival(entity, new_arrival);
			return;
		}

		// Should we keep doing a task?
		if (settler->busy) {
			// Keep doing it...
			return;
		}

		// What shift are we on?
		auto shift = GetSettlerShift(entity, settler);
		switch (shift) {
		case SLEEP_SHIFT: {
			WanderAimlessley(entity, pos);
		} break;
		case LEISURE_SHIFT: {
			WanderAimlessley(entity, pos);
		} break;
		case WORK_SHIFT: {
			WanderAimlessley(entity, pos);
		}break;
		}
	}
	else if (turret) {
		// Turret behavior tree
	} // etc. for grazers and sentients
}

void UBECS::ProcessMovement() {
	using namespace nfu;

	for (auto &move : MoveRequests) {
		if (ecs.EntityExists(move.entity) && move.endX > 0 && move.endX < REGION_WIDTH - 1 && move.endY > 0 && move.endY < REGION_HEIGHT - 1 && move.endZ > 0 && move.endZ < REGION_DEPTH - 1) {
			auto pos = ecs.GetComponent<position_t>(move.entity);
			if (pos) {
				pos->x = move.endX;
				pos->y = move.endY;
				pos->z = move.endZ;
				const auto deltaX = move.endX - move.startX;
				const auto deltaY = move.endY - move.endY;
				if (deltaX > 0 && deltaY > 0) {
					// South-East
					pos->rotation = 315;
				}
				else if (deltaX > 0 && deltaY < 0) {
					// North-East
					pos->rotation = 225;
				}
				else if (deltaX < 0 && deltaY < 0) {
					// North-West
					pos->rotation = 135;
				}
				else if (deltaX < 0 && deltaY > 0) {
					// South-West
					pos->rotation = 45;
				}
				else if (deltaX > 0) {
					// East
					pos->rotation = 270;
				}
				else if (deltaX < 0) {
					// West
					pos->rotation = 90;
				}
				else if (deltaY < 0) {
					// North
					pos->rotation = 180;
				}
				else if (deltaY > 0) {
					// South
					pos->rotation = 0;
				}
				dm->OnCompositeMoved.Broadcast(move.entity);
			}
		}
	}
}

void UBECS::SettlerNewArrival(const int &entity, ai_settler_new_arrival_t * new_arrival) {
	// New arrival
	if (new_arrival->turns_since_arrival == 0 && rng.RollDice(1, 6) == 1) {
		// Send a quip!
		const FString quip = raws->string_tables[string_tables::NEW_ARRIVAL_QUIPS].random_entry(rng);
		dm->OnSettlerEmote.Broadcast(entity, quip);
	}

	++new_arrival->turns_since_arrival;
	if (new_arrival->turns_since_arrival > 10) {
		ecs.Remove<ai_settler_new_arrival_t>(entity);
	}
}

shift_type_t UBECS::GetSettlerShift(const int &entity, settler_ai_t * ai) {
	const int shift_id = ai->shift_id;
	const int hour_of_day = Calendar->hour;
	auto current_schedule = Calendar->defined_shifts[shift_id].hours[hour_of_day];

	return current_schedule;
}

void UBECS::WanderAimlessley(const int &entity, const position_t * pos) {
	using namespace regiondefs;

	int destX = pos->x;
	int destY = pos->y;
	int destZ = pos->z;

	const auto tile_index = region->mapidx(destX, destY, destZ);
	const auto direction = rng.RollDice(1, 10);
	switch (direction) {
	case 1: if (testbit(tile_flags::CAN_GO_UP, region->TileFlags[tile_index])) ++destZ; break;
	case 2: if (testbit(tile_flags::CAN_GO_DOWN, region->TileFlags[tile_index])) --destZ; break;
	case 3: if (testbit(tile_flags::CAN_GO_NORTH, region->TileFlags[tile_index])) --destY; break;
	case 4: if (testbit(tile_flags::CAN_GO_SOUTH, region->TileFlags[tile_index])) ++destY; break;
	case 5: if (testbit(tile_flags::CAN_GO_EAST, region->TileFlags[tile_index])) ++destX; break;
	case 6: if (testbit(tile_flags::CAN_GO_WEST, region->TileFlags[tile_index])) --destX; break;
	case 7: if (testbit(tile_flags::CAN_GO_NORTH_EAST, region->TileFlags[tile_index])) { --destY; ++destX; } break;
	case 8: if (testbit(tile_flags::CAN_GO_NORTH_WEST, region->TileFlags[tile_index])) { --destY; --destX; } break;
	case 9: if (testbit(tile_flags::CAN_GO_SOUTH_EAST, region->TileFlags[tile_index])) { ++destY; ++destX; } break;
	case 10: if (testbit(tile_flags::CAN_GO_SOUTH_WEST, region->TileFlags[tile_index])) { ++destY; --destX; } break;
	}

	const auto destIdx = region->mapidx(destX, destY, destZ);
	if (destIdx != tile_index && region->WaterLevel[destIdx] < 3) MoveRequests.Emplace(move_request_t{ entity, pos->x, pos->y, pos->z, destX, destY, destZ });
}