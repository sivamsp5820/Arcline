#pragma once

#include <string_view>
#include <string>

namespace arcline::audio {

/**
 * @brief Explicit lifecycle states of the ARCLINE audio processing engine.
 */
enum class AudioEngineState {
    Uninitialized,       ///< Initial state before audio subsystem setup.
    Initializing,        ///< Subsystem resources and scratch buffers are being configured.
    DeviceReady,         ///< Audio hardware opened and quiescent (waiting for stream).
    ProcessingRunning,   ///< Real-time audio rendering callback is active and processing buffers.
    Reconfiguring,       ///< Device parameters (sample rate, buffer size, channel layout) are changing.
    DeviceUnavailable,   ///< Selected audio hardware is disconnected, lost, or rejected by OS.
    RecoverableError,    ///< Non-fatal error captured; engine is in safe fallback mode.
    ShuttingDown         ///< Teardown in progress; callbacks quiesced and releasing resources.
};

/**
 * @brief Convert AudioEngineState to human-readable string.
 */
inline std::string_view toString(AudioEngineState state) noexcept {
    switch (state) {
        case AudioEngineState::Uninitialized:     return "Uninitialized";
        case AudioEngineState::Initializing:      return "Initializing";
        case AudioEngineState::DeviceReady:       return "Device Ready";
        case AudioEngineState::ProcessingRunning: return "Running";
        case AudioEngineState::Reconfiguring:     return "Reconfiguring";
        case AudioEngineState::DeviceUnavailable: return "Device Unavailable";
        case AudioEngineState::RecoverableError:  return "Recoverable Error";
        case AudioEngineState::ShuttingDown:      return "Shutting Down";
    }
    return "Unknown";
}

/**
 * @brief Enforce valid lifecycle transitions to prevent dangling callbacks or invalid operations.
 */
inline bool isValidTransition(AudioEngineState from, AudioEngineState to) noexcept {
    if (from == to) return true;

    switch (from) {
        case AudioEngineState::Uninitialized:
            return to == AudioEngineState::Initializing || to == AudioEngineState::DeviceUnavailable;

        case AudioEngineState::Initializing:
            return to == AudioEngineState::DeviceReady || 
                   to == AudioEngineState::DeviceUnavailable || 
                   to == AudioEngineState::RecoverableError;

        case AudioEngineState::DeviceReady:
            return to == AudioEngineState::ProcessingRunning || 
                   to == AudioEngineState::Reconfiguring || 
                   to == AudioEngineState::DeviceUnavailable || 
                   to == AudioEngineState::ShuttingDown;

        case AudioEngineState::ProcessingRunning:
            return to == AudioEngineState::DeviceReady || 
                   to == AudioEngineState::Reconfiguring || 
                   to == AudioEngineState::DeviceUnavailable || 
                   to == AudioEngineState::RecoverableError || 
                   to == AudioEngineState::ShuttingDown;

        case AudioEngineState::Reconfiguring:
            return to == AudioEngineState::DeviceReady || 
                   to == AudioEngineState::ProcessingRunning || 
                   to == AudioEngineState::DeviceUnavailable || 
                   to == AudioEngineState::RecoverableError || 
                   to == AudioEngineState::ShuttingDown;

        case AudioEngineState::DeviceUnavailable:
            return to == AudioEngineState::Initializing || 
                   to == AudioEngineState::Reconfiguring || 
                   to == AudioEngineState::ShuttingDown;

        case AudioEngineState::RecoverableError:
            return to == AudioEngineState::DeviceReady || 
                   to == AudioEngineState::Reconfiguring || 
                   to == AudioEngineState::ShuttingDown;

        case AudioEngineState::ShuttingDown:
            return to == AudioEngineState::Uninitialized;
    }

    return false;
}

} // namespace arcline::audio
