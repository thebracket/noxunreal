-- Scary ant-people who just want to eat the world

species_sentient['emmet'] = {
    name = "Emmet", male_name = "Drone", female_name = "Queen", group_name = "Emmet",
    description = "An ant-like creature of nightmares. It eats everything, and spreads blight.",
    stat_mods = { con = 2, str = 2, int = -2, wis = -2 },
    parts = humanoid_parts,
    ethics = { diet = "omnivore", blight = true, alignment="devour" },
    max_age = 20, infant_age = 1, child_age = 2,
    glyph = glyphs['ant'], ascii_glyph = glyphs['a'],
    worldgen_glyph = glyphs['ant'],
    composite_render=false, vox = voxelId("ant")
}

