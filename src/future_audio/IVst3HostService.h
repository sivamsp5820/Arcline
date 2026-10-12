#pragma once

#include <string>
#include <vector>
#include <memory>

namespace arcline::future_audio {

enum class PluginCategory {
    Effect,
    Instrument,
    Spatializer,
    Analyzer
};

struct PluginDescriptor {
    std::string uniqueId;
    std::string name;
    std::string vendor;
    std::string version;
    std::string path;
    PluginCategory category;
    int audioInputChannels = 2;
    int audioOutputChannels = 2;
    bool hasEditor = true;
};

/**
 * @brief Extension point interface for VST3 plugin hosting and lifecycle management (Phase 6).
 * 
 * Boundaries and Contracts:
 * - Scanning and loading plugins is strictly isolated to background threads to prevent UI freezes.
 * - Audio callback invokes IComponent / IAudioProcessor::process() synchronously within the graph block.
 * - Parameter automation changes from the UI are queued into VST3 IParameterChanges queues without heap allocation.
 * - Plugin state serialization (IBStream chunks) is performed during project save without interrupting real-time audio.
 */
class IVst3HostService {
public:
    virtual ~IVst3HostService() = default;

    virtual std::vector<PluginDescriptor> getScannedPlugins() const = 0;
    virtual void scanPluginsAsync(const std::vector<std::string>& searchPaths) = 0;
    virtual bool isScanning() const noexcept = 0;

    virtual bool instantiatePlugin(const std::string& trackId, int slotIndex, const std::string& pluginId) = 0;
    virtual void removePlugin(const std::string& trackId, int slotIndex) = 0;

    virtual void openPluginEditor(const std::string& trackId, int slotIndex) = 0;
    virtual void closePluginEditor(const std::string& trackId, int slotIndex) = 0;
};

} // namespace arcline::future_audio

namespace sonora::future_audio {
    using PluginCategory = arcline::future_audio::PluginCategory;
    using PluginDescriptor = arcline::future_audio::PluginDescriptor;
    using IVst3HostService = arcline::future_audio::IVst3HostService;
}
