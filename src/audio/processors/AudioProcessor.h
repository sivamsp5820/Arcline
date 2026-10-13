#pragma once

namespace arcline::audio {

/**
 * @brief Lightweight, real-time safe audio processing block interface.
 */
class AudioProcessor {
public:
    virtual ~AudioProcessor() = default;

    /**
     * @brief Prepares DSP buffers and state before audio streaming starts.
     * Called on non-audio thread. Allocations permitted here.
     */
    virtual void prepareToPlay(double sampleRate, int maxBlockSize) = 0;

    /**
     * @brief Hard real-time audio block callback.
     * Zero allocations, zero blocking, deterministic execution.
     */
    virtual void processBlock(float* const* outputChannels, int numChannels, int numSamples) = 0;

    /**
     * @brief Releases resources when audio streaming stops.
     */
    virtual void releaseResources() = 0;
};

} // namespace arcline::audio
