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

        inline std::unordered_map<std::string, std::string> names_cache{};
    }



    inline std::string_view get_name(const std::string& public_key) {
        // Check if name is cached
        const auto it = detail::names_cache.find(public_key);
        if (it != detail::names_cache.end()) return it->second;

        // Check if public_key is right length
        if (std::size(public_key) != proto::E2E_KEYS_L * 2) return "missingno";

        // Getting max value of key so we can get right adj and noun
        constexpr std::size_t max_value = 256 * proto::E2E_KEYS_L;

        std::size_t value = 0;
        for (const char character : public_key) {
            value += character;
        }


        // Getting indices for nouns and adj, also number so we almost cannot get the same names
        std::cout << value << " " << max_value << std::endl;
        const std::size_t index1 = value / max_value * std::size(detail::adjectives);
        const std::size_t index2 = (max_value - value) / max_value * std::size(detail::nouns);
        const std::size_t number = 1;

        // Caching the name
        auto& name = detail::names_cache[public_key];
        name = std::format("{}-{}-{}",detail::adjectives[index1], detail::nouns[index2], number);

        return name;
    }
}

#endif //MYDAK_BACKEND_NAMER_HPP
