#include "audio/transport/TransportController.h"

namespace arcline::audio {

TransportController::TransportController() {
    clock_.setSampleRate(48000.0);
    clock_.setTempoBpm(120.0);
    clock_.setTimeSignature(4, 4);
}

void TransportController::play() {
    auto current = state_.load(std::memory_order_relaxed);
    if (current != future_audio::TransportState::Playing) {
        state_.store(future_audio::TransportState::Playing, std::memory_order_release);
        notifyStateChanged(future_audio::TransportState::Playing);
    }
}

void TransportController::stop() {
    auto current = state_.load(std::memory_order_relaxed);
    if (current != future_audio::TransportState::Stopped) {
        state_.store(future_audio::TransportState::Stopped, std::memory_order_release);
        notifyStateChanged(future_audio::TransportState::Stopped);
    }
}

void TransportController::record() {
    auto current = state_.load(std::memory_order_relaxed);
    if (current != future_audio::TransportState::Recording) {
        state_.store(future_audio::TransportState::Recording, std::memory_order_release);
        notifyStateChanged(future_audio::TransportState::Recording);
    }
}

void TransportController::pause() {
    auto current = state_.load(std::memory_order_relaxed);
    if (current != future_audio::TransportState::Paused) {
        state_.store(future_audio::TransportState::Paused, std::memory_order_release);
        notifyStateChanged(future_audio::TransportState::Paused);
    }
}

void TransportController::returnToStart() {
    locateToSample(0);
}

void TransportController::locateToSample(int64_t sampleIndex) {
    int64_t s = std::max<int64_t>(0, sampleIndex);
    pendingLocateSample_.store(s, std::memory_order_release);
    // Direct clock update for immediate UI reflection if stopped
    if (state_.load(std::memory_order_relaxed) == future_audio::TransportState::Stopped) {
        clock_.locateToSample(s);
    }
}

void TransportController::locateToTick(int64_t tick) {
    locateToSample(clock_.tickToSample(tick));
}

void TransportController::setLooping(bool loopEnabled) {
    isLooping_.store(loopEnabled, std::memory_order_relaxed);
    clock_.setLooping(loopEnabled);
}

void TransportController::setLoopRange(int64_t startTick, int64_t endTick) {
    clock_.setLoopRange(startTick, endTick);
}

void TransportController::prepareToPlay(double sampleRate, int /*maxBlockSize*/) {
    clock_.setSampleRate(sampleRate);
}

void TransportController::advanceClockInCallback(int numSamples) {
    // Check if an atomic locate was requested
    int64_t pendingLocate = pendingLocateSample_.exchange(-1, std::memory_order_acq_rel);
    if (pendingLocate >= 0) {
        clock_.locateToSample(pendingLocate);
    }

    auto currentState = state_.load(std::memory_order_acquire);
    if (currentState == future_audio::TransportState::Playing || 
        currentState == future_audio::TransportState::Recording) {
        clock_.advanceSamples(numSamples);
    }
}

future_audio::TransportState TransportController::getState() const noexcept {
    return state_.load(std::memory_order_relaxed);
}

bool TransportController::isPlaying() const noexcept {
    auto s = state_.load(std::memory_order_relaxed);
    return s == future_audio::TransportState::Playing || s == future_audio::TransportState::Recording;
}

bool TransportController::isRecording() const noexcept {
    return state_.load(std::memory_order_relaxed) == future_audio::TransportState::Recording;
}

bool TransportController::isLooping() const noexcept {
    return isLooping_.load(std::memory_order_relaxed);
}

int64_t TransportController::getCurrentSamplePosition() const noexcept {
    return clock_.getCurrentSamplePosition();
}

int64_t TransportController::getCurrentTickPosition() const noexcept {
    return clock_.sampleToTick(clock_.getCurrentSamplePosition());
}

double TransportController::getCurrentTimeSeconds() const noexcept {
    double sr = clock_.getSampleRate();
    return sr > 0.0 ? (static_cast<double>(clock_.getCurrentSamplePosition()) / sr) : 0.0;
}

future_audio::MusicalPosition TransportController::getMusicalPosition() const noexcept {
    return clock_.getMusicalPosition();
}

void TransportController::setTempoBpm(double bpm) {
    clock_.setTempoBpm(bpm);
}

double TransportController::getTempoBpm() const noexcept {
    return clock_.getTempoBpm();
}

void TransportController::getTimeSignature(int& numerator, int& denominator) const noexcept {
    numerator = clock_.getTimeSigNumerator();
    denominator = clock_.getTimeSigDenominator();
}

void TransportController::setTimeSignature(int numerator, int denominator) {
    clock_.setTimeSignature(numerator, denominator);
}

int TransportController::getTimeSigNumerator() const noexcept {
    return clock_.getTimeSigNumerator();
}

int TransportController::getTimeSigDenominator() const noexcept {
    return clock_.getTimeSigDenominator();
}

void TransportController::addStateChangeListener(std::function<void(future_audio::TransportState)> listener) {
    if (listener) {
        listeners_.push_back(std::move(listener));
    }
}

void TransportController::notifyStateChanged(future_audio::TransportState newState) {
    for (const auto& listener : listeners_) {
        if (listener) listener(newState);
    }
}

} // namespace arcline::audio
