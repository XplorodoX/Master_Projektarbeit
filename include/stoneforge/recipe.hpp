#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "stoneforge/simulation.hpp"

namespace stoneforge {

struct ItemStackSpec {
    ItemId item = ItemId::None;
    int count = 0;
};

class RecipeBase {
public:
    virtual ~RecipeBase() = default;

    virtual RecipeId id() const = 0;
    virtual std::string_view key() const = 0;
    virtual std::string_view label() const = 0;

    virtual bool requiresWorkbench() const = 0;
    virtual const std::vector<ItemStackSpec>& inputs() const = 0;
    virtual const std::vector<ItemStackSpec>& outputs() const = 0;

    virtual bool canCraftWithTools(int axeLevel, int pickaxeLevel) const = 0;
    virtual void applyToolUpgrades(int& axeLevel, int& pickaxeLevel) const = 0;
};

class ItemRecipe final : public RecipeBase {
public:
    ItemRecipe(
        RecipeId id,
        std::string key,
        std::string label,
        std::vector<ItemStackSpec> inputs,
        std::vector<ItemStackSpec> outputs,
        bool requiresWorkbench
    );

    RecipeId id() const override;
    std::string_view key() const override;
    std::string_view label() const override;
    bool requiresWorkbench() const override;
    const std::vector<ItemStackSpec>& inputs() const override;
    const std::vector<ItemStackSpec>& outputs() const override;
    bool canCraftWithTools(int axeLevel, int pickaxeLevel) const override;
    void applyToolUpgrades(int& axeLevel, int& pickaxeLevel) const override;

private:
    RecipeId id_ = RecipeId::Planks;
    std::string key_;
    std::string label_;
    std::vector<ItemStackSpec> inputs_;
    std::vector<ItemStackSpec> outputs_;
    bool requiresWorkbench_ = false;
};

class ToolRecipe final : public RecipeBase {
public:
    ToolRecipe(
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
    );

    RecipeId id() const override;
    std::string_view key() const override;
    std::string_view label() const override;
    bool requiresWorkbench() const override;
    const std::vector<ItemStackSpec>& inputs() const override;
    const std::vector<ItemStackSpec>& outputs() const override;
    bool canCraftWithTools(int axeLevel, int pickaxeLevel) const override;
    void applyToolUpgrades(int& axeLevel, int& pickaxeLevel) const override;

private:
    RecipeId id_ = RecipeId::Planks;
    std::string key_;
    std::string label_;
    std::vector<ItemStackSpec> inputs_;
    std::vector<ItemStackSpec> outputs_;
    bool requiresWorkbench_ = true;
    int minAxeLevel_ = -1;
    int maxAxeLevel_ = -1;
    int minPickaxeLevel_ = -1;
    int maxPickaxeLevel_ = -1;
    int setAxeLevel_ = -1;
    int setPickaxeLevel_ = -1;
};

class RecipeCatalog {
public:
    RecipeCatalog();

    const RecipeBase* find(RecipeId recipeId) const;

    void registerRecipe(std::unique_ptr<RecipeBase> recipe);
    bool loadJsonFile(const std::filesystem::path& jsonPath, const std::string& sourceNamespace, std::string* errorMessage);

private:
    std::unordered_map<RecipeId, std::unique_ptr<RecipeBase>> recipes_;
};

RecipeCatalog& recipeCatalog();

}  // namespace stoneforge
