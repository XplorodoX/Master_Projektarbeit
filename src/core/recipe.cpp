#include "stoneforge/recipe.hpp"

#include <algorithm>
#include <fstream>
#include <optional>
#include <sstream>

#include <nlohmann/json.hpp>

#include "stoneforge/item.hpp"

namespace stoneforge {

namespace {

using json = nlohmann::json;

std::string resolveRecipeItemId(const std::string& itemToken, const std::string& sourceNamespace) {
    if(itemToken.empty()) {
        return {};
    }

    if(itemToken.find(':') != std::string::npos) {
        return itemToken;
    }

    if(itemIdFromKey(itemToken) != ItemId::None) {
        return normalizeItemKey(itemToken);
    }

    if(!sourceNamespace.empty()) {
        return sourceNamespace + ":" + itemToken;
    }

    return normalizeItemKey(itemToken);
}

bool parseStackArray(const json& arrayNode, std::vector<ItemStackSpec>& out, const std::string& sourceNamespace, std::string* errorMessage) {
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
        const std::string itemId = resolveRecipeItemId(itemKey, sourceNamespace);
        if(itemId.empty() || count <= 0) {
            if(errorMessage) {
                *errorMessage = "invalid item stack in recipe json: item='" + itemKey + "'";
            }
            return false;
        }
        out.push_back(ItemStackSpec{itemId, count});
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
    std::string id,
    std::string label,
    std::vector<ItemStackSpec> inputs,
    std::vector<ItemStackSpec> outputs,
    bool requiresWorkbench
)
    : id_(std::move(id)),
      label_(std::move(label)),
      inputs_(std::move(inputs)),
      outputs_(std::move(outputs)),
      requiresWorkbench_(requiresWorkbench) {}

std::string_view ItemRecipe::id() const {
    return id_;
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
    std::string id,
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
    : id_(std::move(id)),
      label_(std::move(label)),
      inputs_(std::move(inputs)),
      requiresWorkbench_(requiresWorkbench),
      minAxeLevel_(minAxeLevel),
      maxAxeLevel_(maxAxeLevel),
      minPickaxeLevel_(minPickaxeLevel),
      maxPickaxeLevel_(maxPickaxeLevel),
      setAxeLevel_(setAxeLevel),
      setPickaxeLevel_(setPickaxeLevel) {}

std::string_view ToolRecipe::id() const {
    return id_;
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
        "stoneforge:planks",
        "Planks x4",
        std::vector<ItemStackSpec>{{"stoneforge:wood", 1}},
        std::vector<ItemStackSpec>{{"stoneforge:planks", 4}},
        false
    ));

    registerRecipe(std::make_unique<ItemRecipe>(
        "stoneforge:sticks",
        "Sticks x4",
        std::vector<ItemStackSpec>{{"stoneforge:planks", 2}},
        std::vector<ItemStackSpec>{{"stoneforge:sticks", 4}},
        false
    ));

    registerRecipe(std::make_unique<ItemRecipe>(
        "stoneforge:workbench",
        "Workbench Kit x1",
        std::vector<ItemStackSpec>{{"stoneforge:planks", 10}},
        std::vector<ItemStackSpec>{{"stoneforge:workbench_kit", 1}},
        false
    ));

    registerRecipe(std::make_unique<ToolRecipe>(
        "stoneforge:axe_tier1",
        "Axe Lv1",
        std::vector<ItemStackSpec>{{"stoneforge:planks", 3}, {"stoneforge:sticks", 2}},
        true,
        -1,
        0,
        -1,
        -1,
        1,
        -1
    ));

    registerRecipe(std::make_unique<ToolRecipe>(
        "stoneforge:pickaxe_tier1",
        "Pickaxe Lv1",
        std::vector<ItemStackSpec>{{"stoneforge:planks", 3}, {"stoneforge:sticks", 2}},
        true,
        -1,
        -1,
        -1,
        0,
        -1,
        1
    ));

    registerRecipe(std::make_unique<ToolRecipe>(
        "stoneforge:axe_tier2",
        "Axe Lv2",
        std::vector<ItemStackSpec>{{"stoneforge:ore", 3}, {"stoneforge:sticks", 2}},
        true,
        1,
        1,
        -1,
        -1,
        2,
        -1
    ));

    registerRecipe(std::make_unique<ToolRecipe>(
        "stoneforge:pickaxe_tier2",
        "Pickaxe Lv2",
        std::vector<ItemStackSpec>{{"stoneforge:ore", 3}, {"stoneforge:sticks", 2}},
        true,
        -1,
        -1,
        1,
        1,
        -1,
        2
    ));
}

const RecipeBase* RecipeCatalog::find(std::string_view recipeId) const {
    if(recipeId.empty()) {
        return nullptr;
    }

    std::string key(recipeId);
    const auto direct = recipes_.find(key);
    if(direct != recipes_.end()) {
        return direct->second.get();
    }

    if(key.find(':') == std::string::npos) {
        const std::string stoneforgeKey = "stoneforge:" + key;
        const auto sf = recipes_.find(stoneforgeKey);
        if(sf != recipes_.end()) {
            return sf->second.get();
        }
    }

    const std::string suffix = ":" + key;
    const RecipeBase* matched = nullptr;
    for(const auto& [id, recipe] : recipes_) {
        if(id.size() >= suffix.size() && id.compare(id.size() - suffix.size(), suffix.size(), suffix) == 0) {
            if(matched != nullptr) {
                return nullptr;
            }
            matched = recipe.get();
        }
    }
    return matched;
}

std::vector<const RecipeBase*> RecipeCatalog::all() const {
    std::vector<const RecipeBase*> out;
    out.reserve(recipes_.size());
    for(const auto& [id, recipe] : recipes_) {
        (void)id;
        out.push_back(recipe.get());
    }

    std::sort(out.begin(), out.end(), [](const RecipeBase* a, const RecipeBase* b) {
        return a->id() < b->id();
    });
    return out;
}

void RecipeCatalog::registerRecipe(std::unique_ptr<RecipeBase> recipe) {
    if(!recipe) {
        return;
    }

    const std::string id(recipe->id());
    if(id.empty()) {
        return;
    }

    recipes_[id] = std::move(recipe);
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

        std::vector<ItemStackSpec> inputs;
        std::vector<ItemStackSpec> outputs;
        if(!parseStackArray(node.value("inputs", json::array()), inputs, sourceNamespace, errorMessage)) {
            return false;
        }
        if(!parseStackArray(node.value("outputs", json::array()), outputs, sourceNamespace, errorMessage)) {
            return false;
        }

        const std::string type = node.value("type", "item");
        const std::string label = node.value("label", idText);
        const bool requiresWorkbench = node.value("requiresWorkbench", false);

        if(type == "tool_upgrade") {
            registerRecipe(std::make_unique<ToolRecipe>(
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
