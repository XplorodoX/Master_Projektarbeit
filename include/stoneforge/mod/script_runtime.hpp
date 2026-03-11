#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "stoneforge/mod/mod_loader.hpp"

namespace stoneforge::mod {

class ScriptRuntime {
public:
    ScriptRuntime();
    ~ScriptRuntime();

    bool initialize();
    void loadScripts(const std::vector<LoadedModInfo>& mods);
    void emitEvent(const std::string& eventName, const std::unordered_map<std::string, std::string>& payload);

    bool enabled() const;
    const std::string& lastError() const;

private:
    void* state_ = nullptr;
    bool enabled_ = false;
    std::string lastError_;
};

}  // namespace stoneforge::mod
