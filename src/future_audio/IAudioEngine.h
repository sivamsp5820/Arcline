#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace arcline::future_audio {

/**
 * @brief Extension point interface for the future real-time audio engine (Phase 2+).
 * 
 * Strict architectural boundaries for Phase 2 implementation:
 * 1. Hard Real-Time Safety:
 *    - Zero dynamic memory allocation (malloc, free, new, delete) inside the audio callback.
 *    - Zero blocking system calls (file I/O, sockets, IPC, mutex acquisitions with unbounded wait).
 *    - Lock-free, wait-free algorithms only (atomic operations and SPSC queues).
 * 2. Thread Decoupling:
 *    - The audio thread never accesses GUI state or domain models directly.
 *    - State synchronization occurs across lock-free ring buffers (Audio-to-UI and UI-to-Audio).
 * 3. Processing Lifecycle:
 *    - Pre-allocates all audio channel scratch buffers and voice pools during initialize().
 *    - Processes sample blocks deterministically within the hardware buffer window.
 */
class IAudioEngine {
public:
    virtual ~IAudioEngine() = default;

    virtual bool initialize(double sampleRate, int blockSize) = 0;
    virtual void shutdown() = 0;
    virtual bool isRunning() const noexcept = 0;

    virtual double getSampleRate() const noexcept = 0;
    virtual int getBlockSize() const noexcept = 0;
    virtual double getCpuUsagePercent() const noexcept = 0;
};

} // namespace arcline::future_audio

namespace sonora::future_audio {
    using IAudioEngine = arcline::future_audio::IAudioEngine;
}
