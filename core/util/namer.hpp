//
// Created by akicatt on 06.08.2026.
//

#ifndef MYDAK_BACKEND_NAMER_HPP
#define MYDAK_BACKEND_NAMER_HPP
#include <array>
#include <string_view>
#include <vector>
#include <map>

#include "proto.hpp"

namespace mydak::namer {
    namespace detail {
        constexpr std::array<std::string_view, 256> adjectives = {
            "silent", "brave", "swift", "hidden", "golden", "fierce", "quiet", "ancient",
            "bold", "frozen", "gentle", "wild", "vivid", "shining", "rapid", "curious",
            "restless", "mighty", "faint", "radiant", "stormy", "steady", "silver", "distant",
            "eager", "crimson", "hollow", "bright", "rugged", "calm", "sharp", "soft",
            "dark", "pale", "deep", "tall", "broad", "narrow", "vast", "tiny",
            "noble", "humble", "proud", "weary", "fresh", "stale", "warm", "cold",
            "icy", "fiery", "misty", "cloudy", "sunny", "windy", "dusty", "muddy",
            "rocky", "sandy", "lush", "barren", "fertile", "dry", "damp", "moist",
            "arid", "tropical", "polar", "alpine", "azure", "emerald", "indigo", "scarlet",
            "turquoise", "violet", "loud", "harsh", "tame", "feral", "docile", "savage",
            "tranquil", "modern", "timeless", "eternal", "fleeting", "brief", "lasting", "permanent",
            "temporary", "sudden", "quick", "slow", "lazy", "hasty", "careful", "reckless",
            "cautious", "daring", "timid", "fearless", "clever", "cunning", "wise", "foolish",
            "sly", "shrewd", "naive", "dull", "keen", "loyal", "faithful", "true",
            "false", "honest", "deceptive", "genuine", "fake", "pure", "tainted", "sacred",
            "profane", "holy", "cursed", "blessed", "doomed", "fated", "chosen", "forgotten",
            "remembered", "lonely", "crowded", "empty", "full", "solid", "sturdy", "fragile",
            "brittle", "tough", "smooth", "rough", "jagged", "sleek", "polished", "worn",
            "weathered", "pristine", "tarnished", "gleaming", "luminous", "shadowy", "murky", "clear",
            "hazy", "crisp", "blurry", "muted", "vibrant", "whispering", "roaring", "howling",
            "murmuring", "echoing", "booming", "rustling", "crackling", "humming", "buzzing", "molten",
            "boiling", "chilled", "scorching", "tepid", "lukewarm", "freezing", "blazing", "searing",
            "nimble", "clumsy", "graceful", "awkward", "elegant", "crude", "refined", "coarse",
            "delicate", "robust", "majestic", "modest", "grand", "plain", "ornate", "simple",
            "elaborate", "minimal", "lavish", "austere", "solitary", "sociable", "reclusive", "gregarious",
            "aloof", "friendly", "hostile", "peaceful", "warlike", "serene", "vigilant", "careless",
            "alert", "drowsy", "wakeful", "sluggish", "energetic", "lethargic", "spirited", "dormant",
            "dim", "dazzling", "gloomy", "cheerful", "somber", "joyful", "mournful", "festive",
            "solemn", "resolute", "hesitant", "determined", "uncertain", "confident", "doubtful", "assured",
            "wary", "trusting", "suspicious", "venerable", "youthful", "aged", "withered", "blooming",
            "ripe", "unripe", "mature", "budding", "stalwart", "resilient", "vulnerable", "hardy",
            "tenacious", "yielding", "firm", "pliant", "crystalline", "opaque", "translucent", "transparent"
        };

        constexpr std::array<std::string_view, 256> nouns = {
            "tiger", "river", "mountain", "shadow", "comet", "falcon", "ember", "harbor",
            "storm", "raven", "glacier", "meadow", "phoenix", "canyon", "wolf", "ocean",
            "thunder", "forest", "eagle", "crystal", "horizon", "lantern", "spark", "willow",
            "cascade", "nebula", "cobalt", "granite", "sparrow", "tempest", "badger", "otter",
            "lynx", "heron", "viper", "panther", "cobra", "hawk", "bison", "moose",
            "stag", "fox", "bear", "lion", "puma", "jaguar", "cheetah", "leopard",
            "hyena", "crane", "swan", "owl", "dove", "finch", "robin", "wren",
            "magpie", "condor", "kestrel", "reef", "delta", "valley", "plateau", "summit",
            "ridge", "dune", "cliff", "cavern", "grotto", "volcano", "island", "lagoon",
            "fjord", "tundra", "prairie", "savanna", "jungle", "desert", "oasis", "blaze",
            "flame", "cinder", "spire", "beacon", "meteor", "asteroid", "galaxy", "nova",
            "quasar", "pulsar", "orbit", "zenith", "eclipse", "aurora", "tide", "current",
            "wave", "breeze", "gale", "cyclone", "monsoon", "frost", "blizzard", "avalanche",
            "hail", "mist", "fog", "quartz", "topaz", "opal", "jasper", "onyx",
            "amber", "pearl", "coral", "ivory", "jade", "copper", "bronze", "iron",
            "steel", "titanium", "platinum", "obsidian", "marble", "slate", "maple", "oak",
            "birch", "cedar", "pine", "aspen", "elm", "cypress", "juniper", "blossom",
            "petal", "thorn", "root", "branch", "canopy", "orchard", "grove", "thicket",
            "bramble", "knight", "ranger", "warrior", "hunter", "sentinel", "guardian", "wanderer",
            "nomad", "voyager", "pilgrim", "scholar", "sage", "oracle", "mystic", "alchemist",
            "artisan", "smith", "weaver", "carver", "mason", "citadel", "bastion", "fortress",
            "tower", "keep", "rampart", "gate", "bridge", "archway", "corridor", "compass",
            "anchor", "sail", "mast", "hull", "keel", "rudder", "dock", "pier",
            "lighthouse", "signal", "echo", "whisper", "murmur", "rumor", "legend", "myth",
            "saga", "dagger", "blade", "arrow", "shield", "helm", "gauntlet", "armor",
            "banner", "crest", "emblem", "crow", "vulture", "albatross", "pelican", "gull",
            "tern", "plover", "sandpiper", "curlew", "marlin", "salmon", "trout", "pike",
            "perch", "carp", "eel", "ray", "shark", "dolphin", "orca", "whale",
            "narwhal", "seal", "walrus", "beaver", "muskrat", "platypus", "echidna", "cactus",
            "fern", "moss", "lichen", "vine", "reed", "rush", "sedge", "clover",
            "thistle", "basalt", "limestone", "sandstone", "shale", "pumice", "flint", "chert",
            "gneiss", "schist", "rapids", "spring", "brook", "stream", "creek", "estuary"
        };

        inline std::unordered_map<std::uint16_t, std::string> names_cache{};
    }



    inline std::string_view get_name(std::uint16_t value) {
        // Check if name is cached
        auto [it, inserted] = detail::names_cache.try_emplace(value);

        if (inserted) {
            // Getting max value of key so we can get right adj and noun
            constexpr std::size_t max_value = std::numeric_limits<std::decay_t<decltype(value)>>::max();

            // Getting indices for nouns and adj, also number so we almost cannot get the same names
            const std::size_t adj_index = value * std::size(detail::adjectives) / max_value;
            const std::size_t noun_index = (max_value - value) * std::size(detail::nouns) / max_value;
            const std::size_t number = value * 16384 / max_value;

            // Caching the name
            it->second = std::format("{}-{}-{}",
                detail::adjectives[std::clamp(adj_index, static_cast<std::size_t>(0), std::size(detail::adjectives) - 1)],
                detail::nouns[std::clamp(noun_index, static_cast<std::size_t>(0), std::size(detail::nouns) - 1)],
                number
            );
        }

        return it->second;
    }
}

#endif //MYDAK_BACKEND_NAMER_HPP
