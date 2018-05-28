// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BECS.generated.h"

struct test_component {
	int n;
};

struct test_component2 {
	int n;
};

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
 * 
 */
UCLASS()
class NOXUNREAL_API UBECS : public UObject
{
	GENERATED_BODY()
	
public:

private:
	ecs_t<test_component> ecs;
	
	
};
