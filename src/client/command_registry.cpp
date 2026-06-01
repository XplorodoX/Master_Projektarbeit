#include "stoneforge/client/command_registry.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "stoneforge/game_config.hpp"
#include "stoneforge/item.hpp"

namespace stoneforge::client {

namespace {

std::string toLowerCopy(std::string value) {
    for(char& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

std::string normalizeRegistryId(const std::string& id) {
    if(id.find(':') == std::string::npos) {
        return "stoneforge:" + id;
    }
    return id;
}

bool parseIntStrict(const std::string& value, int& out) {
    if(value.empty()) {
        return false;
    }

    int sign = 1;
    std::size_t idx = 0;
    if(value[0] == '-') {
        sign = -1;
        idx = 1;
    }
    if(idx >= value.size()) {
        return false;
    }

    int result = 0;
    for(; idx < value.size(); ++idx) {
        const char c = value[idx];
        if(c < '0' || c > '9') {
            return false;
        }
        result = result * 10 + static_cast<int>(c - '0');
    }

    out = result * sign;
    return true;
}

std::vector<std::string> splitTokens(const std::string& rawInput) {
    std::istringstream in(rawInput);
    std::vector<std::string> out;
    std::string token;
    while(in >> token) {
        out.push_back(token);
    }
    return out;
}

std::string joinTokens(const std::vector<std::string>& tokens) {
    std::string out;
    for(std::size_t i = 0; i < tokens.size(); ++i) {
        if(i > 0) {
            out += ' ';
        }
        out += tokens[i];
    }
    return out;
}

std::vector<std::string> filterPrefix(const std::vector<std::string>& values, const std::string& prefix) {
    std::vector<std::string> out;
    const std::string lowerPrefix = toLowerCopy(prefix);
    for(const auto& value : values) {
        if(lowerPrefix.empty()) {
            out.push_back(value);
            continue;
        }

        const std::string lowerValue = toLowerCopy(value);
        if(lowerValue.rfind(lowerPrefix, 0) == 0) {
            out.push_back(value);
        }
    }
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

std::vector<std::string> builtinItemIds() {
    return {"stoneforge:wood", "stoneforge:planks", "stoneforge:sticks", "stoneforge:ore", "stoneforge:workbench_kit"};
}

std::vector<std::string> builtinBlockIds() {
    return {
        "stoneforge:empty",
        "stoneforge:wall",
        "stoneforge:resource",
        "stoneforge:exit",
        "stoneforge:tree",
        "stoneforge:workbench",
        "stoneforge:wood_wall",
        "stoneforge:wood_log",
        "stoneforge:structure_grassland",
        "stoneforge:structure_forest",
        "stoneforge:structure_desert",
        "stoneforge:structure_mountain",
        "stoneforge:structure_steppe",
        "stoneforge:structure_tundra",
        "stoneforge:structure_helle",
    };
}

std::vector<std::string> builtinBiomeIds() {
    return {"grasland", "wald", "wueste", "bergland", "steppe", "tundra", "hoelle"};
}

std::vector<std::string> builtinEntityIds() {
    std::vector<std::string> ids;
    for(const auto& [entityId, behavior] : stoneforge::gameConfig().entityBehaviorMap) {
        (void)behavior;
        ids.push_back(entityId);
    }

    if(ids.empty()) {
        ids = {"stoneforge:mob", "stoneforge:zombie", "stoneforge:animal", "stoneforge:boss"};
    }

    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    return ids;
}

bool parseTileId(const std::string& token, stoneforge::TileType& outTile, std::string& outCanonicalId) {
    int numeric = 0;
    if(parseIntStrict(token, numeric)) {
        if(numeric >= 0 && numeric <= static_cast<int>(stoneforge::TileType::StructureHelle)) {
            outTile = static_cast<stoneforge::TileType>(numeric);
            outCanonicalId = builtinBlockIds()[static_cast<std::size_t>(numeric)];
            return true;
        }
        return false;
    }

    const std::string id = normalizeRegistryId(token);
    if(id == "stoneforge:empty") {
        outTile = stoneforge::TileType::Empty;
    } else if(id == "stoneforge:wall") {
        outTile = stoneforge::TileType::Wall;
    } else if(id == "stoneforge:resource") {
        outTile = stoneforge::TileType::Resource;
    } else if(id == "stoneforge:exit") {
        outTile = stoneforge::TileType::Exit;
    } else if(id == "stoneforge:tree") {
        outTile = stoneforge::TileType::Tree;
    } else if(id == "stoneforge:workbench") {
        outTile = stoneforge::TileType::Workbench;
    } else if(id == "stoneforge:wood_wall") {
        outTile = stoneforge::TileType::WoodWall;
    } else if(id == "stoneforge:wood_log") {
        outTile = stoneforge::TileType::WoodLog;
    } else if(id == "stoneforge:structure_grassland") {
        outTile = stoneforge::TileType::StructureGrassland;
    } else if(id == "stoneforge:structure_forest") {
        outTile = stoneforge::TileType::StructureForest;
    } else if(id == "stoneforge:structure_desert") {
        outTile = stoneforge::TileType::StructureDesert;
    } else if(id == "stoneforge:structure_mountain") {
        outTile = stoneforge::TileType::StructureMountain;
    } else if(id == "stoneforge:structure_steppe") {
        outTile = stoneforge::TileType::StructureSteppe;
    } else if(id == "stoneforge:structure_tundra") {
        outTile = stoneforge::TileType::StructureTundra;
    } else if(id == "stoneforge:structure_helle") {
        outTile = stoneforge::TileType::StructureHelle;
    } else {
        return false;
    }

    outCanonicalId = id;
    return true;
}

bool parseNbtLikeToken(const std::string& token, std::string& key, std::string& value) {
    const std::size_t sep = token.find('=');
    if(sep == std::string::npos || sep == 0 || sep + 1 >= token.size()) {
        return false;
    }
    key = toLowerCopy(token.substr(0, sep));
    value = token.substr(sep + 1);
    return true;
}

bool parseBoolLike(const std::string& token, bool& out) {
    const std::string lower = toLowerCopy(token);
    if(lower == "true" || lower == "1" || lower == "yes" || lower == "on") {
        out = true;
        return true;
    }
    if(lower == "false" || lower == "0" || lower == "no" || lower == "off") {
        out = false;
        return true;
    }
    return false;
}

class TpCommand final : public CommandDefinition {
public:
    std::string name() const override {
        return "tp";
    }

    std::vector<std::string> aliases() const override {
        return {"teleport"};
    }

    std::string usage() const override {
        return "/tp <x> <y>";
    }

    std::string shortHelp() const override {
        return "Teleport the player to coordinates.";
    }

    CommandExecutionResult execute(const std::vector<std::string>& tokens, const CommandExecutionContext& ctx) const override {
        if(tokens.size() < 3) {
            return {false, "Usage: " + usage()};
        }

        int x = 0;
        int y = 0;
        if(!parseIntStrict(tokens[1], x) || !parseIntStrict(tokens[2], y)) {
            return {false, "Invalid coordinates."};
        }

        if(!ctx.sim.commandTeleportPlayer({x, y})) {
            return {false, "Teleport failed (target blocked)."};
        }
        return {true, "Teleported to " + std::to_string(x) + "," + std::to_string(y)};
    }
};

class GiveCommand final : public CommandDefinition {
public:
    std::string name() const override {
        return "give";
    }

    std::string usage() const override {
        return "/give <item_id> [count]";
    }

    std::string shortHelp() const override {
        return "Give an item stack to player inventory.";
    }

    std::vector<std::string> suggest(std::size_t argumentIndex, const std::string& prefix, const CommandExecutionContext& ctx) const override {
        if(argumentIndex != 0) {
            return {};
        }
        (void)ctx;
        return filterPrefix(builtinItemIds(), prefix);
    }

    CommandExecutionResult execute(const std::vector<std::string>& tokens, const CommandExecutionContext& ctx) const override {
        if(tokens.size() < 2) {
            return {false, "Usage: " + usage()};
        }

        const std::string itemId = normalizeRegistryId(tokens[1]);

        int count = 1;
        if(tokens.size() >= 3 && !parseIntStrict(tokens[2], count)) {
            return {false, "Invalid count."};
        }
        count = std::clamp(count, 1, 999);

        if(!ctx.sim.commandGiveItem(itemId, count)) {
            return {false, "Inventory full."};
        }
        return {true, "Given " + std::to_string(count) + "x " + stoneforge::itemDisplayName(itemId)};
    }
};

class SetBlockCommand final : public CommandDefinition {
public:
    std::string name() const override {
        return "setblock";
    }

    std::string usage() const override {
        return "/setblock <x> <y> <block_id|tile_id>";
    }

    std::string shortHelp() const override {
        return "Set a block by namespaced ID or tile enum ID.";
    }

    std::vector<std::string> suggest(std::size_t argumentIndex, const std::string& prefix, const CommandExecutionContext& ctx) const override {
        if(argumentIndex != 2) {
            return {};
        }
        (void)ctx;
        return filterPrefix(builtinBlockIds(), prefix);
    }

    CommandExecutionResult execute(const std::vector<std::string>& tokens, const CommandExecutionContext& ctx) const override {
        if(tokens.size() < 4) {
            return {false, "Usage: " + usage()};
        }

        int x = 0;
        int y = 0;
        if(!parseIntStrict(tokens[1], x) || !parseIntStrict(tokens[2], y)) {
            return {false, "Invalid coordinates."};
        }

        stoneforge::TileType tile = stoneforge::TileType::Empty;
        std::string blockId;
        if(!parseTileId(tokens[3], tile, blockId)) {
            return {false, "Unknown block id/tile id: " + tokens[3]};
        }

        if(!ctx.sim.commandSetTile({x, y}, tile)) {
            return {false, "Setblock failed."};
        }

        return {true, "Placed " + blockId + " at " + std::to_string(x) + "," + std::to_string(y)};
    }
};

class SpawnCommand final : public CommandDefinition {
public:
    std::string name() const override {
        return "spawn";
    }

    std::vector<std::string> aliases() const override {
        return {"summon"};
    }

    std::string usage() const override {
        return "/spawn <entity_id> [x y] [hp=..] [aggro=true|false] [variant=name] [behavior=type]";
    }

    std::string shortHelp() const override {
        return "Spawn entities by ID with NBT-like params.";
    }

    std::string longHelp() const override {
        return "Spawn/summon command with NBT-like args.\n"
               "Usage: " + usage() + "\n"
               "Examples:\n"
               "  /spawn stoneforge:zombie\n"
               "  /spawn stoneforge:zombie 12 7 hp=8 aggro=true variant=alpha behavior=zombie\n"
               "  /summon stoneforge:boss hp=20 aggro=1 variant=boss";
    }

    std::vector<std::string> suggest(std::size_t argumentIndex, const std::string& prefix, const CommandExecutionContext& ctx) const override {
        if(argumentIndex == 0) {
            (void)ctx;
            return filterPrefix(builtinEntityIds(), prefix);
        }
        if(argumentIndex >= 3) {
            return filterPrefix({"hp=", "aggro=true", "aggro=false", "variant=", "behavior="}, prefix);
        }
        return {};
    }

    CommandExecutionResult execute(const std::vector<std::string>& tokens, const CommandExecutionContext& ctx) const override {
        if(tokens.size() < 2) {
            return {false, "Usage: " + usage()};
        }

        const std::string entityId = normalizeRegistryId(tokens[1]);

        stoneforge::Vec2i pos = ctx.sim.playerPos();
        std::size_t tokenIndex = 2;
        if(tokens.size() >= 4) {
            int x = 0;
            int y = 0;
            if(parseIntStrict(tokens[2], x) && parseIntStrict(tokens[3], y)) {
                pos = {x, y};
                tokenIndex = 4;
            }
        }

        int hp = 1;
        bool aggro = false;
        std::string variant = "default";
        std::string behavior = stoneforge::gameConfig().behaviorTypeForEntity(entityId, "default");

        for(std::size_t i = tokenIndex; i < tokens.size(); ++i) {
            std::string key;
            std::string value;
            if(!parseNbtLikeToken(tokens[i], key, value)) {
                return {false, "Invalid NBT-like token: " + tokens[i]};
            }

            if(key == "hp") {
                int parsedHp = 0;
                if(!parseIntStrict(value, parsedHp)) {
                    return {false, "Invalid hp value."};
                }
                hp = std::clamp(parsedHp, 1, 500);
            } else if(key == "aggro") {
                bool parsedAggro = false;
                if(!parseBoolLike(value, parsedAggro)) {
                    return {false, "Invalid aggro value."};
                }
                aggro = parsedAggro;
            } else if(key == "variant") {
                variant = value;
            } else if(key == "behavior" || key == "type") {
                behavior = value;
            } else {
                return {false, "Unknown summon parameter: " + key};
            }
        }

        if(!ctx.sim.commandSpawnEntity(entityId, pos, hp, aggro, variant, behavior)) {
            return {false, "Spawn failed (target blocked/occupied)."};
        }

        return {true, "Spawned " + entityId + " at " + std::to_string(pos.x) + "," + std::to_string(pos.y)};
    }
};

class BiomeCommand final : public CommandDefinition {
public:
    std::string name() const override {
        return "biome";
    }

    std::string usage() const override {
        return "/biome list";
    }

    std::string shortHelp() const override {
        return "List built-in biome IDs.";
    }

    std::vector<std::string> suggest(std::size_t argumentIndex, const std::string& prefix, const CommandExecutionContext& ctx) const override {
        if(argumentIndex == 0) {
            return filterPrefix({"list"}, prefix);
        }
        (void)ctx;
        return {};
    }

    CommandExecutionResult execute(const std::vector<std::string>& tokens, const CommandExecutionContext& ctx) const override {
        if(tokens.size() < 2 || toLowerCopy(tokens[1]) != "list") {
            return {false, "Usage: " + usage()};
        }

        (void)ctx;
        const auto ids = builtinBiomeIds();
        std::string out = "Biomes (" + std::to_string(ids.size()) + "): ";
        for(std::size_t i = 0; i < ids.size(); ++i) {
            if(i > 0) {
                out += ", ";
            }
            out += ids[i];
        }
        return {true, out};
    }
};

class EntityCommand final : public CommandDefinition {
public:
    std::string name() const override {
        return "entity";
    }

    std::string usage() const override {
        return "/entity list";
    }

    std::string shortHelp() const override {
        return "List configured entity IDs.";
    }

    std::vector<std::string> suggest(std::size_t argumentIndex, const std::string& prefix, const CommandExecutionContext& ctx) const override {
        if(argumentIndex == 0) {
            return filterPrefix({"list"}, prefix);
        }
        (void)ctx;
        return {};
    }

    CommandExecutionResult execute(const std::vector<std::string>& tokens, const CommandExecutionContext& ctx) const override {
        if(tokens.size() < 2 || toLowerCopy(tokens[1]) != "list") {
            return {false, "Usage: " + usage()};
        }

        (void)ctx;
        const auto entries = builtinEntityIds();
        if(entries.empty()) {
            return {false, "No entities configured."};
        }

        std::string out = "Entities (" + std::to_string(entries.size()) + "): ";
        for(std::size_t i = 0; i < entries.size() && i < 10; ++i) {
            if(i > 0) {
                out += ", ";
            }
            out += entries[i];
        }
        if(entries.size() > 10) {
            out += ", ...";
        }
        return {true, out};
    }
};

class BlockCommand final : public CommandDefinition {
public:
    std::string name() const override {
        return "block";
    }

    std::string usage() const override {
        return "/block list";
    }

    std::string shortHelp() const override {
        return "List available block IDs.";
    }

    std::vector<std::string> suggest(std::size_t argumentIndex, const std::string& prefix, const CommandExecutionContext& ctx) const override {
        if(argumentIndex == 0) {
            return filterPrefix({"list"}, prefix);
        }
        (void)ctx;
        return {};
    }

    CommandExecutionResult execute(const std::vector<std::string>& tokens, const CommandExecutionContext& ctx) const override {
        if(tokens.size() < 2 || toLowerCopy(tokens[1]) != "list") {
            return {false, "Usage: " + usage()};
        }

        (void)ctx;
        const auto entries = builtinBlockIds();
        if(entries.empty()) {
            return {false, "No blocks available."};
        }

        std::string out = "Blocks (" + std::to_string(entries.size()) + "): ";
        for(std::size_t i = 0; i < entries.size() && i < 10; ++i) {
            if(i > 0) {
                out += ", ";
            }
            out += entries[i];
        }
        if(entries.size() > 10) {
            out += ", ...";
        }
        return {true, out};
    }
};

}  // namespace

std::vector<std::string> CommandDefinition::aliases() const {
    return {};
}

std::string CommandDefinition::longHelp() const {
    return shortHelp() + "\nUsage: " + usage();
}

std::vector<std::string> CommandDefinition::suggest(std::size_t argumentIndex, const std::string& prefix, const CommandExecutionContext& ctx) const {
    (void)argumentIndex;
    (void)prefix;
    (void)ctx;
    return {};
}

CommandRegistry::CommandRegistry() {
    registerCommand(std::make_unique<TpCommand>());
    registerCommand(std::make_unique<GiveCommand>());
    registerCommand(std::make_unique<SetBlockCommand>());
    registerCommand(std::make_unique<SpawnCommand>());
    registerCommand(std::make_unique<BiomeCommand>());
    registerCommand(std::make_unique<BlockCommand>());
    registerCommand(std::make_unique<EntityCommand>());

    helpPages_["gameplay"] =
        "Gameplay Commands:\n"
        "  /tp <x> <y>\n"
        "  /give <item_id> [count]\n"
        "  /setblock <x> <y> <block_id|tile_id>\n"
        "  /spawn <entity_id> [x y] [hp=..] [aggro=true|false] [variant=name] [behavior=type]\n"
        "  /block list\n"
        "  /entity list";

    helpPages_["registry"] =
        "ID System:\n"
        "  - Blocks and entities use namespaced IDs like stoneforge:wall\n"
        "  - setblock also accepts tile enum IDs (0..7)\n"
        "  - List entries with /biome list, /block list and /entity list";

    helpPages_["nbt"] =
        "NBT-like Summon Params:\n"
        "  hp=<int>\n"
        "  aggro=<true|false|1|0>\n"
        "  variant=<name>\n"
        "  behavior=<zombie|animal|boss|default>\n"
        "Example: /summon stoneforge:zombie 12 7 hp=20 aggro=true variant=alpha behavior=zombie";
}

CommandExecutionResult CommandRegistry::execute(const std::string& rawInput, const CommandExecutionContext& ctx) const {
    std::string normalized = rawInput;
    if(!normalized.empty() && normalized[0] == '/') {
        normalized.erase(0, 1);
    }

    const auto tokens = splitTokens(normalized);
    if(tokens.empty()) {
        return {false, "Command is empty."};
    }

    const std::string key = toLowerCopy(tokens[0]);
    if(key == "help") {
        if(tokens.size() == 1) {
            return {true, helpOverview()};
        }
        return {true, helpFor(toLowerCopy(tokens[1]))};
    }

    const auto* command = findCommand(key);
    if(command == nullptr) {
        return {false, "Unknown command. Use /help"};
    }

    return command->execute(tokens, ctx);
}

std::vector<std::string> CommandRegistry::autocomplete(const std::string& rawInput, const CommandExecutionContext& ctx) const {
    const bool endsWithSpace = !rawInput.empty() && std::isspace(static_cast<unsigned char>(rawInput.back())) != 0;

    std::string normalized = rawInput;
    if(!normalized.empty() && normalized[0] == '/') {
        normalized.erase(0, 1);
    }
    auto tokens = splitTokens(normalized);

    std::vector<std::string> results;

    if(tokens.empty()) {
        for(const auto& [name, cmd] : commandLookup_) {
            (void)cmd;
            if(name.find(' ') == std::string::npos) {
                results.push_back('/' + name + ' ');
            }
        }
        results.push_back("/help ");
    } else if(tokens.size() == 1 && !endsWithSpace) {
        const auto prefix = toLowerCopy(tokens[0]);
        for(const auto& [name, cmd] : commandLookup_) {
            (void)cmd;
            if(name.find(' ') != std::string::npos) {
                continue;
            }
            if(name.rfind(prefix, 0) == 0) {
                results.push_back('/' + name + ' ');
            }
        }
        if(std::string("help").rfind(prefix, 0) == 0) {
            results.push_back("/help ");
        }
    } else {
        const std::string cmdToken = toLowerCopy(tokens[0]);
        if(cmdToken == "help") {
            const std::string prefix = (endsWithSpace ? std::string{} : toLowerCopy(tokens.back()));
            std::vector<std::string> topics;
            for(const auto& [topic, body] : helpPages_) {
                (void)body;
                topics.push_back(topic);
            }
            for(const auto& [name, cmd] : commandLookup_) {
                (void)cmd;
                if(name.find(' ') == std::string::npos) {
                    topics.push_back(name);
                }
            }
            for(const auto& topic : filterPrefix(topics, prefix)) {
                results.push_back("/help " + topic + ' ');
            }
        } else {
            const auto* command = findCommand(cmdToken);
            if(command != nullptr) {
                const std::size_t argIndex = endsWithSpace ? (tokens.size() - 1) : (tokens.size() - 2);
                const std::string prefix = endsWithSpace ? std::string{} : tokens.back();
                auto suggestions = command->suggest(argIndex, prefix, ctx);

                for(const auto& suggestion : suggestions) {
                    std::vector<std::string> completed = tokens;
                    if(endsWithSpace) {
                        completed.push_back(suggestion);
                    } else if(!completed.empty()) {
                        completed.back() = suggestion;
                    }
                    results.push_back('/' + joinTokens(completed) + ' ');
                }
            }
        }
    }

    std::sort(results.begin(), results.end());
    results.erase(std::unique(results.begin(), results.end()), results.end());
    if(results.size() > 16) {
        results.resize(16);
    }
    return results;
}

void CommandRegistry::registerCommand(std::unique_ptr<CommandDefinition> command) {
    if(!command) {
        return;
    }

    const std::string primary = toLowerCopy(command->name());
    commandLookup_[primary] = command.get();
    for(const auto& alias : command->aliases()) {
        commandLookup_[toLowerCopy(alias)] = command.get();
    }
    commands_.push_back(std::move(command));
}

const CommandDefinition* CommandRegistry::findCommand(const std::string& nameOrAlias) const {
    const auto it = commandLookup_.find(toLowerCopy(nameOrAlias));
    if(it == commandLookup_.end()) {
        return nullptr;
    }
    return it->second;
}

std::string CommandRegistry::helpOverview() const {
    std::string out = "Help pages: gameplay, registry, nbt\nCommands:\n";
    std::vector<std::string> names;
    names.reserve(commands_.size());
    for(const auto& command : commands_) {
        names.push_back(command->name());
    }
    std::sort(names.begin(), names.end());
    for(const auto& name : names) {
        const auto* cmd = findCommand(name);
        if(cmd == nullptr) {
            continue;
        }
        out += "  /" + name + " - " + cmd->shortHelp() + "\n";
    }
    out += "Use /help <command|page> for details.";
    return out;
}

std::string CommandRegistry::helpFor(const std::string& topic) const {
    const auto pageIt = helpPages_.find(topic);
    if(pageIt != helpPages_.end()) {
        return pageIt->second;
    }

    const auto* cmd = findCommand(topic);
    if(cmd != nullptr) {
        return cmd->longHelp();
    }

    return "No help found for '" + topic + "'.";
}

}  // namespace stoneforge::client
