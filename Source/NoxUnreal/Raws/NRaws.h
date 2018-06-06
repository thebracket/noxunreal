// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "NStringTable.h"
#include <lua.hpp>
#include "Runtime/Core/Public/Containers/BitArray.h"
#include "NRaws.generated.h"

USTRUCT(BlueprintType)
struct FNColor
{
	GENERATED_USTRUCT_BODY()

	FNColor() = default;
	FNColor(uint8 red, uint8 green, uint8 blue) {
		r = (float)red / 255.0f;
		g = (float)green / 255.0f;
		b = (float)blue / 255.0f;
	}
	FNColor(float R, float G, float B) : r(R), g(G), b(B) {}

	UPROPERTY(BlueprintReadWrite)
	float r = 0.0;

	UPROPERTY(BlueprintReadWrite)
	float g = 0.0;

	UPROPERTY(BlueprintReadWrite)
	float b = 0.0;
};

namespace rawdefs {

	enum material_def_spawn_type_t { NO_SPAWN_TYPE, CLUSTER_ROCK, ROCK, SOIL, SAND, METAL, SYNTHETIC, ORGANIC, LEATHER, FOOD, SPICE, BLIGHT };

	constexpr int NUMBER_OF_ITEM_CATEGORIES = 64;
	constexpr int COMPONENT = 0;
	constexpr int TOOL_CHOPPING = 1;
	constexpr int TOOL_DIGGING = 2;
	constexpr int WEAPON_MELEE = 3;
	constexpr int WEAPON_RANGED = 4;
	constexpr int WEAPON_AMMO = 5;
	constexpr int ITEM_FOOD = 6;
	constexpr int ITEM_SPICE = 7;
	constexpr int ITEM_DRINK = 8;
	constexpr int ITEM_HIDE = 9;
	constexpr int ITEM_BONE = 10;
	constexpr int ITEM_SKULL = 11;
	constexpr int ITEM_LEATHER = 12;
	constexpr int ITEM_FARMING = 13;
	constexpr int ITEM_SEED = 14;
	constexpr int ITEM_TOPSOIL = 15;
	constexpr int ITEM_FERTILIZER = 16;
	constexpr int ITEM_FOOD_PREPARED = 17;

	/*
	* Defines a region biome type.
	*/
	struct biome_type_t {
		FString name = "";
		int8_t min_rain = 0;
		int8_t max_rain = 100;
		int8_t min_temp = -100;
		int8_t max_temp = 100;
		uint8_t min_mutation = 0;
		uint8_t max_mutation = 100;
		uint8_t soil_pct = 50;
		uint8_t sand_pct = 50;
		TArray<uint8_t> occurs;
		unsigned int worldgen_texture_index = 0;
		TArray<TPair<FString, int>> plants;
		TArray<FString> wildlife;
		int deciduous_tree_chance = 0;
		int evergreen_tree_chance = 0;
		TArray<FString> nouns;
		int food_bonus = 0;
		float traversal_cost = 1.0f;
	};

	/*
	* Defines a material type. Used by just about everything.
	*/
	struct material_def_t {
		FString tag = "";
		FString name = "";
		material_def_spawn_type_t spawn_type = ROCK;
		FString parent_material_tag = "";
		uint16_t glyph;
		FNColor fg;
		FNColor bg;
		uint8_t hit_points = 0;
		FString mines_to_tag = "";
		FString mines_to_tag_second = "";
		FString layer = "";
		TArray<FString> ore_materials;
		int damage_bonus = 0;
		float ac_bonus = 0.0F;
		FString floor_rough;
		FString floor_smooth;
		FString wall_rough;
		FString wall_smooth;
		int floor_rough_id;
		int floor_smooth_id;
		int wall_rough_id;
		int wall_smooth_id;
	};

	/*
	* Defines template for clothes. These need to be merged into items eventually.
	*/
	struct clothing_t {
		FString name = "";
		TArray<FString> colors;
		FString slot = "";
		FString description = "";
		float armor_class = 0.0F;
		uint16_t clothing_glyph = 0;
		int clothing_layer = 0;
		int voxel_model = 0;
	};

	/*
	* Defines a template for events that can happen to a settler during character generation.
	*/
	struct life_event_template {
		int min_age = 0;
		int max_age = 0;
		FString description = "";
		int weight = 0;
		short strength = 0;
		short dexterity = 0;
		short constitution = 0;
		short intelligence = 0;
		short wisdom = 0;
		short charisma = 0;
		short comeliness = 0;
		short ethics = 0;
		TArray<FString> skills;
		TArray<FString> requires_event;
		TArray<FString> precludes_event;
	};

	/*
	* Defines a starting profession.
	*/
	struct profession_t {
		FString name = "";
		short strength = 0;
		short dexterity = 0;
		short constitution = 0;
		short intelligence = 0;
		short wisdom = 0;
		short charisma = 0;
		short comeliness = 0;
		short ethics = 0;
		TArray< TTuple< uint8_t, FString, FString >> starting_clothes;
	};

	struct stockpile_def_t {
		int index = 0;
		FString name = "";
		FString tag = "";
	};

	struct item_def_t {
		item_def_t() {
			categories = TBitArray<FDefaultBitArrayAllocator>(false, 64);
		}

		FString tag = "";
		FString name = "";
		FString description = "";
		TBitArray<FDefaultBitArrayAllocator> categories;
		uint16_t glyph;
		uint16_t glyph_ascii = 1;
		FNColor fg;
		FNColor bg;
		int damage_n = 0, damage_d = 0, damage_mod = 0;
		int range = 0;
		FString ammo;
		int stack_size = 1;
		int initiative_penalty = 0;
		FString damage_stat = "";
		int stockpile_idx = 0;
		int voxel_model = 0;
		int clothing_layer = 0;
	};

	enum provides_t {
		provides_sleep, provides_food, provides_seating, provides_desk, provides_door,
		provides_wall, provides_floor, provides_stairs_up, provides_stairs_down, provides_stairs_updown,
		provides_ramp, provides_light, provides_cage_trap, provides_stonefall_trap, provides_blades_trap,
		provides_spikes, provides_lever, provides_signal_recipient, provides_storage, provides_pressure_plate,
		provides_oscillator, provides_and_gate, provides_or_gate, provides_not_gate, provides_nor_gate,
		provides_nand_gate, provides_xor_gate, provides_float_gauge, provides_proximity_sensor,
		provides_support
	};

	struct building_provides_t {
		provides_t provides;
		int energy_cost = 0;
		int radius = 0;
		int alternate_vox = 0;
		FNColor color{ 1.0f, 1.0f, 1.0f };
	};

	struct reaction_input_t {
		FString tag = "";
		size_t required_material = 0;
		material_def_spawn_type_t required_material_type = NO_SPAWN_TYPE;
		int quantity = 0;
	};

	struct building_def_t {
		FString tag = "";
		std::size_t hashtag = 0;
		FString description = "";
		FString name = "";
		TArray<reaction_input_t> components;
		TPair<FString, int> skill;
		TArray<building_provides_t> provides;
		int width = 1;
		int height = 1;
		//TArray<xp::vchar> glyphs; // TODO
		//TArray<xp::vchar> glyphs_ascii;
		bool emits_smoke = false;
		bool structure = false;
		int vox_model = 0;
		FString blocked = "";
	};

	constexpr int special_reaction_cooking = 0;
	constexpr int special_reaction_tanning = 1;

	struct reaction_t {
		reaction_t() {
			specials = TBitArray<FDefaultBitArrayAllocator>(false, 2);
		}

		FString tag = "";
		size_t hashtag = 0;
		FString name = "";
		FString workshop = "";
		TArray<reaction_input_t> inputs;
		TArray<TPair<FString, int>> outputs;
		FString skill = "";
		int difficulty = 10;
		bool automatic = false;
		int power_drain = 0;
		bool emits_smoke = false;
		TBitArray<FDefaultBitArrayAllocator> specials;
	};

	constexpr int PLANT_SPREADS = 0;
	constexpr int PLANT_ANNUAL = 1;

	/*
	* Defines a plant template.
	*/
	struct plant_t {
		FString tag = "";
		FString name = "";
		TArray<int> lifecycle;
		//std::vector<xp::vchar> glyphs_ascii;
		TArray<FString> provides;
		//std::bitset<2> tags;
		bool requires_light = true;
	};

	/*
	* Defines species dietary preferences
	*/
	enum diet_t { diet_omnivore, diet_herbivore, diet_carnivore };

	/*
	* Defines base species alignment. This will be replaced at some point.
	*/
	enum alignment_t { align_good, align_neutral, align_evil, align_devour };

	/*
	* Basic definition of a species.
	*/
	struct body_part_t {
		FString name;
		int qty;
		int size;
	};

	struct raw_species_t {
		FString tag = "";
		FString name = "";
		FString male_name = "";
		FString female_name = "";
		FString collective_name = "";
		FString description = "";
		TMap<FString, int> stat_mods;
		TArray<body_part_t> body_parts;
		diet_t diet = diet_omnivore;
		alignment_t alignment = align_neutral;
		bool spreads_blight = false;
		int max_age = 90;
		int infant_age = 5;
		int child_age = 12;
		uint16_t glyph = '@';
		uint16_t glyph_ascii = '@';
		uint16_t worldgen_glyph = '@';
		bool render_composite = false;
		uint16_t base_male_glyph = 352;
		uint16_t base_female_glyph = 353;
		int voxel_model = 0;
		TArray<TPair<FString, FNColor>> skin_colors;
		TArray<TPair<FString, FNColor>> hair_colors;
	};

	/*
	* Defines a creature's built-in attack type.
	*/
	struct creature_attack_t {
		FString type = "";
		int hit_bonus = 0;
		int damage_n_dice = 1;
		int damage_dice = 6;
		int damage_mod = 0;
	};

	/*
	* What type of critter AI should the creature use?
	*/
	enum creature_ai_t { creature_grazer };

	/*
	* Defines a creature template.
	*/
	struct raw_creature_t {
		FString tag = "";
		FString name = "";
		FString male_name = "";
		FString female_name = "";
		FString collective_name = "";
		FString description = "";
		TMap<FString, int> stats;
		TArray<body_part_t> body_parts;
		int armor_class = 10;
		TArray<creature_attack_t> attacks;
		int yield_hide = 0;
		int yield_meat = 0;
		int yield_bone = 0;
		int yield_skull = 0;
		creature_ai_t ai;
		uint16_t glyph;
		uint16_t glyph_ascii;
		int vox = 0;
		FNColor fg;
		int hp_n, hp_dice, hp_mod;
		int group_size_n_dice, group_size_dice, group_size_mod;
	};


	/*
	* Defines systems of government
	*/
	struct raw_government_t {
		FString tag = "";
		FString name = "";
		float tax = 0.0f;
		int black_market = 0;
		int base_relationship_mod = 0;
	};

	/*
	* Defines world-gen level units
	*/
	struct raw_unit_t {
		FString tag;
		FString name;
		int cost;
		int attack_strength;
		int defense_strength;
		int max_size;
		int tech_level;
	};
}



// Helpers

struct lua_parser_inner_t {
	FString key;
	TFunction<void()> func;
};

inline TMap<FString, const TFunction<void()>> lua_parser(std::initializer_list<lua_parser_inner_t> &init) {
	TMap<FString, const TFunction<void()>> result;
	for (auto &n : init) {
		result.Add(n.key, n.func);
	}	
	return result;
}

inline FString to_proper_noun_case(const FString &original)
{
	FString result = original.Mid(1, original.Len()-1).ToLower();
	result = original.Mid(0, 1).ToUpper() + result;
	// TODO:
	return result;
}

/**
 * 
 */
class NOXUNREAL_API NRaws
{
public:
	NRaws();
	~NRaws();

	TMap<int8, NStringTable> string_tables;	

	void LoadRaws();
	void LoadGameTables();

	lua_State * lua_state;
	void InitLua();
	void ExitLua();
	void LoadLuaScript(const FString &filename);
	FNColor ReadLuaColor(const FString &field);

	/*
	* Iterate all biomes
	*/
	template <typename FUNC>
	inline void each_biome(const FUNC &&func) noexcept {
		for (int i=0; i<biome_defs.Num(); ++i) {
			func(&biome_defs[i]);
		}
	}

	inline rawdefs::biome_type_t * get_biome_def(const size_t &index) {
		if (index > biome_defs.Num()-1) return nullptr;
		return &biome_defs[index];
	}

	inline rawdefs::material_def_t * get_material(const size_t &index) {
		return &material_defs[index];
	}

	inline size_t get_material_by_tag(const FString &tag) {
		const auto finder = material_defs_idx.Find(tag);
		return finder != nullptr ? *finder : 0;
	}

	inline size_t get_plant_idx(const FString &tag) noexcept {
		const auto finder = plant_defs_idx.Find(tag);
		return finder == nullptr ? 0 : *finder;
	}

	inline rawdefs::plant_t * get_plant_def(const std::size_t &index) noexcept
	{
		if (index > plant_defs.Num()) return nullptr;
		return &plant_defs[index];
	}

	inline rawdefs::building_def_t * get_building_def(const FString &tag) noexcept {
		auto finder = building_defs.Find(tag);
		return finder;
	}

	inline rawdefs::item_def_t * get_item_def(const FString &tag) noexcept {
		auto finder = item_defs.Find(tag);
		return finder;
	}

	inline rawdefs::clothing_t * get_clothing_by_tag(const FString &tag) noexcept {
		auto finder = clothing_types.Find(tag);
		return finder;
	}

	inline FString material_name(const std::size_t &id) noexcept {
		if (id < material_defs.Num()) return material_defs[id].name;
		FString name = "Unknown material: ";
		name.AppendInt(id);
		return name;
	}

	inline rawdefs::raw_species_t * get_species_def(const FString &tag) noexcept {
		auto finder = species_defs.Find(tag);
		return finder;
	}

	inline rawdefs::profession_t * get_random_profession(RandomNumberGenerator &rng) noexcept {
		auto roll = rng.RollDice(1, starting_professions.Num()) - 1;
		return &starting_professions[roll];
	}

	inline rawdefs::life_event_template * get_life_event(const FString &tag) noexcept {
		auto finder = life_event_defs.Find(tag);
		return finder;
	}

	template <typename FUNC>
	void each_life_event(const FUNC &func) noexcept {
		for (auto &it : life_event_defs) {
			func(it.Key, &it.Value);
		}
	}

	inline rawdefs::raw_creature_t * get_creature_def(const FString &tag) noexcept {
		auto finder = creature_defs.Find(tag);
		return finder;
	}

	bool LoadedRaws = false;

	TMap<FString, size_t> material_defs_idx;
	TArray<rawdefs::material_def_t> material_defs;
	TMap<int, FString> texture_atlas;
	TMap<FString, rawdefs::clothing_t> clothing_types;
	TMap<FString, rawdefs::life_event_template> life_event_defs;
	TArray<rawdefs::profession_t> starting_professions;
	TMap<int, rawdefs::stockpile_def_t> stockpile_defs;
	int clothing_stockpile = 0;
	TMap<FString, rawdefs::item_def_t> item_defs;
	TMap<FString, rawdefs::building_def_t> building_defs;
	TMap<int, rawdefs::reaction_t> reaction_defs;
	TMap<int, TArray<FString>> reaction_building_defs;
	TMap<FString, int> plant_defs_idx;
	TArray<rawdefs::plant_t> plant_defs;
	TArray<rawdefs::biome_type_t> biome_defs;
	TMap<FString, rawdefs::raw_species_t> species_defs;
	TMap<FString, rawdefs::raw_creature_t> creature_defs;
	TArray<rawdefs::raw_government_t> government_defs;
	TArray<rawdefs::raw_unit_t> unit_defs;

	void ReadMaterialTypes();
	void ReadClothing();
	void ReadLifeEvents();
	void ReadProfessions();
	void ReadStockpiles();
	void ReadItems();
	void ReadBuildings();
	void ReadReactions();
	void ReadPlantTypes();
	void ReadBiomeTypes();
	void ReadSpeciesTypes();
	void ReadCreatureTypes();
	void ReadGovernmentTypes();
	void ReadUnitTypes();

	// Templatey Stuff
	inline void call_functor_key(const TMap<FString, const TFunction<void()>> &parser, const FString &field, const FString &table) {
		const auto finder = parser.Find(field);
		if (finder != nullptr) {
			const auto functor = *finder;
			functor();
		}
	}

	template<typename ONSTART, typename ONEND>
	inline void read_lua_table(const FString &table, const ONSTART &&on_start, const ONEND &&on_end, const TMap<FString, const TFunction<void()>> &parser) noexcept
	{
		lua_getglobal(lua_state, TCHAR_TO_ANSI(*table));
		lua_pushnil(lua_state);
		while (lua_next(lua_state, -2) != 0)
		{
			const FString key = lua_tostring(lua_state, -2);

			on_start(key);
			lua_pushstring(lua_state, TCHAR_TO_ANSI(*key));
			lua_gettable(lua_state, -2);
			while (lua_next(lua_state, -2) != 0) {
				const FString field = lua_tostring(lua_state, -2);
				call_functor_key(parser, field, table);
				lua_pop(lua_state, 1);
			}
			on_end(key);

			lua_pop(lua_state, 1);
		}
	}

	void read_lua_table_inner(const FString &table, const TFunction<void(FString)> &functor)  noexcept;
	void read_lua_table_inner_p(const FString &table, const TFunction<void(FString)> &on_start, const TFunction<void(FString)> &on_end, const TMap<FString, const TFunction<void()>> &parser) noexcept;

	inline FString lua_str() noexcept { return lua_tostring(lua_state, -1); }
	inline int lua_int() noexcept { return static_cast<int>(lua_tonumber(lua_state, -1)); }
	inline float lua_float() noexcept { return static_cast<float>(lua_tonumber(lua_state, -1)); }

	template<class T>
	void read_stat_modifiers(const FString &table, T &obj) noexcept {
		read_lua_table_inner("modifiers", [&obj, this](auto stat) {
			if (stat == "str") obj.strength = lua_int();
			if (stat == "dex") obj.dexterity = lua_int();
			if (stat == "con") obj.constitution = lua_int();
			if (stat == "int") obj.intelligence = lua_int();
			if (stat == "wis") obj.wisdom = lua_int();
			if (stat == "cha") obj.charisma = lua_int();
			if (stat == "com") obj.comeliness = lua_int();
			if (stat == "eth") obj.ethics = lua_int();
		});
	}
};
