// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "../Raws/NRaws.h"
#include "../BEngine/RandomNumberGenerator.h"

struct position_t {
	int x = 0;
	int y = 0;
	int z = 0;
	int rotation = 0;

	position_t() = default;
	position_t(const int &nx, const int &ny, const int &nz) noexcept : x(nx), y(ny), z(nz) {}
	position_t(const float &nx, const float &ny, const int &nz) noexcept : x(static_cast<int>(nx)), y(static_cast<int>(ny)), z(nz) {}
	position_t(const int &nx, const int &ny, const int &nz, const int &rot) noexcept : x(nx), y(ny), z(nz), rotation(rot) {}
	position_t(const float &nx, const float &ny, const int &nz, const int &rot) noexcept : x(static_cast<int>(nx)), y(static_cast<int>(ny)), z(nz), rotation(rot) {}
	bool operator==(const position_t &rhs) const { return (x == rhs.x && y == rhs.y && z == rhs.z); }
};

namespace astar {
	struct navigation_path_t
	{
		bool success = false;
		TArray<position_t> steps{};
		position_t destination{ 0,0,0 };
	};
}

struct world_position_t {
	int world_x = 0;
	int world_y = 0;
	int region_x = 0;
	int region_y = 0;
	int region_z = 0;

	world_position_t(const int wx, const int wy, const int rx, const int ry, const int rz) noexcept :
		world_x(wx), world_y(wy), region_x(rx), region_y(ry), region_z(rz) {}
	world_position_t() = default;
};

enum shift_type_t { SLEEP_SHIFT, LEISURE_SHIFT, WORK_SHIFT };

struct shift_t {

	FString shift_name;
	TArray<shift_type_t> hours;
};

struct calendar_t {
	uint16_t year = 2525;
	uint8_t month = 0;
	uint8_t day = 0;
	uint8_t hour = 0;
	uint8_t minute = 0;
	uint8_t second = 0;
	float sun_x = 0.0f;
	float sun_y = 0.0f;
	float sun_z = 0.0f;
	float sun_r = 1.0f;
	float sun_g = 1.0f;
	float sun_b = 1.0f;
	float moon_x = 0.0f;
	float moon_y = 0.0f;
	float moon_z = 0.0f;
	float moon_r = 138.0f / 255.0f;
	float moon_g = 178.0f / 255.0f;
	float moon_b = 238.0f / 255.0f;

	TArray<shift_t> defined_shifts;

	FString get_date_time() const;
	void next_minute();
};

struct unbuild_t {
	unbuild_t() = default;
	unbuild_t(const bool &build, const std::size_t id) noexcept : is_building(build), building_id(id) {}
	bool is_building = true;
	std::size_t building_id;
};

struct guard_designation_t {
	bool active = false;
	position_t guardpost;
};

struct designations_t {

	TMap<size_t, position_t> chopping;
	TArray<guard_designation_t> guard_points;
	TArray<unbuild_t> deconstructions;
	TArray<size_t> levers_to_pull;
	int current_power = 10;
	uint64_t current_cash = 100;

	uint8_t standing_order_idle_move = 1;
	uint8_t standing_order_wildlife_treatment = 1;
	uint8_t standing_order_upgrade = 1;

	// Not serialized
	//color_t alert_color{ 1.0f, 1.0f, 1.0f };
	int total_capacity = 10;
};

// TODO:
struct logger_t {
	logger_t() = default;
};

enum game_camera_mode_t { TOP_DOWN, FRONT, DIAGONAL_LOOK_NW, DIAGONAL_LOOK_NE, DIAGONAL_LOOK_SW, DIAGONAL_LOOK_SE };

struct camera_options_t {

	camera_options_t() = default;
	camera_options_t(const game_camera_mode_t mode, const bool ascii, const int zoom) noexcept : camera_mode(mode), ascii_mode(ascii), zoom_level(zoom) {}

	game_camera_mode_t camera_mode = game_camera_mode_t::TOP_DOWN;
	bool ascii_mode = false;
	int zoom_level = 12;
	bool perspective = true;
	int following = 0;
	bool fps = false;
};

enum mining_type_t { MINE_DIG, MINE_CHANNEL, MINE_RAMP, MINE_STAIRS_UP, MINE_STAIRS_DOWN, MINE_STAIRS_UPDOWN, MINE_DELETE };

struct mining_designations_t {

	TMap<int, uint8> mining_targets;
	int brush_type = 0;
	int brush_size_x = 1;
	int brush_size_y = 1;
	int mine_mode = 0;
	int radius = 1;
	bool stairs_helper = true;
	int stairs_depth = 1;
};

namespace farm_steps {
	constexpr uint8_t CLEAR = 0;
	constexpr uint8_t FIX_SOIL = 1;
	constexpr uint8_t PLANT_SEEDS = 2;
	constexpr uint8_t GROWING = 3;
}

struct farm_cycle_t {
	farm_cycle_t() = default;
	farm_cycle_t(const uint8_t st, const FString t, const size_t seed) noexcept : state(st), seed_type(t), seed_id(seed) {}
	uint8_t state = 0;
	FString seed_type = "";
	int days_since_weeded = 0;
	int days_since_watered = 0;
	bool fertilized = false;
	std::size_t seed_id = 0;
};

struct harvest_designation_t {
	bool active = false;
	position_t pos;
};

struct farming_designations_t {

	TArray<harvest_designation_t> harvest;
	TMap<int, farm_cycle_t> farms;
};

struct reaction_input_t {
	FString tag = "";
	size_t required_material = 0;
	rawdefs::material_def_spawn_type_t required_material_type = rawdefs::material_def_spawn_type_t::NO_SPAWN_TYPE;
	int quantity = 0;

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(tag, required_material, required_material_type, quantity); // serialize things by passing them to the archive
	}
};

struct building_task_component_id {
	int id;
	bool ready;
};

struct building_designation_t {
	int x, y, z;
	TArray<building_task_component_id> component_ids;

	FString name;
	FString tag;
	TArray<reaction_input_t> components;
	int width;
	int height;
	int building_entity = 0;
};

struct build_order_t {
	int number;
	FString tag;
};

struct standing_build_order_t {
	FString item;
	int number;
	FString reaction;
};

struct building_designations_t {

	TArray<build_order_t> build_orders;
	TArray<standing_build_order_t> standing_build_orders; // Item, <#/Reaction>
};

struct architecture_designations_t {
	TMap<int, uint8_t> architecture;
};

struct building_built_from_t {
	FString tag;
	int material;
};

struct building_t {
	building_t(const FString &ntag, const int w, const int h, const bool comp, const std::size_t owner, const uint8_t maxhp,
		const uint8_t hp, const int vox) :
		tag(ntag), width(w), height(h), complete(comp), civ_owner(owner),
		max_hit_points(maxhp), hit_points(hp), vox_model(vox)
	{}

	FString tag;
	int width = 0, height = 0;
	bool complete = false;
	TArray<building_built_from_t> built_with;
	size_t civ_owner = 0;
	uint8_t max_hit_points = 10;
	uint8_t hit_points = 10;
	int vox_model = 0;

	building_t() = default;

};

struct name_t {
	FString first_name;
	FString last_name;

	name_t() = default;
	name_t(const FString &fn, const FString &ln) : first_name(fn), last_name(ln) {}
};

struct construct_provides_sleep_t {
};

struct construct_container_t {
};

struct item_stored_t {
	int stored_in;
	item_stored_t() = default;
	explicit item_stored_t(const int carrier) noexcept : stored_in(carrier) {}
};

struct renderable_t {
	uint16_t glyph = 0;
	uint16_t glyph_ascii = 0;
	int vox = 0;
	//bengine::color_t foreground{};
	//bengine::color_t background{};

	renderable_t() = default;
	renderable_t(const uint16_t ch, const uint16_t cha, const int vx = 0) noexcept : glyph(ch), glyph_ascii(cha), vox(vx) {}
};

enum item_type_t { CLOTHING, ITEM };

FNColor colname_to_col(const FString &col);

struct item_t {
	FString item_name;
	FString item_tag;
	item_type_t type;
	std::size_t material;
	int stack_size = 1;
	uint16_t clothing_glyph = 0;
	FNColor clothing_color;
	int clothing_layer = 0;

	item_t() = default;
	explicit item_t(const FString name, NRaws * raws, RandomNumberGenerator * rng) noexcept;
	item_t(const FString tag, const FString name, const std::size_t mat, int stack = 1, int clothing = 0, NRaws * raws = nullptr) noexcept;

};

namespace ecs_item_quality {
	constexpr uint8_t AWFUL = 1;
	constexpr uint8_t POOR = 2;
	constexpr uint8_t AVERAGE = 3;
	constexpr uint8_t ABOVE_AVERAGE = 4;
	constexpr uint8_t GOOD = 5;
	constexpr uint8_t GREAT = 6;
	constexpr uint8_t MASTERWORK = 7;
}

struct item_quality_t {

	item_quality_t() = default;
	explicit item_quality_t(const uint8_t q) noexcept : quality(q) {}
	uint8_t quality = 3;

	FString get_quality_text() const {
		switch (quality) {
		case 1: return FString("awful");
		case 2: return FString("poor");
		case 3: return FString("average");
		case 4: return FString("above average");
		case 5: return FString("good");
		case 6: return FString("great");
		case 7: return FString("masterwork");
		default: return "Unknown";
		}
	}
};

struct item_wear_t {

	item_wear_t() = default;
	explicit item_wear_t(const uint8_t w) noexcept : wear(w) {}
	uint8_t wear = 100;

	FString get_wear_text() const {
		if (wear < 10) return FString("falling apart");
		if (wear < 30) return FString("tattered");
		if (wear < 50) return FString("worn");
		if (wear < 75) return FString("lightly used");
		return FString("pristine");
	}
};

struct item_creator_t {

	item_creator_t() = default;
	item_creator_t(const int id, const FString name) : creator_id(id), creator_name(name) {}
	int creator_id = 0;
	FString creator_name = "";
};

struct item_chopping_t {
};

struct item_digging_t {
};

struct item_melee_t {
};

struct item_ranged_t {
};

struct item_ammo_t {
};

struct item_food_t {
};

struct item_spice_t {
};

struct item_drink_t {
};

struct item_hide_t {
};

struct item_bone_t {
};

struct item_skull_t {
};

struct item_leather_t {
};

struct item_farming_t {
};

struct item_seed_t {
	item_seed_t() = default;
	explicit item_seed_t(const FString &target) noexcept : grows_into(target) {}
	FString grows_into = "";
};

struct item_topsoil_t {
};

struct item_fertilizer_t {
};

struct item_food_prepared_t {
};

struct viewshed_t {
	viewshed_t() = default;
	viewshed_t(const int radius, const bool pen, const bool good_guy = true) noexcept : viewshed_radius(radius), penetrating(pen), good_guy_visibility(good_guy) {}

	int viewshed_radius = 0;
	bool penetrating = false;
	bool good_guy_visibility = true;
	TSet<size_t> visible_entities;

	// Non-persistent
	TSet<int> visible_cache;
};

struct lightsource_t {
	lightsource_t() = default;
	lightsource_t(const int r, const FNColor col) noexcept : radius(r), color(col) {}
	lightsource_t(const int r, const FNColor col, const bool alert) noexcept : radius(r), color(col), alert_status(alert) {}

	int radius = 0;
	FNColor color{ 1.0f, 1.0f, 1.0f };
	bool alert_status = false;

	// Non-persistent
	TArray<int> lit_cache;
};

struct construct_power_t {
	int storage_capacity;
	int generation_always;
	int generation_solar;

	construct_power_t() = default;
	construct_power_t(const int store, const int gen_all, const int gen_solar) : storage_capacity(store), generation_always(gen_all), generation_solar(gen_solar) {}
};

struct smoke_emitter_t {
};

struct construct_door_t {
	bool locked = false;
};

struct receives_signal_t {
	bool active = false;
	TArray<TTuple<int, int, int, int, int>> receives_from;
};

struct turret_t {
	turret_t() = default;
	turret_t(const int rng, const int hb, const int n, const int d, const int bonus, const std::size_t civ) noexcept
		: range(rng), hit_bonus(hb), damage_dice(n), damage_die(d), damage_bonus(bonus), owner_civilization(civ) {}

	int range = 6;
	int hit_bonus = 0;
	int damage_dice = 1;
	int damage_die = 6;
	int damage_bonus = 0;
	size_t owner_civilization = 0;
};

struct initiative_t {
	initiative_t() = default;
	explicit initiative_t(const int init_mod) noexcept : initiative_modifier(init_mod) {}
	int initiative = 0;
	int initiative_modifier = 0;
};

enum gender_t { MALE, FEMALE };
enum sexuality_t { HETEROSEXUAL, HOMOSEXUAL, BISEXUAL };
enum hair_color_t { WHITE_HAIR, BROWN_HAIR, BLACK_HAIR, BLONDE_HAIR, RED_HAIR };
enum hair_style_t { BALD, SHORT_HAIR, LONG_HAIR, PIGTAILS, MOHAWK, BALDING, TRIANGLE };

struct named_color_pair_t {
	FString name;
	FNColor color;
};

struct species_t {
	FString tag = "";
	std::size_t index = 0;
	gender_t gender;
	sexuality_t sexuality;
	hair_style_t hair_style;
	named_color_pair_t skin_color;
	named_color_pair_t hair_color;
	float height_cm;
	float weight_kg;
	bool bearded;
	uint16_t base_male_glyph;
	uint16_t base_female_glyph;

	FString gender_str();
	FString gender_pronoun();
	FString sexuality_str();
	FString height_feet();
	FString weight_lbs();
	FString ethnicity();
	FString hair_color_str();
	FString hair_style_str();
};

struct skill_t {
	skill_t() = default;
	skill_t(const int8_t lvl, const uint16_t &xp) noexcept : skill_level(lvl), experience_gained(xp) {}
	int16_t skill_level = 0;
	uint16_t experience_gained = 0;
};

inline short stat_modifier(const short &stat)
{
	if (stat == 1)
		return -5;
	return ((stat - 1) / 2) - 4;
}

struct game_stats_t {

	FString original_profession;
	FString profession_tag;
	short strength;
	short dexterity;
	short constitution;
	short intelligence;
	short wisdom;
	short charisma;
	short comeliness;
	short ethics;
	int age;
	TMap<FString, skill_t> skills;

	FString strength_str();
	FString dexterity_str();
	FString constitution_str();
	FString intelligence_str();
	FString wisdom_str();
	FString charisma_str();
	FString comeliness_str();
	FString ethics_str();
};

enum attributes_t { strength, dexterity, constitution, intelligence, wisdom, charisma, ethics };



inline TMap<FString, attributes_t> skill_table_parser(std::initializer_list<TPair<FString, attributes_t>> init) {
	TMap<FString, attributes_t> result;
	for (auto &n : init) {
		result.Add(n.Key, n.Value);
	}
	return result;
}

const TMap<FString, attributes_t> skill_table = skill_table_parser(std::initializer_list<TPair<FString, attributes_t>>{
	TPair<FString, attributes_t>{ "Mining", strength },
	TPair<FString, attributes_t>{ "Lumberjacking", strength },
	TPair<FString, attributes_t>{ "Construction", dexterity },
	TPair<FString, attributes_t>{ "Carpentry", dexterity },
	TPair<FString, attributes_t>{ "Masonry", strength },
	TPair<FString, attributes_t>{ "Furnace Operation", dexterity },
	TPair<FString, attributes_t>{ "Glassmaking", dexterity },
	TPair<FString, attributes_t>{ "Metalworking", strength },
	TPair<FString, attributes_t>{ "Swimming", strength },
	TPair<FString, attributes_t>{ "Melee Attacks", strength },
	TPair<FString, attributes_t>{ "Ranged Attacks", dexterity },
	TPair<FString, attributes_t>{ "Negotiation", charisma },
	TPair<FString, attributes_t>{ "Stealth", dexterity },
	TPair<FString, attributes_t>{ "Medicine", intelligence },
	TPair<FString, attributes_t>{ "Chemistry", intelligence },
	TPair<FString, attributes_t>{ "Literacy", intelligence },
	TPair<FString, attributes_t>{ "Acrobatics", dexterity },
	TPair<FString, attributes_t>{ "Farming", wisdom }
});

enum skill_roll_result_t { CRITICAL_FAIL, FAIL, SUCCESS, CRITICAL_SUCCESS };
constexpr int DIFFICULTY_TRIVIAL = 0;
constexpr int DIFFICULTY_EASY = 5;
constexpr int DIFFICULTY_AVERAGE = 10;
constexpr int DIFICULTY_TOUGH = 15;
constexpr int DIFFICULTY_CHALLENGING = 20;
constexpr int DIFFICULTY_FORMIDABLE = 25;
constexpr int DIFFICULTY_HEROIC = 30;
constexpr int DIFFICULTY_NEARLY_IMPOSSIBLE = 40;

int get_attribute_modifier_for_skill(const game_stats_t &stats, const FString &skill);
int16_t get_skill_modifier(const game_stats_t &stats, const FString &skill);
void gain_skill_from_success(const int settler_id, game_stats_t &stats, const FString &skill, const int &difficulty, RandomNumberGenerator * rng);
skill_roll_result_t skill_roll(const int settler_id, game_stats_t &stats, RandomNumberGenerator * rng, const FString skill_name, const int difficulty);
TTuple<skill_roll_result_t, int, int> skill_roll_ext(const int settler_id, game_stats_t &stats, RandomNumberGenerator * rng, const FString skill_name, const int difficulty);

struct health_part_t {
	FString part = "";
	int max_hitpoints = 0;
	int current_hitpoints = 0;
	int size = 0;
};

struct health_t {
	int max_hitpoints;
	int current_hitpoints;
	bool unconscious = false;
	bool blind = false;
	bool immobile = false;
	bool slow = false;
	bool no_grasp = false;
	int stunned_counter = 0;
	TArray<health_part_t> parts;
};

inline health_t create_health_component_sentient(const rawdefs::raw_species_t * species, const int base_hp) noexcept
{
	health_t result;

	result.max_hitpoints = base_hp;
	result.current_hitpoints = base_hp;
	for (const auto &part : species->body_parts) {
		const auto n_parts = part.qty;
		for (auto i = 0; i<n_parts; ++i) {
			health_part_t p;
			const auto pct = static_cast<float>(part.size / 100.0F);
			auto hitpoints = static_cast<float>(base_hp) * pct;
			if (hitpoints < 1.0F) hitpoints = 1.0F;
			p.part = part.name;
			p.max_hitpoints = static_cast<int>(hitpoints);
			p.current_hitpoints = static_cast<int>(hitpoints);
			p.size = part.size;
			result.parts.Emplace(p);
		}
	}
	return result;
}

inline health_t create_health_component_settler(const FString &tag, const int base_hp, NRaws * raws) noexcept {
	health_t result;

	result.max_hitpoints = base_hp;
	result.current_hitpoints = base_hp;

	//std::cout << "Finding parts for " << tag << "\n";
	auto species = *raws->get_species_def(tag);
	for (const auto &part : species.body_parts) {
		const auto n_parts = part.qty;
		for (auto i = 0; i<n_parts; ++i) {
			health_part_t p;
			const auto pct = static_cast<float>(part.size) / 100.0F;
			auto hitpoints = static_cast<float>(base_hp) * pct;
			if (hitpoints < 1.0F) hitpoints = 1.0F;
			p.part = part.name;
			p.max_hitpoints = static_cast<int>(hitpoints);
			p.current_hitpoints = static_cast<int>(hitpoints);
			p.size = part.size;
			result.parts.Emplace(p);
		}
	}

	return result;
}

inline health_t create_health_component_creature(const FString &tag, NRaws * raws, RandomNumberGenerator * rng) noexcept {
	health_t result;

	auto species = *raws->get_creature_def(tag);
	int base_hp = rng->RollDice(species.hp_n, species.hp_dice) + species.hp_mod;
	if (base_hp < 0) base_hp = 1;

	result.max_hitpoints = base_hp;
	result.current_hitpoints = base_hp;

	for (const auto &part : species.body_parts) {
		const auto n_parts = part.qty;
		for (auto i = 0; i<n_parts; ++i) {
			health_part_t p;
			const auto pct = static_cast<float>(part.size) / 100.0F;
			auto hitpoints = static_cast<float>(base_hp) * pct;
			if (hitpoints < 1.0F) hitpoints = 1.0F;
			p.part = part.name;
			p.max_hitpoints = static_cast<int>(hitpoints);
			p.current_hitpoints = static_cast<int>(hitpoints);
			p.size = part.size;
			result.parts.Emplace(p);
		}
	}

	return result;
}

namespace major_tasks {
	constexpr int SLEEP = 1;
}

struct settler_ai_t {
	int shift_id = 0;
	bool busy = false;
	int major_task = 0;
	int minor_task = 0;

	bool designated_lumberjack = false;
	bool designated_miner = false;
	bool designated_farmer = false;
	bool designated_hunter = false;

	FString status = "New Arrival";

	int claimed_entity = -1;
	position_t target_position;
	astar::navigation_path_t path;
};

enum render_mode_t { RENDER_SETTLER, RENDER_SENTIENT };

struct renderable_composite_t {
	render_mode_t render_mode = RENDER_SETTLER;
	uint16_t ascii_char = '@';

	renderable_composite_t() = default;
	explicit renderable_composite_t(const render_mode_t &rm) noexcept : render_mode(rm) {}
	renderable_composite_t(const render_mode_t &rm, const uint16_t &ascii) noexcept : render_mode(rm), ascii_char(ascii) {}
};

struct ai_settler_new_arrival_t {
	int turns_since_arrival = 0;
};

struct sleep_clock_t {
	int sleep_requirement = 0;
	bool is_sleeping = false;
};

struct hunger_t {
	int hunger_clock = 24 * 60; // 24 hours in minutes
	bool is_hungry = false;
	bool is_starving = false;
};

struct thirst_t {
	int thirst_clock = 24 * 60; // 24 hours in minutes
	bool is_thirsty = false;
	bool is_dehydrating = false;
};

enum ecs_item_location_t { INVENTORY, HEAD, TORSO, LEGS, FEET, EQUIP_TOOL, EQUIP_MELEE, EQUIP_RANGED, EQUIP_AMMO, EQUIP_SHIELD };

inline FString item_loc_name(const ecs_item_location_t &loc) {
	switch (loc) {
	case INVENTORY: return FString("Carried");
	case HEAD: return FString("Head");
	case TORSO: return FString("Torso");
	case LEGS: return FString("Legs");
	case FEET: return FString("Feet");
	case EQUIP_TOOL: return FString("Tool");
	case EQUIP_MELEE: return FString("Melee");
	case EQUIP_RANGED: return FString("Ranged");
	case EQUIP_AMMO: return FString("Ammunition");
	case EQUIP_SHIELD: return FString("Shield");
	}
	return FString("Unknown");
}

struct item_carried_t {
	ecs_item_location_t location = INVENTORY;
	int carried_by;
	item_carried_t() = default;
	item_carried_t(const ecs_item_location_t loc, const int carrier) : location(loc), carried_by(carrier) {}
};

struct claimed_t {
	claimed_t() = default;
	explicit claimed_t(const int &id) noexcept : claimed_by(id) {}
	int claimed_by = 0;
};
