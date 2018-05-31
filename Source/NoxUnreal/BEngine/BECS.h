// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Planet/GameComponents.h"
#include "BECS.generated.h"

template <typename T, typename... Ts>
struct Index;

template <typename T, typename... Ts>
struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct Index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};

template <typename T, typename... Ts>
constexpr std::size_t Index_v = Index<T, Ts...>::value;

template<class ... Components>
class ecs_t {
public:
	TTuple<TMap<int, Components>...> ComponentStorage;
	TMap<int, TBitArray<TFixedAllocator<sizeof ... (Components)>>> EntityHasComponents;
	int NextEntityId = 0;

	template <typename C>
	constexpr size_t GetComponentIndex() {
		return Index<C, Components...>::value;
	}

	int AddEntity() {
		const int id = NextEntityId;
		TBitArray<TFixedAllocator<sizeof ... (Components)>> bits;
		for (int i = 0; i < sizeof ... (Components); ++i) bits.Add(false);
		EntityHasComponents.Add(id, bits);
		++NextEntityId;

		return id;
	}

	template <typename FUNC, int N, typename T>
	void _ForEachCS(const FUNC &func) {
		static_assert(N < sizeof ... (Components), "Invalid component store index");
		func(ComponentStorage.Get<N>());
	}

	template <typename FUNC, int N, typename T, typename T2, typename ... Rest>
	void _ForEachCS(const FUNC &func) {
		static_assert(N < sizeof ... (Components), "Invalid component store index");
		func(ComponentStorage.Get<N>());
		_ForEachCS<FUNC, N + 1, T2, Rest...>(func);
	}

	template <typename FUNC>
	constexpr void _ForEachComponentStore(const FUNC &func) {
		_ForEachCS<FUNC, 0, Components...>(func);
	}

	void DeleteEntity(const int &id) {
		EntityHasComponents.Remove(id);
		_ForEachComponentStore([&id](auto &cs) { if (cs.Contains(id)) cs.Remove(id); });
	}

	template <class C>
	constexpr void Assign(const int &EntityID, C && component) {
		constexpr size_t familyId = Index<C, Components...>::value;
		ComponentStorage.Get<familyId>().Add(EntityID, component);
		EntityHasComponents[EntityID][familyId] = true;
	}

	template <class C>
	constexpr void Remove(const int &EntityID) {
		constexpr size_t familyId = Index<C, Components...>::value;
		ComponentStorage.Get<familyId>().Remove(EntityID);
		EntityHasComponents[EntityID][familyId] = false;
	}

	constexpr void Empty() {
		EntityHasComponents.Empty();
		_ForEachComponentStore([](auto &cs) { cs.Empty(); });
	}

	template <class C>
	bool HasComponent(const int &EntityId) {
		constexpr size_t familyId = Index<C, Components...>::value;
		return EntityHasComponents[EntityId][familyId];
	}

	bool EntityExists(const int &EntityId) {
		return EntityHasComponents.Contains(EntityId);
	}

	template <class C>
	C * GetComponent(const int &EntityId) {
		if (!EntityHasComponents.Contains(EntityId)) return nullptr;
		const size_t id = Index<C, Components...>::value;
		if (!EntityHasComponents[EntityId][id]) return nullptr;
		auto &store = ComponentStorage.Get<Index<C, Components...>::value>();
		auto result = store.Find(EntityId);
		return result;
	}

	template <class ... Cs, typename FUNC>
	void Each(const FUNC &f) {
		size_t ids[sizeof ... (Cs)]{ Index<Cs, Components...>::value... };
		for (auto &bitset : EntityHasComponents) {
			bool has_all = true;
			for (size_t i = 0; i < sizeof ... (Cs); ++i) {
				if (bitset.Value[ids[i]] != true) has_all = false;
			}
			if (has_all) {
				f(bitset.Key, ComponentStorage.Get<Index<Cs, Components...>::value>()[bitset.Key]...);
			}
		}
	}

	template <class ... Cs, typename PREDICATE, typename FUNC>
	void EachIf(const PREDICATE &predicate, const FUNC &f) {
		size_t ids[sizeof ... (Cs)]{ Index<Cs, Components...>::value... };
		for (auto &bitset : EntityHasComponents) {
			bool has_all = true;
			for (size_t i = 0; i < sizeof ... (Cs); ++i) {
				if (bitset.Value[ids[i]] != true) has_all = false;
			}
			if (has_all) {
				if (predicate(bitset.Key, ComponentStorage.Get<Index<Cs, Components...>::value>()[bitset.Key]...))
					f(bitset.Key, ComponentStorage.Get<Index<Cs, Components...>::value>()[bitset.Key]...);
			}
		}
	}

	template <class EXCLUDE, class ... Cs, typename FUNC>
	void EachWithout(const FUNC &f) {
		constexpr size_t excludeId = Index<EXCLUDE, Components...>::value;
		size_t ids[sizeof ... (Cs)]{ Index<Cs, Components...>::value... };
		for (auto &bitset : EntityHasComponents) {
			bool has_all = true;
			for (size_t i = 0; i < sizeof ... (Cs); ++i) {
				if (bitset.Value[ids[i]] != true) has_all = false;
			}
			if (has_all && bitset.Value[excludeId == false]) {
				f(bitset.Key, ComponentStorage.Get<Index<Cs, Components...>::value>()[bitset.Key]...);
			}
		}
	}

	template <class EXCLUDE, class EXCLUDE2, class ... Cs, typename FUNC>
	void EachWithoutBoth(const FUNC &f) {
		constexpr size_t excludeId = Index<EXCLUDE, Components...>::value;
		constexpr size_t excludeId2 = Index<EXCLUDE2, Components...>::value;
		size_t ids[sizeof ... (Cs)]{ Index<Cs, Components...>::value... };
		for (auto &bitset : EntityHasComponents) {
			bool has_all = true;
			for (size_t i = 0; i < sizeof ... (Cs); ++i) {
				if (bitset.Value[ids[i]] != true) has_all = false;
			}
			if (has_all && bitset.Value[excludeId == false] && bitset.Value[excludeId2 == false]) {
				f(bitset.Key, ComponentStorage.Get<Index<Cs, Components...>::value>()[bitset.Key]...);
			}
		}
	}
};

/**
 * Holder for the main ECS
 */
UCLASS()
class NOXUNREAL_API UBECS : public UObject
{
	GENERATED_BODY()
	
public:
	ecs_t<world_position_t, calendar_t, designations_t, logger_t, camera_options_t, mining_designations_t, farming_designations_t, 
		building_designations_t, architecture_designations_t, building_t, name_t, construct_provides_sleep_t, construct_container_t,
		item_stored_t, renderable_t, item_t, item_quality_t, item_wear_t, item_creator_t, item_chopping_t, item_digging_t, item_melee_t,
		item_ranged_t, item_ammo_t, item_food_t, item_spice_t, item_drink_t, item_hide_t, item_bone_t, item_skull_t, item_leather_t,
		item_farming_t, item_seed_t, item_topsoil_t, item_fertilizer_t, item_food_prepared_t, viewshed_t, lightsource_t, construct_power_t,
		smoke_emitter_t, construct_door_t, receives_signal_t, turret_t, initiative_t, position_t, species_t, game_stats_t, skill_t,
		health_t, settler_ai_t, renderable_composite_t, ai_settler_new_arrival_t, sleep_clock_t, hunger_t, thirst_t, item_carried_t>
		
		ecs;
	
	int CameraEntity = -1;
	int RegionX = -1;
	int RegionY = -1;
	world_position_t * CameraPosition = nullptr;
	calendar_t * Calendar = nullptr;
	designations_t * Designations = nullptr;
	camera_options_t * Camera = nullptr;
	mining_designations_t * MiningDesignations = nullptr;
	farming_designations_t * FarmingDesignations = nullptr;
	building_designations_t * BuildingDesignations;
	architecture_designations_t * ArchitectureDesignations;

	void LinkMasters();
};
