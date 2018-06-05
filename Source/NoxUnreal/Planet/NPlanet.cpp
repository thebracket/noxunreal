// (c) 2016 - Present, Bracket Productions

#include "NPlanet.h"
#include "../Raws/NRaws.h"
#include "../Public/NoxGameInstance.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


namespace planet_astar {

	struct navigation_path_t
	{
		bool success = false;
		TArray<FVector2D> steps{};
		FVector2D destination{ 0,0 };
	};

	namespace impl {
		UNPlanet * planet;

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
			a_star_t(const FVector2D &start_pos, const FVector2D &end_pos) noexcept : start_(planet->idx(start_pos.X, start_pos.Y)), end_(planet->idx(end_pos.X, end_pos.Y)), end_loc_(end_pos)
			{
				end_x_ = end_pos.X;
				end_y_ = end_pos.Y;
				open_list_.Reserve(500);
				closed_list_.Reserve(300);
				parents_.Reserve(600);

				open_list_.Emplace(node_t{ start_, 0.0f, 0.0f, 0.0f });
			}

			float distance_to_end(const int &idx) const
			{
				const auto IDX = planet->idxmap(idx);
				auto x = IDX.X;
				auto y = IDX.Y;
				return distance2d_squared(x, y, end_x_, end_y_);
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
					auto IDX = planet->idxmap(*parent);
					auto x = IDX.X;
					auto y = IDX.Y;
					if (*parent != start_) result.steps.Insert(FVector2D{ x,y }, 0);
					current = *parent;
				}

				return result;
			}

			navigation_path_t search() noexcept
			{
				auto result = navigation_path_t();

				while (!open_list_.Num() == 0 && step_counter_ < MAX_ASTAR_STEPS)
				{
					++step_counter_;

					// Pop Q off of the list
					const auto q = open_list_[0];
					open_list_.RemoveAt(0);
					auto IDX = planet->idxmap(q.idx);
					auto x = IDX.X;
					auto y = IDX.Y;

					// Generate successors
					TArray<int> successors;
					if (x > 0 && planet->Landblocks[planet->idx(x - 1, y)].height > planet->water_height) successors.Emplace(planet->idx(x - 1, y));
					if (x < nfu::WORLD_WIDTH-1 && planet->Landblocks[planet->idx(x + 1, y)].height > planet->water_height) successors.Emplace(planet->idx(x + 1, y));
					if (y > 0 && planet->Landblocks[planet->idx(x, y-1)].height > planet->water_height) successors.Emplace(planet->idx(x, y-1));
					if (y < nfu::WORLD_HEIGHT-1 && planet->Landblocks[planet->idx(x, y+1)].height > planet->water_height) successors.Emplace(planet->idx(x, y+1));

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
			FVector2D end_loc_;
			int step_counter_ = 0;
		};

		static navigation_path_t a_star(const FVector2D &start, const FVector2D &end) noexcept
		{
			a_star_t searcher(start, end);
			return searcher.search();
		}

		static navigation_path_t find_path(const FVector2D &start, const FVector2D &end) noexcept
		{
			// Step 3 - Try A*
			auto result = a_star(start, end);
			return result;
		}

	}

	navigation_path_t find_path(const FVector2D &start, const FVector2D &end) noexcept
	{
		using namespace nfu;

		// Step 1 - check clipping on both ends; the path auto-fails fast if there is an out-of-bounds issue.
		if (start.X < 1 || start.X > WORLD_WIDTH - 1 || start.Y < 1 || start.Y > WORLD_HEIGHT - 1
			|| end.X < 1 || end.X > WORLD_WIDTH - 1 || end.Y < 1 || end.Y > WORLD_HEIGHT - 1)
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

		auto result = impl::find_path(start, end);
		if (result.success) return result;

		return result;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////


void UNPlanet::BuildPlanet(const int seed, const int water_divisor, const int plains_divisor, const int starting_settlers, const bool strict_beamdown) {
	planet_astar::impl::planet = this;

	// Set incoming parameters
	RngSeed = seed;	
	WaterDivisor = 3;
	PlaintsDivisor = 3;
	StartingSettlers = 10;
	StrictBeamdown = strict_beamdown;
	rng.ReSeed(seed);
	PerlinSeed = seed;

	// Clear the planet
	ZeroFillPlanet();

	// Noise-based world map
	FastNoise noise = PlanetNoiseMap();

	// Divide by height
	BaseTypeAllocation();

	// Find coastlines
	MarkCoastlines();

	// Rainfall
	Rainfall();

	// Biomes
	BuildBiomes(rng);

	// Rivers
	RunRivers(rng);

	// Initial Civs
	BuildCivilizations(rng);
}

void UNPlanet::ZeroFillPlanet() {
	Landblocks.Empty();
	Landblocks.AddUninitialized(nfu::WORLD_TILES_COUNT);
	for (auto & lb : Landblocks) {
		lb = FNPlanetBlock{};
	}
	MigrantCounter = 0;
	RemainingSettlers = 200;
	// TODO: Civilizations
}

constexpr int REGION_FRACTION_TO_CONSIDER = 64;
constexpr unsigned int octaves = 5;
constexpr float persistence = 0.25F;
constexpr float frequency = 4.0F;

inline uint8_t noise_to_planet_height(const float &n) {
	return static_cast<uint8_t>((n + 1.0F) * 150.0F);
}

inline float noise_x(const int world_x, const int region_x) {
	const auto big_x = static_cast<float>((world_x * nfu::WORLD_WIDTH) + region_x);
	return (big_x / (static_cast<float>(nfu::WORLD_WIDTH) * static_cast<float>(nfu::REGION_WIDTH))) * nfu::NOISE_SIZE;
}

inline float noise_y(const int world_y, const int region_y) {
	const auto big_y = static_cast<float>((world_y * nfu::WORLD_HEIGHT) + region_y);
	return (big_y / (static_cast<float>(nfu::WORLD_HEIGHT) * static_cast<float>(nfu::REGION_HEIGHT))) * nfu::NOISE_SIZE;
}

FastNoise UNPlanet::PlanetNoiseMap() {
	using namespace nfu;

	FastNoise noise(PerlinSeed);
	noise.SetSeed(PerlinSeed);
	noise.SetNoiseType(FastNoise::GradientFractal);
	noise.SetFractalType(FastNoise::FBM);
	noise.SetFractalOctaves(octaves);
	noise.SetFractalGain(persistence);
	noise.SetFractalLacunarity(frequency);
	noiseMap = noise;

	constexpr auto max_temperature = 56.7F;
	constexpr auto min_temperature = -55.2F;
	constexpr auto temperature_range = max_temperature - min_temperature;
	constexpr auto half_planet_height = WORLD_HEIGHT / 2.0F;

	// Determine height of the region block as an average of the containing tiles
	for (auto y = 0; y<WORLD_HEIGHT; ++y) {
		const auto distance_from_equator = std::abs((WORLD_HEIGHT / 2) - y);
		const auto temp_range_pct = 1.0F - (static_cast<float>(distance_from_equator) / half_planet_height);
		const auto base_temp_by_latitude = ((temp_range_pct * temperature_range) + min_temperature);
		//std::cout << y << "/" << distance_from_equator << "/" << temp_range_pct << "/" << base_temp_by_latitude << "\n";			
		for (auto x = 0; x<WORLD_WIDTH; ++x) {
			auto total_height = 0L;

			uint8_t max = 0;
			uint8_t min = 255;
			auto n_tiles = 0;
			for (auto y1 = 0; y1<REGION_HEIGHT / REGION_FRACTION_TO_CONSIDER; ++y1) {
				for (auto x1 = 0; x1<REGION_WIDTH / REGION_FRACTION_TO_CONSIDER; ++x1) {
					const auto nh = noise.GetNoise(noise_x(x, x1*REGION_FRACTION_TO_CONSIDER), noise_y(y, y1*REGION_FRACTION_TO_CONSIDER));
					//std::cout << nh << "\n";
					const auto n = noise_to_planet_height(nh);
					if (n < min) min = n;
					if (n > max) max = n;
					total_height += n;
					++n_tiles;
				}
			}
			Landblocks[idx(x, y)].height = static_cast<uint8_t>(total_height / n_tiles);
			Landblocks[idx(x, y)].type = 0;
			Landblocks[idx(x, y)].variance = max - min;
			const auto altitude_deduction = std::abs(Landblocks[idx(x, y)].height - water_height) / 20.0F;
			Landblocks[idx(x, y)].temperature_c = base_temp_by_latitude;
			if (Landblocks[idx(x, y)].temperature_c < -55) Landblocks[idx(x, y)].temperature_c = -55;
			if (Landblocks[idx(x, y)].temperature_c > 55) Landblocks[idx(x, y)].temperature_c = 55;

		}
	}

	return noise;
}

void UNPlanet::BaseTypeAllocation() {
	using namespace nfu;

	int candidate = 0;
	const int remaining_divisor = 10 - (WaterDivisor + PlaintsDivisor);
	const int n_cells = WORLD_HEIGHT * WORLD_WIDTH;
	const int n_cells_water = (n_cells / WaterDivisor);
	const int n_cells_plains = (n_cells / PlaintsDivisor) + n_cells_water;
	const int n_cells_hills = (n_cells / remaining_divisor) + n_cells_plains;

	water_height = DetermineProportion(candidate, n_cells_water);
	plains_height = DetermineProportion(candidate, n_cells_plains);
	hills_height = DetermineProportion(candidate, n_cells_hills);

	//std::cout << "Heights (water/plains/hills): " << +planet.water_height << " / " << +planet.plains_height << " / " << +planet.hills_height << "\n";

	for (auto i = 0; i<Landblocks.Num(); ++i) {
		auto &block = Landblocks[i];
		if (block.height <= water_height) {
			block.type = BlockType::WATER;
			block.rainfall = 10;
			if (block.height + block.variance / 2 > water_height) block.type = BlockType::SALT_MARSH;
		}
		else if (block.height <= plains_height) {
			block.type = BlockType::PLAINS;
			block.rainfall = 10;
			if (block.height - block.variance / 2 > water_height) block.type = BlockType::MARSH;
		}
		else if (block.height <= hills_height) {
			block.type = BlockType::HILLS;
			block.rainfall = 20;
			if (block.variance < 2) {
				block.type = BlockType::HIGHLANDS;
				block.rainfall = 10;
			}
		}
		else {
			block.type = BlockType::MOUNTAINS;
			block.rainfall = 30;
			if (block.variance < 3) {
				block.type = BlockType::PLATEAU;
				block.rainfall = 10;
			}
		}
	}
}

int32 UNPlanet::DetermineProportion(int32 &candidate, int32 target) {
	auto count = 0;
	while (count < target) {
		for (const auto &lb : Landblocks) {
			if (lb.height <= candidate) ++count;
		}

		if (count >= target) {
			return candidate;
		}
		else {
			++candidate;
			count = 0;
		}
	}
	return count;
}

void UNPlanet::MarkCoastlines() {
	using namespace nfu;

	for (int y = 1; y<WORLD_HEIGHT - 1; ++y) {
		for (int x = 1; x<WORLD_WIDTH - 1; ++x) {
			if (Landblocks[idx(x, y)].type > BlockType::WATER) {
				if (
					Landblocks[idx(x - 1, y - 1)].type == BlockType::WATER ||
					Landblocks[idx(x, y - 1)].type == BlockType::WATER ||
					Landblocks[idx(x + 1, y - 1)].type == BlockType::WATER ||
					Landblocks[idx(x - 1, y)].type == BlockType::WATER ||
					Landblocks[idx(x + 1, y)].type == BlockType::WATER ||
					Landblocks[idx(x - 1, y + 1)].type == BlockType::WATER ||
					Landblocks[idx(x, y + 1)].type == BlockType::WATER ||
					Landblocks[idx(x + 1, y + 1)].type == BlockType::WATER
				) {
					Landblocks[idx(x, y)].type = BlockType::COASTAL;
					Landblocks[idx(x, y)].rainfall = 20;
				}
			}
		}
	}
}

void UNPlanet::Rainfall() {
	using namespace nfu;

	for (int y = 0; y<WORLD_HEIGHT; ++y) {

		int rain_amount = 10;
		for (int x = 0; x<WORLD_WIDTH; ++x) {
			if (Landblocks[idx(x, y)].type == BlockType::MOUNTAINS) {
				rain_amount -= 20;
			}
			else if (Landblocks[idx(x, y)].type == BlockType::HILLS) {
				rain_amount -= 10;
			}
			else if (Landblocks[idx(x, y)].type == BlockType::COASTAL) {
				rain_amount -= 5;
			}
			else {
				rain_amount += 1;
			}
			if (rain_amount < 0) rain_amount = 0;
			if (rain_amount > 20) rain_amount = 20;

			Landblocks[idx(x, y)].rainfall += rain_amount;
			if (Landblocks[idx(x, y)].rainfall < 0) Landblocks[idx(x, y)].rainfall = 0;
			if (Landblocks[idx(x, y)].rainfall > 99) Landblocks[idx(x, y)].rainfall = 99;
		}

	}
}

void UNPlanet::BuildBiomes(RandomNumberGenerator &rng) {
	using namespace nfu;

	const auto n_biomes = 768;
	Biomes.Empty();
	Biomes.AddDefaulted(n_biomes);
	//for (int i = 0; i < n_biomes; ++i)
	//	Biomes.Emplace(FNBiome{});

	/*FastNoise biome_noise(PerlinSeed);
	biome_noise.SetNoiseType(FastNoise::Cellular);

	for (int y = 0; y < nfu::WORLD_HEIGHT; ++y) {
		for (int x = 0; x < nfu::WORLD_WIDTH; ++x) {
			const int block_idx = idx(x, y);
			const int z = Landblocks[block_idx].height;
			const auto CellNoise = biome_noise.GetCellular(x*8.0f, y*8.0f, z*8.0f);
			const auto NoiseRamp = (CellNoise + 2.0f) / 4.0f;
			int32 BiomeIdx = NoiseRamp * n_biomes;
			Landblocks[block_idx].biome_idx = BiomeIdx;
		}
	}*/

	// Randomly place biome centers
	TArray<TPair<int32, int32>> centroids;
	for (auto i = 0; i<n_biomes; ++i) {
		bool ok = false;
		while (!ok) {
			int x = rng.RollDice(1, WORLD_WIDTH-1);
			int y = rng.RollDice(1, WORLD_HEIGHT-1);
			bool dupe = false;
			for (const auto &c : centroids) {
				if (c.Key == x && c.Value == y) dupe = true;
			}

			if (!dupe) {
				centroids.Emplace(TPair<int32, int32>(x, y));
				ok = true;
			}
		}
	}

	// Assign each cell based on proximity
	for (auto y = 0; y<WORLD_HEIGHT; ++y) {
		for (auto x = 0; x<WORLD_WIDTH; ++x) {
			auto distance = 2147483647;
			auto closest_index = -1;
			auto mytype = Landblocks[idx(x, y)].type;

			for (auto i = 0; i<n_biomes; ++i) {
				const auto biome_x = centroids[i].Key;
				const auto biome_y = centroids[i].Value;
				const auto dx = std::abs(biome_x - x);
				const auto dy = std::abs(biome_y - y);
				const auto biome_distance = biome_x == x && biome_y == y ? 0 : (dx*dx) + (dy*dy);
				if (biome_distance == 0 || biome_distance < distance && Landblocks[idx(biome_x,biome_y)].type == mytype) {
					distance = biome_distance;
					closest_index = i;
				}
			}

			if (closest_index > -1) {
				Landblocks[idx(x, y)].biome_idx = closest_index;
				Biomes[closest_index].mean_temperature = Landblocks[idx(x, y)].temperature_c;
			}
			else {
				for (auto i = 0; i<n_biomes; ++i) {
					const auto biome_x = centroids[i].Key;
					const auto biome_y = centroids[i].Value;
					const auto dx = std::abs(biome_x - x);
					const auto dy = std::abs(biome_y - y);
					const auto biome_distance = biome_x == x && biome_y == y ? 0 : (dx*dx) + (dy*dy);
					if (biome_distance == 0 || biome_distance < distance) {
						distance = biome_distance;
						closest_index = i;
					}
				}
				if (closest_index == -1) closest_index = 1;
				Landblocks[idx(x, y)].biome_idx = closest_index;
				Biomes[closest_index].mean_temperature = Landblocks[idx(x, y)].temperature_c;
			}
		}
	}

	// Process each biome and determine important facts defining it
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto raws = game->GetRaws();

	std::size_t count = 0;
	std::size_t no_match = 0;
	for (auto &biome : Biomes) {
		auto membership_count = BiomeMembership(count);
		if (!membership_count.Num() == 0) {
			auto possible_types = FindPossibleBiomes(membership_count, biome);
			if (possible_types.Num() > 0) {

				
				/*auto max_roll = 0.0;
				for (const auto &possible : possible_types) {
					max_roll += possible.Key;
				}
				auto dice_roll = rng.RollDice(1, static_cast<int>(max_roll));
				for (const auto &possible : possible_types) {
					dice_roll -= static_cast<int>(possible.Key);
					if (dice_roll < 0) {
						biome.type = possible.Value;
						break;
					}
				}
				if (biome.type == -1) biome.type = possible_types[possible_types.Num() - 1].Value;*/
				
				const int numAvailable = possible_types.Num();
				const int roll = rng.RollDice(1, numAvailable) - 1;
				biome.type = possible_types[roll].Value;
				biome.name = NameBiome(rng, biome);
				biome.BiomeTypeName = raws->get_biome_def(biome.type)->name;
			}
			else {
				++no_match;
			}
		}
	}
}

TMap<uint8, double> UNPlanet::BiomeMembership(const int32_t &bidx) {
	using namespace nfu;

	TMap<uint8, double>  percents;
	TMap<uint8, long>  counts;
	auto n_cells = 0L;
	auto total_rainfall = 0L;
	auto total_height = 0L;
	auto total_variance = 0L;
	auto total_x = 0L;
	auto total_y = 0L;

	for (auto y = 0; y<WORLD_HEIGHT-1; ++y) {
		for (auto x = 0; x<WORLD_WIDTH-1; ++x) {
			const auto block_idx = idx(x, y);

			if (Landblocks[block_idx].biome_idx == bidx) {
				// Increment total counters
				++n_cells;
				total_rainfall += Landblocks[block_idx].rainfall;
				total_height += Landblocks[block_idx].height;
				total_variance += Landblocks[block_idx].variance;
				total_x += x;
				total_y += y;

				// Increment count by cell type
				auto finder = counts.Find(Landblocks[block_idx].type);
				if (finder == nullptr) {
					counts.Add(Landblocks[block_idx].type,  1L);
				}
				else {
					++counts[Landblocks[block_idx].type];
				}
			}
		}
	}

	// Calculate the averages
	if (n_cells == 0) {
		//std::unordered_map<uint8_t, double>();
		return percents;
	}

	const auto counter = static_cast<double>(n_cells);
	Biomes[bidx].mean_altitude = static_cast<uint8_t>(static_cast<double>(total_height) / counter);
	Biomes[bidx].mean_rainfall = static_cast<uint8_t>(static_cast<double>(total_rainfall) / counter);
	Biomes[bidx].mean_variance = static_cast<uint8_t>(static_cast<double>(total_variance) / counter);
	Biomes[bidx].center_x = total_x / n_cells;
	Biomes[bidx].center_y = total_y / n_cells;

	const auto distance_from_pole = static_cast<int>(FMath::Min(distance2d(Biomes[bidx].center_x, Biomes[bidx].center_y, WORLD_WIDTH / 2, 0), distance2d(Biomes[bidx].center_x, Biomes[bidx].center_y, WORLD_WIDTH / 2, WORLD_HEIGHT)));
	const auto distance_from_center = static_cast<int>(distance2d(Biomes[bidx].center_x, Biomes[bidx].center_y, WORLD_WIDTH / 2, WORLD_HEIGHT / 2));

	if (distance_from_pole > 200) {
		Biomes[bidx].warp_mutation = 0;
	}
	else {
		Biomes[bidx].warp_mutation = (200 - distance_from_pole) / 2;
	}
	Biomes[bidx].savagery = FMath::Min(100, distance_from_center);

	for (auto i = 0; i <= BlockType::MAX_BLOCK_TYPE; ++i) {
		const auto finder = counts.Find(i);
		if (finder == nullptr) {
			percents.Add(i, 1.0);
		}
		else {
			const auto pct = static_cast<double>(*finder) / counter;
			percents.Add(i, pct);
		}

	}

	return percents;
}

TArray<TPair<double, size_t>> UNPlanet::FindPossibleBiomes(TMap<uint8, double> &percents, const FNBiome &biome) {
	TArray<TPair<double, size_t>> result;

	std::size_t idx = 0;
	
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	NRaws * raws = game->GetRaws();
	for (idx = 0; idx < raws->biome_defs.Num(); ++idx) {
		const auto bt = &raws->biome_defs[idx];
		/*for (const auto &occur : bt->occurs) {
			auto finder = percents.Find(occur);
			if (finder != nullptr && *finder > 0) {
				result.Emplace(TPair<double, size_t>(25.0, idx));
			}
		}*/
		
		if (biome.mean_temperature >= bt->min_temp && biome.mean_temperature <= bt->max_temp) {

			// It's possible, so check to see if tile types are available
			for (const auto &occur : bt->occurs) {
				auto finder = percents.Find(occur);
				if (finder != nullptr && *finder > 0) {
					result.Emplace(TPair<double, size_t>(*finder * 100.0, idx));
				}
				else {
					//result.Emplace(TPair<double, size_t>(25.0, idx));
				}
			}
		}
	};

	return result;
}

FString UNPlanet::NameBiome(RandomNumberGenerator &rng, FNBiome &biome) {
	using namespace nfu;

	FString name = "Nameless";

	TArray<FString> adjectives;

	// Location-based adjective
	if (std::abs(biome.center_x - WORLD_WIDTH / 2) < WORLD_WIDTH / 10 && std::abs(biome.center_y - WORLD_HEIGHT / 2) < WORLD_HEIGHT / 10) {
		adjectives.Emplace("Central");
	}
	else {
		if (biome.center_x < WORLD_WIDTH / 2) adjectives.Emplace("Western");
		if (biome.center_x > WORLD_WIDTH / 2) adjectives.Emplace("Eastern");
		if (biome.center_y < WORLD_HEIGHT / 2) adjectives.Emplace("Northern");
		if (biome.center_y > WORLD_WIDTH / 2) adjectives.Emplace("Southern");
	}

	// Water-based adjectives
	if (biome.mean_rainfall < 10) {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Dry");
			break;
		case 2:
			adjectives.Emplace("Arid");
			break;
		case 3:
			adjectives.Emplace("Parched");
			break;
		case 4:
			adjectives.Emplace("Cracked");
			break;
		}
	}
	else if (biome.mean_rainfall < 30) {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Dusty");
			break;
		case 2:
			adjectives.Emplace("Withered");
			break;
		case 3:
			adjectives.Emplace("Droughty");
			break;
		case 4:
			adjectives.Emplace("Dehydrated");
			break;
		}
	}
	else if (biome.mean_rainfall < 50) {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Pleasant");
			break;
		case 2:
			adjectives.Emplace("Kind");
			break;
		case 3:
			adjectives.Emplace("Gentle");
			break;
		case 4:
			adjectives.Emplace("Timid");
			break;
		}
	}
	else if (biome.mean_rainfall < 70) {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Damp");
			break;
		case 2:
			adjectives.Emplace("Dank");
			break;
		case 3:
			adjectives.Emplace("Moist");
			break;
		case 4:
			adjectives.Emplace("Fresh");
			break;
		}
	}
	else {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Wet");
			break;
		case 2:
			adjectives.Emplace("Soggy");
			break;
		case 3:
			adjectives.Emplace("Soaked");
			break;
		case 4:
			adjectives.Emplace("Drenched");
			break;
		}
	}

	// Temperature based adjectives
	if (biome.mean_temperature < 10) {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Frozen");
			break;
		case 2:
			adjectives.Emplace("Cold");
			break;
		case 3:
			adjectives.Emplace("Icy");
			break;
		case 4:
			adjectives.Emplace("Biting");
			break;
		}
	}
	else if (biome.mean_temperature < 20) {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Chilly");
			break;
		case 2:
			adjectives.Emplace("Frigid");
			break;
		case 3:
			adjectives.Emplace("Chilling");
			break;
		case 4:
			adjectives.Emplace("Shivering");
			break;
		}
	}
	else if (biome.mean_temperature < 30) {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Pleasant");
			break;
		case 2:
			adjectives.Emplace("Nice");
			break;
		case 3:
			adjectives.Emplace("Temperate");
			break;
		case 4:
			adjectives.Emplace("Comfortable");
			break;
		}
	}
	else if (biome.mean_temperature < 40) {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Warm");
			break;
		case 2:
			adjectives.Emplace("Toasty");
			break;
		case 3:
			adjectives.Emplace("Cozy");
			break;
		case 4:
			adjectives.Emplace("Snug");
			break;
		}
	}
	else {
		switch (rng.RollDice(1, 4)) {
		case 1:
			adjectives.Emplace("Hot");
			break;
		case 2:
			adjectives.Emplace("Scorching");
			break;
		case 3:
			adjectives.Emplace("Burning");
			break;
		case 4:
			adjectives.Emplace("Fuming");
			break;
		}
	}

	FString noun;
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	NRaws * raws = game->GetRaws();
	const auto bt = raws->get_biome_def(biome.type);
	noun = bt->nouns[rng.RollDice(1, bt->nouns.Num()) - 1];

	name = noun;
	if (adjectives.Num() > 0) {
		const auto adj1 = rng.RollDice(1, adjectives.Num()) - 1;
		name = adjectives[adj1] + " " + noun;
		if (adjectives.Num() > 1 && rng.RollDice(1, 6)>2) {
			auto adj2 = rng.RollDice(1, adjectives.Num()) - 1;
			while (adj1 == adj2) {
				adj2 = rng.RollDice(1, adjectives.Num()) - 1;
			}
			name = adjectives[adj2] + " " + name;
		}
	}

	//std::cout << "Biome: " << name << "\n";
	return name;
}

void UNPlanet::RunRivers(RandomNumberGenerator &rng) {
	using namespace nfu;

	const int n_rivers = WORLD_WIDTH;
	TSet<int> used_starts;

	for (int i = 0; i<n_rivers; ++i) {
		FNRiver river;

		bool start_ok = false;
		while (!start_ok) {
			river.start_x = rng.RollDice(1, WORLD_WIDTH) - 1;
			river.start_y = rng.RollDice(1, WORLD_HEIGHT) - 1;
			const int pidx = idx(river.start_x, river.start_y);
			if ((Landblocks[pidx].type == BlockType::MOUNTAINS || Landblocks[pidx].type == BlockType::HILLS) && used_starts.Find(pidx) == nullptr) start_ok = true;
		}
		used_starts.Add(idx(river.start_x, river.start_y));

		TSet<int> used_steps;
		bool done = false;
		int x = river.start_x;
		int y = river.start_y;
		while (!done) {
			TMap<uint8_t, TPair<int, int>> candidates;
			if (x > 0 && used_steps.Find(idx(x - 1, y)) == nullptr) candidates.Add( Landblocks[idx(x - 1, y)].height, TPair<int, int>{ x - 1, y } );
			if (x < WORLD_WIDTH - 1 && used_steps.Find(idx(x + 1, y)) == nullptr) candidates.Add( Landblocks[idx(x + 1, y)].height, TPair<int, int>( x + 1, y) );
			if (y > 0 && used_steps.Find(idx(x, y - 1)) == nullptr) candidates.Add( Landblocks[idx(x, y - 1)].height, TPair<int, int>( x, y - 1 ) );
			if (y < WORLD_HEIGHT - 1 && used_steps.Find(idx(x, y + 1)) == nullptr) candidates.Add( Landblocks[idx(x, y + 1)].height, TPair<int, int>( x, y + 1 ) );
			FNRiverStep step;
			if (candidates.Num() == 0) {
				done = true;
			}
			else {

				for (const auto &test : Rivers) {
					if (!done) {
						for (const auto &step : test.steps) {
							if (x == step.x && y == step.y) { done = true; break; }
						}
					}
				}

				if (!done) {
					auto it = candidates.CreateIterator();
					step.x = it->Value.Key;
					step.y = it->Value.Value;

					if (Landblocks[idx(x, y)].type == BlockType::WATER || x == 0 || x == WORLD_WIDTH || y == 0 || y == WORLD_HEIGHT) {
						done = true;
					}
					else {
						river.steps.Emplace(step);
						used_steps.Add(idx(step.x, step.y));
						x = step.x;
						y = step.y;
					}
				}
			}
		}

		Rivers.Emplace(river);

		int rx = river.start_x;
		int ry = river.start_y;
		int block = idx(rx, ry);
		setbit(RiverMaskBits::START, Landblocks[block].RiverMask);

		int step = 0;
		for (const auto &r : river.steps) {
			rx = r.x;
			ry = r.y;
			block = idx(rx, ry);

			if (step < river.steps.Num() - 1) {
				const int nextX = river.steps[step + 1].x;
				const int nextY = river.steps[step + 1].y;

				if (nextX > rx) {
					setbit(RiverMaskBits::EAST, Landblocks[block].RiverMask);
				}
				else if (nextX < rx) {
					setbit(RiverMaskBits::WEST, Landblocks[block].RiverMask);
				}
				else if (nextY < ry) {
					setbit(RiverMaskBits::NORTH, Landblocks[block].RiverMask);
				}
				else if (nextY > ry) {
					setbit(RiverMaskBits::SOUTH, Landblocks[block].RiverMask);
				}
				else {
					setbit(RiverMaskBits::START, Landblocks[block].RiverMask);
				}
			}

			if (step > 0) {
				const int nextX = river.steps[step - 1].x;
				const int nextY = river.steps[step - 1].y;

				if (nextX > rx) {
					setbit(RiverMaskBits::EAST, Landblocks[block].RiverMask);
				}
				else if (nextX < rx) {
					setbit(RiverMaskBits::WEST, Landblocks[block].RiverMask);
				}
				else if (nextY < ry) {
					setbit(RiverMaskBits::NORTH, Landblocks[block].RiverMask);
				}
				else if (nextY > ry) {
					setbit(RiverMaskBits::SOUTH, Landblocks[block].RiverMask);
				}
				else {
					setbit(RiverMaskBits::START, Landblocks[block].RiverMask);
				}
			}

			++step;
		}
	}
}

FNPlanetBlock UNPlanet::GetWorldBlock(const int x, const int y) {
	const int pidx = idx(x, y);
	return Landblocks[pidx];
}

FNBiome UNPlanet::GetWorldBiome(const int x, const int y) {
	const int pidx = idx(x, y);
	return Biomes[Landblocks[pidx].biome_idx];
}

void UNPlanet::BuildCivilizations(RandomNumberGenerator &rng) {
	// Build initial civilizations
	PlaceStartingCivs(rng);
}

void UNPlanet::PlaceStartingCivs(RandomNumberGenerator &rng) {
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	NRaws * raws = game->GetRaws();

	for (int i = 0; i < n_civs; ++i) {
		const int roll = rng.RollDice(1, raws->species_defs.Num()) - 1;
		FNCivilization civ;

		int bidx = 0;
		for (const auto &species : raws->species_defs) {
			if (bidx == roll) civ.SpeciesTag = species.Value.tag;
			++bidx;
		}

		bool ok = false;
		int x = 0;
		int y = 0;
		bidx = idx(x, y);
		while (!ok) {
			x = rng.RollDice(1, nfu::WORLD_WIDTH - 1);
			y = rng.RollDice(1, nfu::WORLD_HEIGHT - 1);
			bidx = idx(x, y);
			bool far_enough = true;

			for (const auto &other : civilizations) {
				const float d = distance2d_squared(other.StartX, other.StartY, x, y);
				if (d < 10.0f) far_enough = false;
			}

			if (far_enough && Landblocks[bidx].height > water_height && !Biomes[Landblocks[bidx].biome_idx].BiomeTypeName.Contains("Ocean") && !Biomes[Landblocks[bidx].biome_idx].BiomeTypeName.Contains("Desert")) ok = true;
		}
		setbit(FeatureMaskBits::HUTS, Landblocks[bidx].Features);
		setbit(FeatureMaskBits::ROAD_N, Landblocks[bidx].Features);
		setbit(FeatureMaskBits::ROAD_S, Landblocks[bidx].Features);
		setbit(FeatureMaskBits::ROAD_E, Landblocks[bidx].Features);
		setbit(FeatureMaskBits::ROAD_W, Landblocks[bidx].Features);
		Landblocks[bidx].OwnerCiv = i;

		civ.StartX = x;
		civ.StartY = y;
		civ.Color = FLinearColor(rng.RollDice(1, 255)/255.0f, rng.RollDice(1, 255)/255.0f, rng.RollDice(1, 255)/255.0f, 1.0f);
		const auto govroll = rng.RollDice(1, raws->government_defs.Num())-1;
		civ.GovernmentIndex = govroll;
		civilizations.Emplace(civ);

		FSettlement settlement;
		settlement.x = x;
		settlement.y = y;
		settlement.Civilization = i;
		settlement.size = 1;
		settlements.Add(bidx, settlement);		
	}
}

void UNPlanet::RunWorldgenYear() {
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	NRaws * raws = game->GetRaws();

	TArray<int> DeadSettlements;

	for (auto &settlement : settlements) {
		const int n_available_improvements = settlement.Value.size * 2;
		const int radius = settlement.Value.size;

		// Claim radius tiles and mark resource availability
		TArray<TPair<int, int>> FarmableTiles;
		TSet<int> AdjacentCivs;

		for (int cx = -radius; cx < radius+1; ++cx) {
			for (int cy = -radius; cy < radius+1; ++cy) {
				int sx = settlement.Value.x + cx;
				int sy = settlement.Value.y + cy;
				if (sx > 0 && sx < nfu::WORLD_WIDTH - 1 && sy > 0 && sy < nfu::WORLD_HEIGHT - 1) {
					int currentOwner = Landblocks[idx(sx, sy)].OwnerCiv;
					if (currentOwner == -1 || currentOwner == settlement.Value.Civilization) {
						Landblocks[idx(sx, sy)].OwnerCiv = settlement.Value.Civilization;
						Landblocks[idx(sx, sy)].OwnerSettlement = settlement.Key;
						if (Landblocks[idx(sx, sy)].height > water_height && (cx != 0 && cy != 0)) {
							FarmableTiles.Emplace(TPair<int, int>(idx(sx, sy), raws->get_biome_def(Biomes[Landblocks[idx(sx, sy)].biome_idx].type)->food_bonus));
						}
					}
				}
			}
		}

		// Determine adjacent civilizations
		TArray<int> AdjacentCities;
		const int radius2 = radius * 2;
		for (int cx = -radius2; cx < radius2 + 1; ++cx) {
			for (int cy = -radius2; cy < radius2 + 1; ++cy) {
				int sx = settlement.Value.x + cx;
				int sy = settlement.Value.y + cy;
				if (sx > 0 && sx < nfu::WORLD_WIDTH - 1 && sy > 0 && sy < nfu::WORLD_HEIGHT - 1) {
					int OtherCiv = Landblocks[idx(sx, sy)].OwnerCiv;
					if (OtherCiv != -1 && OtherCiv != settlement.Value.Civilization && !AdjacentCivs.Contains(OtherCiv)) {
						AdjacentCivs.Add(OtherCiv);
						AdjacentCities.Add(Landblocks[idx(sx, sy)].OwnerSettlement);
					}
				}
			}
		}

		// If we can use additional tiles, do so.
		if (settlement.Value.DevelopedTiles.Num() < n_available_improvements) {
			int to_build = FMath::Min(n_available_improvements - settlement.Value.DevelopedTiles.Num(), FarmableTiles.Num());
			FarmableTiles.Sort([](auto &a, auto &b) { return a.Value > b.Value; });
			for (int i = 0; i < to_build; ++i) {
				auto biomeType = raws->get_biome_def(Biomes[Landblocks[FarmableTiles[i].Key].biome_idx].type);

				if ((biomeType->deciduous_tree_chance > 0 || biomeType->evergreen_tree_chance > 0) && rng.RollDice(1,6)<3) {
					settlement.Value.DevelopedTiles.Emplace(FarmableTiles[i].Key);
					setbit(FeatureMaskBits::LUMBER, Landblocks[FarmableTiles[i].Key].Features);
				}
				else if (rng.RollDice(1, 6) < 3) {
					settlement.Value.DevelopedTiles.Emplace(FarmableTiles[i].Key);
					setbit(FeatureMaskBits::MINE, Landblocks[FarmableTiles[i].Key].Features);
				}
				else {
					settlement.Value.DevelopedTiles.Emplace(FarmableTiles[i].Key);
					setbit(FeatureMaskBits::FARM, Landblocks[FarmableTiles[i].Key].Features);
				}
			}
		}

		// Expand the food supply
		int addedFood = 0;
		int addedLumber = 0;
		int addedMinerals = 0;
		for (const auto &i : settlement.Value.DevelopedTiles) {
			addedFood += (1 + raws->get_biome_def(Biomes[Landblocks[i].biome_idx].type)->food_bonus);
			if (testbit(FeatureMaskBits::FARM, Landblocks[i].Features)) addedFood += 1;

			if (testbit(FeatureMaskBits::LUMBER, Landblocks[i].Features)) addedLumber += 1;
			if (testbit(FeatureMaskBits::MINE, Landblocks[i].Features)) addedMinerals += 1;
		}
		settlement.Value.FoodStock += addedFood;
		settlement.Value.LumberStock += addedLumber;
		settlement.Value.MineralStock += addedMinerals;

		if (settlement.Value.FoodStock > settlement.Value.size * 10) {
			settlement.Value.FoodStock = 1;
			++settlement.Value.size;
		}		

		// Income from trade
		for (const auto &tr : settlement.Value.TradeRoutes) {
			if (settlements.Contains(tr)) {
				auto OtherCiv = settlements[tr].Civilization;
				auto relationship = civilizations[settlement.Value.Civilization].Relations.Find(OtherCiv);
				if (relationship != nullptr && *relationship > -1) {
					// Only profit if not at war
					settlement.Value.CashStock += settlement.Value.TradeRoutes.Num() * 100;
					*relationship++; // Trade makes friends
					if (settlement.Value.FoodStock < 1 && settlement.Value.CashStock > 0) {
						settlement.Value.CashStock -= 1; // Buy food
						settlement.Value.FoodStock = 1;
					}
				}
				else {
					if (relationship == nullptr) {
						civilizations[settlement.Value.Civilization].Relations.Add(OtherCiv, DetermineInitialRelationship(settlement.Value.Civilization, OtherCiv));
						relationship = civilizations[settlement.Value.Civilization].Relations.Find(OtherCiv);
					}

					// Black market
					settlement.Value.CashStock += settlement.Value.TradeRoutes.Num() * raws->government_defs[civilizations[settlement.Value.Civilization].GovernmentIndex].black_market;
					*relationship++; // Trade makes friends

				}

				// Learn about other civilizations, and start a relationship by proxy.
				for (const auto &DistantCiv : civilizations[OtherCiv].Relations) {
					if (!civilizations[settlement.Value.Civilization].Relations.Contains(DistantCiv.Key)) {
						int base = DetermineInitialRelationship(settlement.Value.Civilization, DistantCiv.Key) / 2;
						base += DistantCiv.Value / 2;
						civilizations[settlement.Value.Civilization].Relations.Add(DistantCiv.Key, base);
					}
				}
			}
		}

		// Fail if insufficient food
		if (settlement.Value.FoodStock < 0) {
			--settlement.Value.size;
			if (settlement.Value.size == 0) {
				// The settlement died!
				DeadSettlements.Emplace(settlement.Key);
			}
		}

		// Adjacency
		for (const auto &OtherCiv : AdjacentCivs) {
			if (!civilizations[settlement.Value.Civilization].Relations.Contains(OtherCiv)) {
				// Determine initial relations
				int InitialRelationship = DetermineInitialRelationship(settlement.Value.Civilization, OtherCiv);
				civilizations[settlement.Value.Civilization].Relations.Add(OtherCiv, InitialRelationship);
				civilizations[OtherCiv].Relations.Add(settlement.Value.Civilization, InitialRelationship);
			}
		}
		for (const auto &OtherCity : AdjacentCities) {
			if (!settlement.Value.TradeRoutes.Contains(OtherCity) && settlements.Contains(OtherCity)) {
				auto path = planet_astar::find_path(FVector2D(settlement.Value.x, settlement.Value.y), FVector2D(settlements[OtherCity].x, settlements[OtherCity].y));

				if (path.success) {
					settlement.Value.TradeRoutes.Add(OtherCity);
					settlements[OtherCity].TradeRoutes.Add(settlement.Key);

					for (int step = 0; step < path.steps.Num() - 1; ++step) {
						int MyX = path.steps[step].X;
						int MyY = path.steps[step].Y;
						int NextX = path.steps[step + 1].X;
						int NextY = path.steps[step + 1].Y;

						if (MyX > NextX) {
							// We're going west
							setbit(FeatureMaskBits::ROAD_W, Landblocks[idx(MyX, MyY)].Features);
							setbit(FeatureMaskBits::ROAD_E, Landblocks[idx(NextX, NextY)].Features);
						}
						else if (MyX < NextX) {
							// We're going east
							setbit(FeatureMaskBits::ROAD_E, Landblocks[idx(MyX, MyY)].Features);
							setbit(FeatureMaskBits::ROAD_W, Landblocks[idx(NextX, NextY)].Features);
						}
						else if (MyY < NextY) {
							// We're going south
							setbit(FeatureMaskBits::ROAD_S, Landblocks[idx(MyX, MyY)].Features);
							setbit(FeatureMaskBits::ROAD_N, Landblocks[idx(NextX, NextY)].Features);
						}
						else if (MyY > NextY) {
							// We're going north
							setbit(FeatureMaskBits::ROAD_N, Landblocks[idx(MyX, MyY)].Features);
							setbit(FeatureMaskBits::ROAD_S, Landblocks[idx(NextX, NextY)].Features);
						}
					}
				}
			}
		}

		// Tithe to the civ
		const float TaxRate = raws->government_defs[civilizations[settlement.Value.Civilization].GovernmentIndex].tax;
		if (settlement.Value.FoodStock > 0) civilizations[settlement.Value.Civilization].FoodWealth += ((float)settlement.Value.FoodStock * TaxRate);
		if (settlement.Value.LumberStock > 0) civilizations[settlement.Value.Civilization].LumberWealth += ((float)settlement.Value.LumberStock * TaxRate);
		if (settlement.Value.MineralStock > 0) civilizations[settlement.Value.Civilization].MineralWealth += ((float)settlement.Value.MineralStock * TaxRate);
		civilizations[settlement.Value.Civilization].CashWealth += ((float)settlement.Value.CashStock * TaxRate);
	}

	// Dead settlement removal
	for (const auto &id : DeadSettlements) {
		const int radius = settlements[id].size;
		for (int cx = -radius; cx < radius + 1; ++cx) {
			for (int cy = -radius; cy < radius + 1; ++cy) {
				int sx = settlements[id].x + cx;
				int sy = settlements[id].y + cy;
				if (sx > 0 && sx < nfu::WORLD_WIDTH - 1 && sy > 0 && sy < nfu::WORLD_HEIGHT - 1) {
					int currentOwner = Landblocks[idx(sx, sy)].OwnerCiv;
					if (currentOwner == settlements[id].Civilization) {
						Landblocks[idx(sx, sy)].OwnerCiv = -1;
						resetbit(FeatureMaskBits::HUTS, Landblocks[idx(sx, sy)].Features);
						resetbit(FeatureMaskBits::FARM, Landblocks[idx(sx, sy)].Features);
					}
				}
			}
		}
		settlements.Remove(id);
		// TODO: Civilization extinction
	}

	++Year;
}

int UNPlanet::DetermineInitialRelationship(const int &civ1, const int &civ2) {
	UNoxGameInstance * game = Cast<UNoxGameInstance>(UGameplayStatics::GetGameInstance(this));
	NRaws * raws = game->GetRaws();

	int base = rng.RollDice(1, 10) - 5;

	if (civilizations[civ1].SpeciesTag == civilizations[civ2].SpeciesTag) {
		base += 3; // Bonus for liking same species
	}
	else {
		base -= 3; // Penalty for other species
	}

	// Modify for government types
	base += raws->government_defs[civilizations[civ1].GovernmentIndex].base_relationship_mod;
	base += raws->government_defs[civilizations[civ2].GovernmentIndex].base_relationship_mod;

	return base;
}