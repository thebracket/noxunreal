// (c) 2016 - Present, Bracket Productions

#include "GameComponents.h"

FString calendar_t::get_date_time() const {
	FString result;
	
	result.AppendInt(day+1);
	result.Append("/");
	result.AppendInt(month+1);
	result.Append("/");
	result.AppendInt(year);
	result.Append(" ");
	result.AppendInt(hour);
	result.Append(":");
	result.AppendInt(minute);

	return result;
}

void calendar_t::next_minute() {
	minute += 1;
	if (minute > 59) { minute = 0; ++hour; }
	if (hour > 23) { hour = 0; ++day; }
	if (day > 30) { day = 0; ++month; }
	if (month > 11) { month = 0; ++year; }
}

FNColor colname_to_col(const FString &col)
{
	if (col == "black") return FNColor((uint8_t)50, 50, 64);
	if (col == "white") return FNColor((uint8_t)250, 250, 250);
	if (col == "red") return FNColor((uint8_t)250, 50, 50);
	if (col == "blue") return FNColor((uint8_t)50, 250, 50);
	if (col == "navy") return FNColor((uint8_t)50, 190, 50);
	if (col == "green") return FNColor((uint8_t)50, 50, 250);
	if (col == "grey") return FNColor((uint8_t)128, 128, 128);
	if (col == "yellow") return FNColor((uint8_t)250, 250, 50);
	if (col == "pink") return FNColor((uint8_t)250, 105, 180);
	if (col == "brown") return FNColor((uint8_t)98, 74, 46);
	if (col == "khaki") return FNColor((uint8_t)240, 230, 140);
	//glog(log_target::LOADER, log_severity::warning, "Unknown clothing color: {0}", col);
	return FNColor((uint8_t)250, 250, 250);
}

item_t::item_t(const FString name, NRaws * raws, RandomNumberGenerator * rng) noexcept : item_tag(name), type(CLOTHING) {
	//std::cout << "[" << item_tag << "]\n";
	auto finder = raws->get_item_def(item_tag);
	if (finder != nullptr) {
		item_name = finder->name;
	}
	else {
		auto finder2 = raws->get_clothing_by_tag(item_tag);
		if (finder2) {
			item_name = finder2->name;
			clothing_glyph = finder2->clothing_glyph;
			clothing_layer = finder2->clothing_layer;
			if (finder2->colors.Num() > 0) {
				clothing_color = colname_to_col(finder2->colors[rng->RollDice(1, finder2->colors.Num()) - 1]);
			}
		}
		else {
			item_name = "ERROR";
			//glog(log_target::LOADER, log_severity::error, "{0} not found", item_tag);
		}
	}
}

item_t::item_t(const FString tag, const FString name, const std::size_t mat, int stack, int clothing, NRaws * raws) noexcept :
	item_name(name), item_tag(tag), type(ITEM), material(mat), stack_size(stack)
{
	item_name = raws->material_name(mat) +FString(" ") + item_name;
	clothing_layer = clothing;
}

FString species_t::gender_str() {
	switch (gender) {
	case MALE: return "Male";
	case FEMALE: return "Female";
	default: return "Male";
	}
}

FString species_t::gender_pronoun() {
	switch (gender) {
	case MALE: return "He";
	case FEMALE: return "She";
	default: return "Xi";
	}
}

FString species_t::sexuality_str() {
	switch (sexuality) {
	case HETEROSEXUAL: return "Heterosexual";
	case HOMOSEXUAL: return "Homosexual";
	case BISEXUAL: return "Bi-sexual";
	default: return "Unknown";
	}
}

FString species_t::height_feet() {
	const float height_inches = height_cm / 2.5F;
	const int height_feet = (int)height_inches / 12;
	const int height_additional_inches = height_feet % 12;
	FString result;
	result.AppendInt(height_feet);
	result += "'";
	result.AppendInt(height_additional_inches);
	result += "\"";
	return result;
}

FString species_t::weight_lbs() {
	const float weight_lbs = weight_kg * 2.20462F;
	FString result;
	result.AppendInt((int)weight_lbs);
	return result;
}

FString species_t::ethnicity() {
	return skin_color.name;
}

FString species_t::hair_color_str() {
	const FString base = hair_color.name;
	FString result = "";

	for (std::size_t i = 0; i<base.Len(); ++i) {
			result += base[i];
	}

	return result;
}

FString species_t::hair_style_str() {
	switch (hair_style) {
	case BALD: return "bald";
	case SHORT_HAIR: return "cropped short";
	case LONG_HAIR: return "long";
	case PIGTAILS: return "in pigtails";
	case MOHAWK: return "in a mowhawk";
	case BALDING: return "balding";
	case TRIANGLE: return "cut into a triangle shape";
	default: return "unknown";
	}
}

FString game_stats_t::strength_str() {
	if (strength < 5) return "is very weak.";
	if (strength < 8) return "is weak.";
	if (strength < 12) return "is of average strength.";
	if (strength < 15) return "is strong.";
	if (strength < 18) return "is very strong.";
	return "has super-human strength";
}

FString game_stats_t::dexterity_str() {
	if (dexterity < 5) return "is very clumsy.";
	if (dexterity < 8) return "is clumsy.";
	if (dexterity < 12) return "is of average dexterity.";
	if (dexterity < 15) return "is agile.";
	if (dexterity < 18) return "is very agile.";
	return "has super-human dexterity";
}

FString game_stats_t::constitution_str() {
	if (constitution < 5) return "is very unhealthy.";
	if (constitution < 8) return "is unhealthy.";
	if (constitution < 12) return "is of average constitution.";
	if (constitution < 15) return "is healthy.";
	if (constitution < 18) return "is very healthy.";
	return "has super-human constitution";
}

FString game_stats_t::intelligence_str() {
	if (intelligence < 5) return "is very stupid.";
	if (intelligence < 8) return "is stupid.";
	if (intelligence < 12) return "is of average intelligence.";
	if (intelligence < 15) return "is intelligent.";
	if (intelligence < 18) return "is very intelligent.";
	return "has super-human intelligence";
}

FString game_stats_t::wisdom_str() {
	if (wisdom < 5) return "is very gullible.";
	if (wisdom < 8) return "is gullible.";
	if (wisdom < 12) return "is of average wisdom.";
	if (wisdom < 15) return "is wise.";
	if (wisdom < 18) return "is very wise.";
	return "has super-human wisdom";
}

FString game_stats_t::charisma_str() {
	if (charisma < 5) return "makes everyone uncomfortable.";
	if (charisma < 8) return "is socially awkward.";
	if (charisma < 12) return "is of average charisma.";
	if (charisma < 15) return "is socially adept.";
	if (charisma < 18) return "is always the center of attention.";
	return "has super-human charisma";
}

FString game_stats_t::comeliness_str() {
	if (comeliness < 5) return "is incredibly ugly.";
	if (comeliness < 8) return "is ugly.";
	if (comeliness < 12) return "is of average comeliness.";
	if (comeliness < 15) return "is good looking.";
	if (comeliness < 18) return "could be a supermodel.";
	return "has super-human appearance";
}

FString game_stats_t::ethics_str() {
	if (ethics < 5) return "would sell grandma for a profit.";
	if (ethics < 8) return "has a weak sense of justice.";
	if (ethics < 12) return "is of average ethics.";
	if (ethics < 15) return "has a strong sense of right and wrong.";
	if (ethics < 18) return "is a paragon of virtue.";
	return "is unbending in their beliefs, a modern-day Paladin.";
}

int get_attribute_modifier_for_skill(const game_stats_t &stats, const FString &skill) {
	auto finder = skill_table.Find(skill);
	if (finder != nullptr) {
		switch (*finder) {
		case strength: return stat_modifier(stats.strength);
		case dexterity: return stat_modifier(stats.dexterity);
		case constitution: return stat_modifier(stats.constitution);
		case intelligence: return stat_modifier(stats.intelligence);
		case wisdom: return stat_modifier(stats.wisdom);
		case charisma: return stat_modifier(stats.charisma);
		case ethics: return stat_modifier(stats.ethics);
		default: return 0;
		}
	}
	else {
		//glog(log_target::GAME, log_severity::error, "Unknown skill: {0}", skill);
		return 0;
	}
}

int16_t get_skill_modifier(const game_stats_t &stats, const FString &skill) {
	auto finder = stats.skills.Find(skill);
	if (finder != nullptr) {
		return finder->skill_level;
	}
	else {
		return 0;
	}
}

void gain_skill_from_success(const int settler_id, game_stats_t &stats, const FString &skill, const int &difficulty, RandomNumberGenerator  * rng) {
	auto finder = stats.skills.Find(skill);
	if (finder != nullptr) {
		finder->experience_gained += difficulty;
		const int gain_bonus_at = (finder->skill_level + 1) * 100;
		if (finder->experience_gained > gain_bonus_at) {
			//systems::logging::log_message msg{LOG{}.settler_name(settler_id)->text(" has improved their skill in ")->text(skill)->chars};
			//systems::logging::log(msg);
			finder->experience_gained = 0;
			++finder->skill_level;

			if (rng->RollDice(1, 6) == 1) {
				auto relevant_attribute = skill_table.Find(skill);
				if (relevant_attribute != nullptr) {
					const int stat_gain_roll = rng->RollDice(3, 6);
					int attribute_target = 0;
					switch (*relevant_attribute) {
					case strength: attribute_target = stats.strength; break;
					case dexterity: attribute_target = stats.dexterity; break;
					case constitution: attribute_target = stats.constitution; break;
					case intelligence: attribute_target = stats.intelligence; break;
					case wisdom: attribute_target = stats.wisdom; break;
					case charisma: attribute_target = stats.charisma; break;
					case ethics: attribute_target = stats.ethics; break;
					}
					if (stat_gain_roll < attribute_target) {
						//systems::logging::log_message msg{LOG{}.settler_name(settler_id)->text(" has gained an attribute point.")->chars};
						//systems::logging::log(msg);
						switch (*relevant_attribute) {
						case strength: ++stats.strength; break;
						case dexterity: ++stats.dexterity; break;
						case constitution: ++stats.constitution; break;
						case intelligence: ++stats.intelligence; break;
						case wisdom: ++stats.wisdom; break;
						case charisma: ++stats.charisma; break;
						case ethics: ++stats.ethics; break;
						}
					}
				}
			}
		}
	}
	else {
		stats.skills.Add(skill, skill_t{ 1, (uint16)difficulty });
	}
}

skill_roll_result_t skill_roll(const int settler_id, game_stats_t &stats, RandomNumberGenerator  * rng, const FString skill_name, const int difficulty) {
	const int luck_component = rng->RollDice(1, 20);
	const int natural_ability = get_attribute_modifier_for_skill(stats, skill_name);
	const int8_t person_skill = get_skill_modifier(stats, skill_name);
	const int total = luck_component + natural_ability + person_skill;

	//glog(log_target::GAME, log_severity::info, "{0} roll, difficulty {1}. 1d20 = {2}, + {3} (ability) + {4} (skill) = {5}", skill_name, difficulty, luck_component, natural_ability, person_skill, total);

	if (luck_component == 1) {
		return CRITICAL_FAIL;
	}
	else if (luck_component == 20) {
		gain_skill_from_success(settler_id, stats, skill_name, difficulty, rng);
		return CRITICAL_SUCCESS;
	}
	else if (total >= difficulty) {
		gain_skill_from_success(settler_id, stats, skill_name, difficulty, rng);
		return SUCCESS;
	}
	else {
		return FAIL;
	}
}

TTuple<skill_roll_result_t, int, int> skill_roll_ext(const int settler_id, game_stats_t &stats, RandomNumberGenerator  * rng, const FString skill_name, const int difficulty) {
	const int luck_component = rng->RollDice(1, 20);
	const int natural_ability = get_attribute_modifier_for_skill(stats, skill_name);
	const int8_t person_skill = get_skill_modifier(stats, skill_name);
	const int total = luck_component + natural_ability + person_skill;
	TTuple<skill_roll_result_t, int, int> result;
	result.Get<1>() = luck_component;
	result.Get<2>() = total - difficulty;

	//glog(log_target::GAME, log_severity::info, "{0} roll, difficulty {1}. 1d20 = {2}, + {3} (ability) + {4} (skill) = {5}", skill_name, difficulty, luck_component, natural_ability, person_skill, total);

	if (luck_component == 1) {
		result.Get<0>() = CRITICAL_FAIL;
	}
	else if (luck_component == 20) {
		gain_skill_from_success(settler_id, stats, skill_name, difficulty, rng);
		result.Get<0>() = CRITICAL_SUCCESS;
	}
	else if (total >= difficulty) {
		gain_skill_from_success(settler_id, stats, skill_name, difficulty, rng);
		result.Get<0>() = SUCCESS;
	}
	else {
		result.Get<0>() = FAIL;
	}
	return result;
}
