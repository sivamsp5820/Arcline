#include "domain/Project.h"
#include <cassert>
#include <iostream>

namespace sonora::tests {

void runProjectTests() {
    std::cout << "[TEST] Running ProjectTests...\n";

    // 1. Creation with defaults
    domain::Project p("Synth Wave", "/path/to/project.sonora");
    assert(p.getName() == "Synth Wave");
    assert(p.getFilePath() == "/path/to/project.sonora");
    assert(!p.getId().empty());
    assert(p.getTempoBpm() == 120.0);
    assert(p.getTimeSigNumerator() == 4);
    assert(p.getTimeSigDenominator() == 4);
    assert(p.isValid());

    // 2. Tempo boundaries
    assert(p.setTempoBpm(140.0));
    assert(p.getTempoBpm() == 140.0);
    assert(!p.setTempoBpm(10.0));  // below min (20.0)
    assert(!p.setTempoBpm(1200.0)); // above max (999.0)

    // 3. Time signature validation
    assert(p.setTimeSignature(3, 4));
    assert(p.getTimeSigNumerator() == 3);
    assert(p.getTimeSigDenominator() == 4);
    assert(!p.setTimeSignature(0, 4));
    assert(!p.setTimeSignature(4, 5)); // invalid denominator

    // 4. Empty name validation
    domain::Project invalidProj("", "/path/to/invalid");
    std::string err;
    assert(!invalidProj.validate(err));
    assert(!err.empty());

    // 5. Track and Region management
    auto audioTrack = p.addTrack(domain::TrackType::Audio, "Lead Vocal");
    assert(audioTrack != nullptr);
    assert(audioTrack->getName() == "Lead Vocal");
    assert(audioTrack->getType() == domain::TrackType::Audio);
    assert(p.getTracks().size() == 1);

    auto audioReg = std::make_shared<domain::Region>("reg_01", audioTrack->getId(), "Vocal Verse",
                                                     domain::RegionType::Audio, 0, 960 * 16);
    audioReg->generateDemoWaveform(100, 3.0f);
    assert(audioReg->getWaveformPeaks().size() == 100);
    audioTrack->addRegion(audioReg);
    assert(audioTrack->getRegions().size() == 1);

    // 6. MIDI Track and Notes
    auto midiTrack = p.addTrack(domain::TrackType::SoftwareInstrument, "Grand Piano");
    assert(p.getTracks().size() == 2);
    auto midiReg = std::make_shared<domain::Region>("reg_02", midiTrack->getId(), "Piano Chords",
                                                    domain::RegionType::Midi, 0, 960 * 16);
    midiReg->addNote({60, 0, 960, 100});
    midiReg->addNote({64, 0, 960, 90});
    midiReg->addNote({67, 0, 960, 95});
    assert(midiReg->getNotes().size() == 3);
    midiTrack->addRegion(midiReg);

    // 7. Selection and splitting
    p.selectRegion("reg_01");
    assert(p.getSelectedRegion() == audioReg);
    p.setPlayheadTick(960 * 4);
    auto rightHalf = p.splitSelectedRegionAtPlayhead();
    assert(rightHalf != nullptr);
    assert(audioTrack->getRegions().size() == 2);
    assert(audioReg->getLengthTicks() == 960 * 4);
    assert(rightHalf->getStartTick() == 960 * 4);

    // 8. Default Logic arrangement initialization
    domain::Project demoProj("Logic Demo", "/path/demo.sonora");
    demoProj.initializeDefaultArrangement();
    assert(demoProj.getTracks().size() == 4);
    assert(demoProj.getSelectedTrack() != nullptr);
    assert(demoProj.getSelectedRegion() != nullptr);

    std::cout << "  ✓ All ProjectTests passed.\n";
}

} // namespace sonora::tests
