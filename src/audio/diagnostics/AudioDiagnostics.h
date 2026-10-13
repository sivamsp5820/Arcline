#pragma once

#include <atomic>
#include <cstdint>
#include <string>

namespace arcline::audio {

/**
 * @brief Thread-safe, lock-free telemetry counters collected during real-time processing.
 */
class AudioDiagnostics {
public:
    struct Snapshot {
        uint64_t totalBuffersProcessed = 0;
        uint64_t totalSamplesProcessed = 0;
        uint64_t xrunCount = 0;
        float peakCallbackTimeUs = 0.0f;
        float lastCallbackTimeUs = 0.0f;
        float cpuUsagePercent = 0.0f;
        uint64_t nanInfDetections = 0;
    };

    AudioDiagnostics() = default;

    // Real-time callback safe methods
    void recordCallback(int numSamples, float durationUs, float bufferBudgetUs, bool hasNanOrInf) noexcept {
        totalBuffersProcessed_.fetch_add(1, std::memory_order_relaxed);
        totalSamplesProcessed_.fetch_add(static_cast<uint64_t>(numSamples), std::memory_order_relaxed);

        lastCallbackTimeUs_.store(durationUs, std::memory_order_relaxed);

        float currentPeak = peakCallbackTimeUs_.load(std::memory_order_relaxed);
        if (durationUs > currentPeak) {
            peakCallbackTimeUs_.store(durationUs, std::memory_order_relaxed);
        }

        if (bufferBudgetUs > 0.0f) {
            float usage = (durationUs / bufferBudgetUs) * 100.0f;
            // Simple exponential smoothing
            float prevUsage = cpuUsagePercent_.load(std::memory_order_relaxed);
            float smoothed = prevUsage * 0.9f + usage * 0.1f;
            cpuUsagePercent_.store(smoothed, std::memory_order_relaxed);

            if (durationUs > bufferBudgetUs) {
                xrunCount_.fetch_add(1, std::memory_order_relaxed);
            }
        }

        if (hasNanOrInf) {
            nanInfDetections_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void recordXrun() noexcept {
        xrunCount_.fetch_add(1, std::memory_order_relaxed);
    }

    void reset() noexcept {
        totalBuffersProcessed_.store(0, std::memory_order_relaxed);
        totalSamplesProcessed_.store(0, std::memory_order_relaxed);
        xrunCount_.store(0, std::memory_order_relaxed);
        peakCallbackTimeUs_.store(0.0f, std::memory_order_relaxed);
        lastCallbackTimeUs_.store(0.0f, std::memory_order_relaxed);
        cpuUsagePercent_.store(0.0f, std::memory_order_relaxed);
        nanInfDetections_.store(0, std::memory_order_relaxed);
    }

    // UI-thread safe reader
    Snapshot getSnapshot() const noexcept {
        Snapshot s;
        s.totalBuffersProcessed = totalBuffersProcessed_.load(std::memory_order_relaxed);
        s.totalSamplesProcessed = totalSamplesProcessed_.load(std::memory_order_relaxed);
        s.xrunCount = xrunCount_.load(std::memory_order_relaxed);
        s.peakCallbackTimeUs = peakCallbackTimeUs_.load(std::memory_order_relaxed);
        s.lastCallbackTimeUs = lastCallbackTimeUs_.load(std::memory_order_relaxed);
        s.cpuUsagePercent = cpuUsagePercent_.load(std::memory_order_relaxed);
        s.nanInfDetections = nanInfDetections_.load(std::memory_order_relaxed);
        return s;
    }

    float getCpuUsagePercent() const noexcept {
        return cpuUsagePercent_.load(std::memory_order_relaxed);
    }

    uint64_t getXrunCount() const noexcept {
        return xrunCount_.load(std::memory_order_relaxed);
    }

private:
    std::atomic<uint64_t> totalBuffersProcessed_{0};
    std::atomic<uint64_t> totalSamplesProcessed_{0};
    std::atomic<uint64_t> xrunCount_{0};
    std::atomic<float> peakCallbackTimeUs_{0.0f};
    std::atomic<float> lastCallbackTimeUs_{0.0f};
    std::atomic<float> cpuUsagePercent_{0.0f};
    std::atomic<uint64_t> nanInfDetections_{0};
};

} // namespace arcline::audio
