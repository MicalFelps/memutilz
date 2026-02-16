```cpp
// main.cpp
#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

//
// ---------------------
// Plugin Interface
// ---------------------
struct CLIPlugin {
    virtual ~CLIPlugin() = default;
    virtual void registerOptions(CLI::App &app) = 0;
    virtual void printValues() const = 0;
};

//
// ---------------------
// Example Plugin: Docking
// ---------------------
struct DockingPlugin : CLIPlugin {
    std::string dockStyle{};
    int dockCount{0};

    void registerOptions(CLI::App &app) override {
        auto group = app.add_option_group("Docking Plugin Options");
        group->add_option("-d,--dock-style", dockStyle, "Dock style (e.g., tabbed, stacked)");
        group->add_option("--dock-count", dockCount, "Number of docks");
    }

    void printValues() const override {
        std::cout << "[DockingPlugin] dockStyle = " << dockStyle
                  << ", dockCount = " << dockCount << "\n";
    }
};

//
// ---------------------
// Example Plugin: Logging
// ---------------------
struct LoggingPlugin : CLIPlugin {
    bool verbose{false};
    std::string logFile;

    void registerOptions(CLI::App &app) override {
        auto group = app.add_option_group("Logging Plugin Options");
        group->add_flag("-v,--verbose", verbose, "Enable verbose output");
        group->add_option("--log-file", logFile, "Path to log file");
    }

    void printValues() const override {
        std::cout << "[LoggingPlugin] verbose = " << (verbose ? "true" : "false")
                  << ", logFile = " << logFile << "\n";
    }
};

//
// ---------------------
// Global Options Namespace
// ---------------------
namespace Options {
    inline std::string file;
    inline int pid = 0;
}

int main(int argc, char** argv) {
    CLI::App app{"MyApp - Example CLI11 App"};

    // Global options
    auto globalGroup = app.add_option_group("Global Options");
    globalGroup->add_option("-f,--file", Options::file, "Input file");
    globalGroup->add_option("-p,--pid", Options::pid, "Process ID");

    // Subcommand: docking
    auto dockingCmd = app.add_subcommand("docking", "Docking specific commands");

    // Plugins
    std::vector<std::unique_ptr<CLIPlugin>> plugins;
    plugins.push_back(std::make_unique<DockingPlugin>());
    plugins.push_back(std::make_unique<LoggingPlugin>());

    // Register plugin options
    for (auto &plugin : plugins) {
        plugin->registerOptions(dockingCmd);
    }

    // Parse
    CLI11_PARSE(app, argc, argv);

    // Access global options
    std::cout << "Global Options:\n";
    std::cout << "  file = " << Options::file << "\n";
    std::cout << "  pid  = " << Options::pid << "\n\n";

    // Access plugin options
    std::cout << "Docking Subcommand Options:\n";
    for (auto &plugin : plugins) {
        plugin->printValues();
    }

    return 0;
}
```