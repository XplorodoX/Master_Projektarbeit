#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "stoneforge/simulation.hpp"

namespace stoneforge::client {

struct CommandExecutionContext {
    stoneforge::Simulation& sim;
};

struct CommandExecutionResult {
    bool success = false;
    std::string message;
};

class CommandDefinition {
public:
    virtual ~CommandDefinition() = default;

    virtual std::string name() const = 0;
    virtual std::vector<std::string> aliases() const;
    virtual std::string usage() const = 0;
    virtual std::string shortHelp() const = 0;
    virtual std::string longHelp() const;

    virtual CommandExecutionResult execute(const std::vector<std::string>& tokens, const CommandExecutionContext& ctx) const = 0;
    virtual std::vector<std::string> suggest(std::size_t argumentIndex, const std::string& prefix, const CommandExecutionContext& ctx) const;
};

class CommandRegistry {
public:
    CommandRegistry();

    CommandExecutionResult execute(const std::string& rawInput, const CommandExecutionContext& ctx) const;
    std::vector<std::string> autocomplete(const std::string& rawInput, const CommandExecutionContext& ctx) const;

private:
    void registerCommand(std::unique_ptr<CommandDefinition> command);
    const CommandDefinition* findCommand(const std::string& nameOrAlias) const;

    std::string helpOverview() const;
    std::string helpFor(const std::string& topic) const;

    std::vector<std::unique_ptr<CommandDefinition>> commands_;
    std::unordered_map<std::string, const CommandDefinition*> commandLookup_;
    std::unordered_map<std::string, std::string> helpPages_;
};

}  // namespace stoneforge::client
