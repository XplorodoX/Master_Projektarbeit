#include "stoneforge/recipe.hpp"

#include <fstream>
#include <optional>
#include <sstream>

#include <nlohmann/json.hpp>

#include "stoneforge/item.hpp"
#include "stoneforge/simulation.hpp"

namespace stoneforge {

namespace {

using json = nlohmann::json;

std::optional<RecipeId> recipeIdFromKey(std::string_view key) {
    if(key == "planks" || key == "stoneforge:planks") {
        return RecipeId::Planks;
    }
    if(key == "sticks" || key == "stoneforge:sticks") {
        return RecipeId::Sticks;
    }
    if(key == "workbench" || key == "stoneforge:workbench") {
        return RecipeId::Workbench;
    }
    if(key == "axe_tier1" || key == "stoneforge:axe_tier1") {
        return RecipeId::AxeTier1;
    }
    if(key == "pickaxe_tier1" || key == "stoneforge:pickaxe_tier1") {
        return RecipeId::PickaxeTier1;
    }
    if(key == "axe_tier2" || key == "stoneforge:axe_tier2") {
        return RecipeId::AxeTier2;
    }
    if(key == "pickaxe_tier2" || key == "stoneforge:pickaxe_tier2") {
        return RecipeId::PickaxeTier2;
    }
    return std::nullopt;
}

bool parseStackArray(const json& arrayNode, std::vector<ItemStackSpec>& out, std::string* errorMessage) {
    if(!arrayNode.is_array()) {
        if(errorMessage) {
            *errorMessage = "recipe stack list must be an array";
        }
        return false;
    }

    out.clear();
    out.reserve(arrayNode.size());
    for(const auto& node : arrayNode) {
        if(!node.is_object()) {
            continue;
        }

        const std::string itemKey = node.value("item", "");
        const int count = node.value("count", 0);
        const ItemId item = itemIdFromKey(itemKey);
        if(item == ItemId::None || count <= 0) {
            if(errorMessage) {
                *errorMessage = "invalid item stack in recipe json: item='" + itemKey + "'";
            }
            return false;
        }
        out.push_back(ItemStackSpec{item, count});
    }

    return true;
}

std::optional<json> readJsonFile(const std::filesystem::path& path, std::string* errorMessage) {
    std::ifstream in(path);
    if(!in.is_open()) {
        if(errorMessage) {
            *errorMessage = "cannot open recipe file: " + path.string();
        }
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    try {
        return json::parse(buffer.str());
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = "recipe json parse error in " + path.string() + ": " + ex.what();
        }
        return std::nullopt;
    }
}

}  // namespace

ItemRecipe::ItemRecipe(
    RecipeId id,
    std::string key,
    std::string label,
    std::vector<ItemStackSpec> inputs,
    std::vector<ItemStackSpec> outputs,
    bool requiresWorkbench
)
    : id_(id),
      key_(std::move(key)),
      label_(std::move(label)),
      inputs_(std::move(inputs)),
      outputs_(std::move(outputs)),
      requiresWorkbench_(requiresWorkbench) {}

RecipeId ItemRecipe::id() const {
    return id_;
}

std::string_view ItemRecipe::key() const {
    return key_;
}

std::string_view ItemRecipe::label() const {
    return label_;
}

bool ItemRecipe::requiresWorkbench() const {
    return requiresWorkbench_;
}

const std::vector<ItemStackSpec>& ItemRecipe::inputs() const {
    return inputs_;
}

const std::vector<ItemStackSpec>& ItemRecipe::outputs() const {
    return outputs_;
}

bool ItemRecipe::canCraftWithTools(int /*axeLevel*/, int /*pickaxeLevel*/) const {
    return true;
}

void ItemRecipe::applyToolUpgrades(int& /*axeLevel*/, int& /*pickaxeLevel*/) const {}

ToolRecipe::ToolRecipe(
    RecipeId id,
    std::string key,
    std::string label,
    std::vector<ItemStackSpec> inputs,
    bool requiresWorkbench,
    int minAxeLevel,
    int maxAxeLevel,
    int minPickaxeLevel,
    int maxPickaxeLevel,
    int setAxeLevel,
    int setPickaxeLevel
)
    : id_(id),
      key_(std::move(key)),
      label_(std::move(label)),
      inputs_(std::move(inputs)),
      requiresWorkbench_(requiresWorkbench),
      minAxeLevel_(minAxeLevel),
      maxAxeLevel_(maxAxeLevel),
      minPickaxeLevel_(minPickaxeLevel),
      maxPickaxeLevel_(maxPickaxeLevel),
      setAxeLevel_(setAxeLevel),
      setPickaxeLevel_(setPickaxeLevel) {}

RecipeId ToolRecipe::id() const {
    return id_;
}

std::string_view ToolRecipe::key() const {
    return key_;
}

std::string_view ToolRecipe::label() const {
    return label_;
}

bool ToolRecipe::requiresWorkbench() const {
    return requiresWorkbench_;
}

const std::vector<ItemStackSpec>& ToolRecipe::inputs() const {
    return inputs_;
}

const std::vector<ItemStackSpec>& ToolRecipe::outputs() const {
    return outputs_;
}

bool ToolRecipe::canCraftWithTools(int axeLevel, int pickaxeLevel) const {
    if(minAxeLevel_ >= 0 && axeLevel < minAxeLevel_) {
        return false;
    }
    if(maxAxeLevel_ >= 0 && axeLevel > maxAxeLevel_) {
        return false;
    }
    if(minPickaxeLevel_ >= 0 && pickaxeLevel < minPickaxeLevel_) {
        return false;
    }
    if(maxPickaxeLevel_ >= 0 && pickaxeLevel > maxPickaxeLevel_) {
        return false;
    }
    return true;
}

void ToolRecipe::applyToolUpgrades(int& axeLevel, int& pickaxeLevel) const {
    if(setAxeLevel_ >= 0) {
        axeLevel = setAxeLevel_;
    }
    if(setPickaxeLevel_ >= 0) {
        pickaxeLevel = setPickaxeLevel_;
    }
}

RecipeCatalog::RecipeCatalog() {
    registerRecipe(std::make_unique<ItemRecipe>(
        RecipeId::Planks,
        "stoneforge:planks",
        "Planks x4",
        std::vector<ItemStackSpec>{{ItemId::Wood, 1}},
        std::vector<ItemStackSpec>{{ItemId::Planks, 4}},
        false
    ));

    registerRecipe(std::make_unique<ItemRecipe>(
        RecipeId::Sticks,
        "stoneforge:sticks",
        "Sticks x4",
        std::vector<ItemStackSpec>{{ItemId::Planks, 2}},
        std::vector<ItemStackSpec>{{ItemId::Sticks, 4}},
        false
    ));

    registerRecipe(std::make_unique<ItemRecipe>(
        RecipeId::Workbench,
        "stoneforge:workbench",
        "Workbench Kit x1",
        std::vector<ItemStackSpec>{{ItemId::Planks, 10}},
        std::vector<ItemStackSpec>{{ItemId::WorkbenchKit, 1}},
        false
    ));

    registerRecipe(std::make_unique<ToolRecipe>(
        RecipeId::AxeTier1,
        "stoneforge:axe_tier1",
        "Axe Lv1",
        std::vector<ItemStackSpec>{{ItemId::Planks, 3}, {ItemId::Sticks, 2}},
        true,
        -1,
        0,
        -1,
        -1,
        1,
        -1
    ));

    registerRecipe(std::make_unique<ToolRecipe>(
        RecipeId::PickaxeTier1,
        "stoneforge:pickaxe_tier1",
        "Pickaxe Lv1",
        std::vector<ItemStackSpec>{{ItemId::Planks, 3}, {ItemId::Sticks, 2}},
        true,
        -1,
        -1,
        -1,
        0,
        -1,
        1
    ));

    registerRecipe(std::make_unique<ToolRecipe>(
        RecipeId::AxeTier2,
        "stoneforge:axe_tier2",
        "Axe Lv2",
        std::vector<ItemStackSpec>{{ItemId::Ore, 3}, {ItemId::Sticks, 2}},
        true,
        1,
        1,
        -1,
        -1,
        2,
        -1
    ));

    registerRecipe(std::make_unique<ToolRecipe>(
        RecipeId::PickaxeTier2,
        "stoneforge:pickaxe_tier2",
        "Pickaxe Lv2",
        std::vector<ItemStackSpec>{{ItemId::Ore, 3}, {ItemId::Sticks, 2}},
        true,
        -1,
        -1,
        1,
        1,
        -1,
        2
    ));
}

const RecipeBase* RecipeCatalog::find(RecipeId recipeId) const {
    const auto it = recipes_.find(recipeId);
    if(it == recipes_.end()) {
        return nullptr;
    }
    return it->second.get();
}

void RecipeCatalog::registerRecipe(std::unique_ptr<RecipeBase> recipe) {
    if(!recipe) {
        return;
    }
    recipes_[recipe->id()] = std::move(recipe);
}

bool RecipeCatalog::loadJsonFile(const std::filesystem::path& jsonPath, const std::string& sourceNamespace, std::string* errorMessage) {
    auto jsonOpt = readJsonFile(jsonPath, errorMessage);
    if(!jsonOpt) {
        return false;
    }

    const auto& root = *jsonOpt;
    if(!root.is_array()) {
        if(errorMessage) {
            *errorMessage = "recipe root must be an array in " + jsonPath.string();
        }
        return false;
    }

    for(const auto& node : root) {
        if(!node.is_object()) {
            continue;
        }

        std::string idText = node.value("id", "");
        if(idText.empty()) {
            continue;
        }

        if(idText.find(':') == std::string::npos) {
            idText = sourceNamespace + ":" + idText;
        }

        auto recipeIdOpt = recipeIdFromKey(idText);
        if(!recipeIdOpt) {
            continue;
        }

        std::vector<ItemStackSpec> inputs;
        std::vector<ItemStackSpec> outputs;
        if(!parseStackArray(node.value("inputs", json::array()), inputs, errorMessage)) {
            return false;
        }
        if(!parseStackArray(node.value("outputs", json::array()), outputs, errorMessage)) {
            return false;
        }

        const std::string type = node.value("type", "item");
        const std::string label = node.value("label", idText);
        const bool requiresWorkbench = node.value("requiresWorkbench", false);

        if(type == "tool_upgrade") {
            registerRecipe(std::make_unique<ToolRecipe>(
                *recipeIdOpt,
                idText,
                label,
                inputs,
                requiresWorkbench,
                node.value("minAxeLevel", -1),
                node.value("maxAxeLevel", -1),
                node.value("minPickaxeLevel", -1),
                node.value("maxPickaxeLevel", -1),
                node.value("setAxeLevel", -1),
                node.value("setPickaxeLevel", -1)
            ));
        } else {
            registerRecipe(std::make_unique<ItemRecipe>(
                *recipeIdOpt,
                idText,
                label,
                inputs,
                outputs,
                requiresWorkbench
            ));
        }
    }

    return true;
}

RecipeCatalog& recipeCatalog() {
    static RecipeCatalog catalog;
    return catalog;
}

}  // namespace stoneforge
