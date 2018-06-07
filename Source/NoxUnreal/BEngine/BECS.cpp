// (c) 2016 - Present, Bracket Productions

#include "BECS.h"
#include "../Public/NoxGameInstance.h"
#include "../Planet/NDisplayManager.h"
#include "../Raws/NRaws.h"
#include "bittools.h"
#include "../Planet/constants.h"
#include "../Planet/regiondefs.h"
#include "../Planet/NRegion.h"
#include "../Planet/NPlanet.h"

UNPlanet * planet;
UNRegion * region;
NRaws * raws;
ANDisplayManager * dm;

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace astar {

	namespace impl {

		constexpr static float MAX_DIRECT_PATH_CHECK = 24.0f;
		constexpr static int Z_WEIGHT = 10;
		constexpr static int MAX_ASTAR_STEPS = 500;		

		struct node_t
		{
			int idx = 0;
			float f = 0.0f;
			float g = 0.0f; // Movement cost to this square
			float h = 0.0f; // Estimated cost from here to the final destination

			bool operator<(const node_t &other) const
			{
				return f < other.f; // We're inverting the priority queue!
			}
		};


		class a_star_t
		{
		public:
			a_star_t(const position_t &start_pos, const position_t &end_pos) noexcept : start_(region->mapidx(start_pos.x, start_pos.y, start_pos.z)), end_(region->mapidx(end_pos.x, end_pos.y, end_pos.z)), end_loc_(end_pos)
			{
				end_x_ = end_pos.x;
				end_y_ = end_pos.y;
				end_z_ = end_pos.z;
				open_list_.Reserve(500);
				closed_list_.Reserve(300);
				parents_.Reserve(600);

				open_list_.Emplace(node_t{ start_, 0.0f, 0.0f, 0.0f });
			}

			float distance_to_end(const int &idx) const
			{
				const auto IDX = region->idxmap(idx);
				auto x = IDX.Get<0>();
				auto y = IDX.Get<1>();
				auto z = IDX.Get<2>();
				return distance3d_squared(x, y, z*Z_WEIGHT, end_x_, end_y_, end_z_*Z_WEIGHT); // We're weighting zs because stairs tend to not be where you want them
			}

			bool add_successor(const node_t &q, const int &idx)
			{
				// Did we reach our goal?
				if (idx == end_)
				{
					parents_.Add(idx, q.idx);
					return true;
				}
				else
				{
					const auto distance = distance_to_end(idx);
					const node_t s{ idx, distance + 1.0f, 1.0f, distance };

					auto should_add = true;

					// If a node with the same possition as successor is in the open list with a lower f, skip add
					for (const auto &e : open_list_)
					{
						if (e.f <= s.f) {
							if (e.idx == idx)
							{
								should_add = false;
								goto skipper;
							}
						}
						else
						{
							goto skipper;
						}
					}

				skipper:

					// If a node with the same position as successor is in the closed list, with a lower f, skip add
					{
						const auto closed_finder = closed_list_.Find(idx);
						if (closed_finder != nullptr)
						{
							if (*closed_finder <= s.f) should_add = false;
						}
					}

					if (should_add) {
						open_list_.Emplace(s);
						//if (parents_.find(idx) != parents_.end()) parents_.erase(idx);
						parents_.Add(idx, q.idx);
					}
				}
				return false;
			}

			navigation_path_t found_it(const int &idx) const noexcept
			{
				auto result = navigation_path_t{};
				result.success = true;
				result.destination = end_loc_;

				result.steps.Insert(end_loc_, 0);
				auto current = end_;
				while (current != start_)
				{
					const auto parent = parents_.Find(current);
					auto IDX = region->idxmap(*parent);
					auto x = IDX.Get<0>();
					auto y = IDX.Get<1>();
					auto z = IDX.Get<2>();
					if (*parent != start_) result.steps.Insert(position_t{ x,y,z }, 0);
					current = *parent;
				}

				return result;
			}

			navigation_path_t search() noexcept
			{
				auto result = navigation_path_t();

				while (!open_list_.Num()==0 && step_counter_ < MAX_ASTAR_STEPS)
				{
					++step_counter_;

					// Pop Q off of the list
					const auto q = open_list_[0];
					open_list_.RemoveAt(0);
					auto IDX = region->idxmap(q.idx);
					auto x = IDX.Get<0>();
					auto y = IDX.Get<1>();
					auto z = IDX.Get<2>();

					// Generate successors
					TArray<int> successors;
					if (testbit(regiondefs::tile_flags::CAN_GO_NORTH, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x, y - 1, z));
					if (testbit(regiondefs::tile_flags::CAN_GO_SOUTH, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x, y + 1, z));
					if (testbit(regiondefs::tile_flags::CAN_GO_WEST, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x - 1, y, z));
					if (testbit(regiondefs::tile_flags::CAN_GO_EAST, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x + 1, y, z));
					if (testbit(regiondefs::tile_flags::CAN_GO_UP, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x, y, z + 1));
					if (testbit(regiondefs::tile_flags::CAN_GO_DOWN, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x, y, z - 1));
					if (testbit(regiondefs::tile_flags::CAN_GO_NORTH_EAST, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x + 1, y - 1, z));
					if (testbit(regiondefs::tile_flags::CAN_GO_NORTH_WEST, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x - 1, y - 1, z));
					if (testbit(regiondefs::tile_flags::CAN_GO_SOUTH_EAST, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x + 1, y + 1, z));
					if (testbit(regiondefs::tile_flags::CAN_GO_SOUTH_WEST, region->TileFlags[q.idx])) successors.Emplace(region->mapidx(x - 1, y + 1, z));

					for (const auto &s : successors)
					{
						if (add_successor(q, s))
						{
							// We found it!
							const auto success = found_it(s);
							result.success = success.success;
							result.steps = success.steps;
							result.destination = success.destination;
							goto BAILOUT;
						}
					}

					if (closed_list_.Find(q.idx) != nullptr) closed_list_.Remove(q.idx);
					closed_list_.Add(q.idx, q.f);
					open_list_.Sort();
				}
				result.success = false;
			BAILOUT:
				/*
				std::cout << "Usage stats from completed search:\n";
				std::cout << "Open list size: " << open_list_.size() << "\n";
				std::cout << "Closed list size: " << closed_list_.size() << "\n";
				std::cout << "Parents size: " << parents_.size() << "\n";
				*/
				return result;
			}

		private:
			int start_;
			int end_;
			int end_x_, end_y_, end_z_;
			TArray<node_t> open_list_;
			TMap<int, float> closed_list_;
			TMap<int, int> parents_;
			position_t end_loc_;
			int step_counter_ = 0;
		};

		static navigation_path_t a_star(const position_t &start, const position_t &end) noexcept
		{
			a_star_t searcher(start, end);
			return searcher.search();
		}

		static navigation_path_t short_direct_line_optimization(const position_t &start, const position_t &end) noexcept
		{
			auto result = navigation_path_t();
			auto blocked = false;
			TSet<int> seen_nodes;
			line_func(start.x, start.y, end.x, end.y, [&blocked, &start, &result, &seen_nodes](const int &x, const int &y)
			{
				const auto idx = region->mapidx(x, y, start.z);
				if (!seen_nodes.Contains(idx)) {
					if (idx != region->mapidx(start.x, start.y, start.z)) result.steps.Emplace(position_t{ x, y, start.z });
					seen_nodes.Add(idx);
				}
				if (!testbit(regiondefs::tile_flags::CAN_STAND_HERE, region->TileFlags[idx])) blocked = true;
			});
			if (!blocked) result.success = true;

			return result;
		}

		static navigation_path_t find_path(const position_t &start, const position_t &end) noexcept
		{
			// Step 2 - check for the simple straight line option on short, flat paths
			/*const auto distance = bengine::distance3d(start.x, start.y, start.z, end.x, end.y, end.z);
			if (distance < MAX_DIRECT_PATH_CHECK && start.z == end.z)
			{
			auto result = short_direct_line_optimization(start, end);
			if (result->success) {
			return result;
			}
			}*/

			// Step 3 - Try A*
			auto result = a_star(start, end);
			return result;
		}

	}

	navigation_path_t find_path(const position_t &start, const position_t &end, const bool find_adjacent, const std::size_t civ) noexcept
	{
		using namespace nfu;

		// Step 1 - check clipping on both ends; the path auto-fails fast if there is an out-of-bounds issue.
		if (start.x < 1 || start.x > REGION_WIDTH - 1 || start.y < 1 || start.y > REGION_HEIGHT - 1 || start.z < 1 || start.z > REGION_DEPTH + 1
			|| end.x < 1 || end.x > REGION_WIDTH - 1 || end.y < 1 || end.y > REGION_HEIGHT - 1 || end.z < 1 || end.z > REGION_DEPTH + 1)
		{
			auto fail = navigation_path_t();
			fail.success = false;
			return fail;
		}

		if (start == end)
		{
			auto stay = navigation_path_t();
			stay.success = true;
			stay.destination = end;
			stay.steps.Insert(start, 0);
			return stay;
		}

		constexpr auto num_paths = 10;

		auto result = impl::find_path(start, end);
		if (find_adjacent && !result.success) {
			TArray<position_t> candidates{
				position_t{ end.x - 1, end.y, end.z },
				position_t{ end.x + 1, end.y, end.z },
				position_t{ end.x, end.y - 1, end.z },
				position_t{ end.x, end.y + 1, end.z },
				position_t{ end.x + 1, end.y + 1, end.z },
				position_t{ end.x - 1, end.y + 1, end.z },
				position_t{ end.x + 1, end.y - 1, end.z },
				position_t{ end.x - 1, end.y - 1, end.z },
				position_t{ end.x, end.y, end.z - 1 },
				position_t{ end.x, end.y, end.z + 1 },
			};
			for (const auto &candidate : candidates)
			{
				result = impl::find_path(start, candidate);
				if (result.success) return result;
			}
		}
		return result;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

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
			switch (settler->major_task) {
			case major_tasks::SLEEP: BedTime(entity, pos); break;
			default: { settler->busy = false; settler->minor_task = 0; }
			}
			return;
		}

		// What shift are we on?
		auto shift = GetSettlerShift(entity, settler);
		switch (shift) {
		case SLEEP_SHIFT: {
			BedTime(entity, pos);
		} break;
		case LEISURE_SHIFT: {
			LeisureTime(entity, pos);
		} break;
		case WORK_SHIFT: {
			WorkTime(entity, pos);
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

void UBECS::BedTime(const int &entity, const position_t * pos) {
	auto settler = ecs.GetComponent<settler_ai_t>(entity);
	if (!settler) return;

	if (!settler->busy) {
		settler->busy = true;
		settler->major_task = major_tasks::SLEEP;
		settler->minor_task = 1;
	}

	if (settler->minor_task == 1) {
		// Find a bed
		int chosen_bed = -1;
		position_t chosen_pos;
		int distance = 10000;
		ecs.EachWithout<claimed_t, position_t, construct_provides_sleep_t>([&chosen_pos, &distance, &pos, &chosen_bed](const int &bed, position_t &bedpos, construct_provides_sleep_t &sleep) {
			const auto dist = distance3d_squared(pos->x, pos->y, pos->z, bedpos.x, bedpos.y, bedpos.z);
			if (dist < distance) {
				distance = dist;
				chosen_pos = bedpos;
				chosen_bed = bed;
			}
		});

		if (chosen_bed > -1) {
			auto path = astar::find_path(*pos, chosen_pos, false, 0);
			if (path.success) {
				settler->claimed_entity = chosen_bed;
				ecs.Assign(chosen_bed, claimed_t{ entity });
				settler->target_position = chosen_pos;
				settler->path = path;
				settler->minor_task = 2;
				return;
			}
		}

		if (chosen_bed == -1) {
			dm->onEmote(entity, TEXT("Sleeping on the ground SUCKS"));
			settler->minor_task = 3;
			auto SleepClock = ecs.GetComponent<sleep_clock_t>(entity);
			SleepClock->sleep_requirement = 12;
			SleepClock->is_sleeping = true;
			settler->claimed_entity = -1;
		}
	}
	else if (settler->minor_task == 2) {

		// Path to the bed
		FollowPath(entity, pos, settler->path,
			// On Success
			[&settler, &entity, this]() {
				settler->minor_task = 3;
				auto SleepClock = ecs.GetComponent<sleep_clock_t>(entity);
				SleepClock->sleep_requirement = 8;
				SleepClock->is_sleeping = true;
				dm->onEmote(entity, "Sleeping in bed");
			},

			// On Fail
			[&settler]() { settler->minor_task = 1; }
		);
	}	

	if (hour_elapsed) {
		auto SleepClock = ecs.GetComponent<sleep_clock_t>(entity);
		if (SleepClock) {
			if (SleepClock->is_sleeping) {
				if (SleepClock->sleep_requirement > 0) {
					--SleepClock->sleep_requirement;
					dm->onEmote(entity, TEXT("Zzzz"));
				}
				else {
					dm->onEmote(entity, TEXT("*YAWN*"));
					settler->busy = false;
					settler->major_task = 0;
					settler->minor_task = 0;
					SleepClock->sleep_requirement = 8;
					if (settler->claimed_entity > -1) ecs.Remove<claimed_t>(settler->claimed_entity);
				}
			}
		}
	}
}

void UBECS::WorkTime(const int &entity, const position_t * pos) {
	auto settler = ecs.GetComponent<settler_ai_t>(entity);
	if (!settler) return;

	if (settler->designated_lumberjack) {
		if (LumberjackStart(entity, settler, pos)) return;
	}
	if (settler->designated_miner) {
		if (MiningStart(entity, settler, pos)) return;
	}
	if (settler->designated_farmer) {
		if (FarmingStart(entity, settler, pos)) return;
	}
	if (settler->designated_hunter) {
		if (HuntingStart(entity, settler, pos)) return;
	}
	if (BuildingStart(entity, settler, pos)) return;
	if (WorkOrdersStart(entity, settler, pos)) return;
	if (ArchitectureStart(entity, settler, pos)) return;
	if (ButcherStart(entity, settler, pos)) return;
	if (StockpilesStart(entity, settler, pos)) return;
	if (DeconstructionStart(entity, settler, pos)) return;

	// We still don't have anything to do!
	WanderAimlessley(entity, pos);
}

void UBECS::LeisureTime(const int &entity, const position_t * pos) {
	dm->onEmote(entity, TEXT("ME time!"));
	// Eating and drinking

	WanderAimlessley(entity, pos);
}

bool UBECS::CanEnterTile(const position_t &pos) {
	const int idx = region->mapidx(pos.x, pos.y, pos.z);
	return testbit(regiondefs::tile_flags::CAN_STAND_HERE, region->TileFlags[idx]);
}

bool UBECS::LumberjackStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	if (Designations->chopping.Num() == 0) return false; // No chopping to do
	return false;
}

bool UBECS::MiningStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	if (MiningDesignations->mining_targets.Num() == 0) return false;
	return false;
}

bool UBECS::FarmingStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	return false;
}

bool UBECS::HuntingStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	return false;
}

bool UBECS::BuildingStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	return false;
}

bool UBECS::WorkOrdersStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	return false;
}

bool UBECS::ArchitectureStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	return false;
}

bool UBECS::ButcherStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	return false;
}

bool UBECS::StockpilesStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	return false;
}

bool UBECS::DeconstructionStart(const int &entity, settler_ai_t * settler, const position_t *pos) {
	return false;
}