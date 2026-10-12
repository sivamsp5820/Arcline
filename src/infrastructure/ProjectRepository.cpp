#include "infrastructure/ProjectRepository.h"
#include "infrastructure/FileSystem.h"
#include <juce_core/juce_core.h>

namespace sonora::infrastructure {

std::string ProjectRepository::serializeToJson(const domain::Project& project) {
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty("format", juce::String(std::string(kProjectFormatId)));
    root->setProperty("schemaVersion", project.getSchemaVersion());
    root->setProperty("id", juce::String(project.getId()));
    root->setProperty("name", juce::String(project.getName()));
    root->setProperty("sampleRate", project.getSampleRate());
    root->setProperty("tempoBpm", project.getTempoBpm());
    root->setProperty("timeSigNumerator", project.getTimeSigNumerator());
    root->setProperty("timeSigDenominator", project.getTimeSigDenominator());
    root->setProperty("keySignature", juce::String(project.getKeySignature()));
    root->setProperty("createdTimestamp", static_cast<juce::int64>(project.getCreatedTimestamp()));
    root->setProperty("modifiedTimestamp", static_cast<juce::int64>(project.getModifiedTimestamp()));

    // Transport state
    root->setProperty("playheadTick", static_cast<juce::int64>(project.getPlayheadTick()));
    root->setProperty("cycleStartTick", static_cast<juce::int64>(project.getCycleStartTick()));
    root->setProperty("cycleEndTick", static_cast<juce::int64>(project.getCycleEndTick()));
    root->setProperty("isCycleActive", project.isCycleActive());
    root->setProperty("masterVolume", project.getMasterVolume());
    root->setProperty("masterPan", project.getMasterPan());

    // Tracks array
    juce::Array<juce::var> tracksArray;
    for (const auto& track : project.getTracks()) {
        if (!track) continue;
        juce::DynamicObject::Ptr tObj = new juce::DynamicObject();
        tObj->setProperty("id", juce::String(track->getId()));
        tObj->setProperty("name", juce::String(track->getName()));
        tObj->setProperty("type", static_cast<int>(track->getType()));
        tObj->setProperty("volume", track->getVolume());
        tObj->setProperty("pan", track->getPan());
        tObj->setProperty("isMuted", track->isMuted());
        tObj->setProperty("isSoloed", track->isSoloed());
        tObj->setProperty("isArmed", track->isArmed());
        tObj->setProperty("isInputMonitored", track->isInputMonitored());
        tObj->setProperty("automationMode", static_cast<int>(track->getAutomationMode()));
        tObj->setProperty("colorArgb", static_cast<juce::int64>(track->getColor()));
        tObj->setProperty("height", track->getHeight());

        // Regions array for track
        juce::Array<juce::var> regionsArray;
        for (const auto& region : track->getRegions()) {
            if (!region) continue;
            juce::DynamicObject::Ptr rObj = new juce::DynamicObject();
            rObj->setProperty("id", juce::String(region->getId()));
            rObj->setProperty("name", juce::String(region->getName()));
            rObj->setProperty("type", static_cast<int>(region->getType()));
            rObj->setProperty("startTick", static_cast<juce::int64>(region->getStartTick()));
            rObj->setProperty("lengthTicks", static_cast<juce::int64>(region->getLengthTicks()));
            rObj->setProperty("offsetTicks", static_cast<juce::int64>(region->getOffsetTicks()));
            rObj->setProperty("isMuted", region->isMuted());
            rObj->setProperty("isLooped", region->isLooped());
            rObj->setProperty("colorArgb", static_cast<juce::int64>(region->getColor()));
            rObj->setProperty("gainDb", region->getGainDb());
            rObj->setProperty("fadeInTicks", static_cast<juce::int64>(region->getFadeInTicks()));
            rObj->setProperty("fadeOutTicks", static_cast<juce::int64>(region->getFadeOutTicks()));

            if (region->getType() == domain::RegionType::Audio) {
                if (!region->getAudioFilePath().empty()) {
                    std::string projectDir = FileSystem::getParentDirectory(project.getFilePath());
                    std::string relPath = FileSystem::makeRelative(projectDir, region->getAudioFilePath());
                    rObj->setProperty("audioFilePath", juce::String(FileSystem::toPosixPath(relPath)));
                }
                juce::Array<juce::var> peakArray;
                for (const auto& p : region->getWaveformPeaks()) {
                    juce::Array<juce::var> pt;
                    pt.add(p.first);
                    pt.add(p.second);
                    peakArray.add(juce::var(pt));
                }
                rObj->setProperty("peaks", peakArray);
            } else if (region->getType() == domain::RegionType::Midi) {
                juce::Array<juce::var> notesArray;
                for (const auto& n : region->getNotes()) {
                    juce::DynamicObject::Ptr nObj = new juce::DynamicObject();
                    nObj->setProperty("pitch", n.pitch);
                    nObj->setProperty("startTick", static_cast<juce::int64>(n.startTick));
                    nObj->setProperty("lengthTicks", static_cast<juce::int64>(n.lengthTicks));
                    nObj->setProperty("velocity", n.velocity);
                    notesArray.add(juce::var(nObj.get()));
                }
                rObj->setProperty("notes", notesArray);
            }
            regionsArray.add(juce::var(rObj.get()));
        }
        tObj->setProperty("regions", regionsArray);
        tracksArray.add(juce::var(tObj.get()));
    }
    root->setProperty("tracks", tracksArray);

    return juce::JSON::toString(juce::var(root.get()), false).toStdString();
}

std::optional<domain::Project> ProjectRepository::deserializeFromJson(const std::string& jsonString, std::string& outError) {
    if (jsonString.empty()) {
        outError = "Project file is empty.";
        return std::nullopt;
    }

    auto parsed = juce::JSON::parse(juce::String(jsonString));
    if (parsed.isVoid() || !parsed.isObject()) {
        outError = "Malformed JSON syntax in project file.";
        return std::nullopt;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj) {
        outError = "Invalid project object structure.";
        return std::nullopt;
    }

    if (!obj->hasProperty("format")) {
        outError = "Malformed project file: missing format identifier.";
        return std::nullopt;
    }

    juce::String format = obj->getProperty("format").toString();
    if (format != kProjectFormatId && format != kLegacyProjectFormatId) {
        outError = "Not a valid ARCLINE project file (unsupported format: '" + format.toStdString() + "').";
        return std::nullopt;
    }

    if (!obj->hasProperty("schemaVersion")) {
        outError = "Malformed project file: missing schemaVersion property.";
        return std::nullopt;
    }

    int schema = static_cast<int>(obj->getProperty("schemaVersion"));
    if (schema <= 0) {
        outError = "Invalid project schema version (" + std::to_string(schema) + ").";
        return std::nullopt;
    }
    if (schema > domain::Project::kCurrentSchemaVersion) {
        outError = "Unsupported project schema version (" + std::to_string(schema) + 
                   "). Maximum supported version is " + std::to_string(domain::Project::kCurrentSchemaVersion) + 
                   ". Please upgrade ARCLINE.";
        return std::nullopt;
    }

    if (!obj->hasProperty("name") || obj->getProperty("name").toString().trim().isEmpty()) {
        outError = "Malformed project file: missing or empty project name.";
        return std::nullopt;
    }

    std::string name = obj->getProperty("name").toString().toStdString();
    domain::Project project;
    project.setId(obj->hasProperty("id") ? obj->getProperty("id").toString().toStdString() : "proj_migrated");
    project.setName(name);
    project.setSchemaVersion(schema);

    double sampleRate = obj->hasProperty("sampleRate") ? static_cast<double>(obj->getProperty("sampleRate")) : 48000.0;
    if (sampleRate < 8000.0 || sampleRate > 384000.0) {
        outError = "Invalid sample rate in project: " + std::to_string(sampleRate);
        return std::nullopt;
    }
    project.setSampleRate(sampleRate);

    double tempoBpm = obj->hasProperty("tempoBpm") ? static_cast<double>(obj->getProperty("tempoBpm")) : 120.0;
    if (tempoBpm < domain::Project::kMinTempoBpm || tempoBpm > domain::Project::kMaxTempoBpm) {
        outError = "Project tempo out of valid range (20 - 999 BPM): " + std::to_string(tempoBpm);
        return std::nullopt;
    }
    project.setTempoBpm(tempoBpm);

    int num = obj->hasProperty("timeSigNumerator") ? static_cast<int>(obj->getProperty("timeSigNumerator")) : 4;
    int den = obj->hasProperty("timeSigDenominator") ? static_cast<int>(obj->getProperty("timeSigDenominator")) : 4;
    if (num < 1 || num > 32 || den < 1 || den > 32) {
        outError = "Project has invalid time signature: " + std::to_string(num) + "/" + std::to_string(den);
        return std::nullopt;
    }
    project.setTimeSignature(num, den);

    if (obj->hasProperty("keySignature")) {
        project.setKeySignature(obj->getProperty("keySignature").toString().toStdString());
    }

    if (obj->hasProperty("createdTimestamp")) {
        project.setCreatedTimestamp(static_cast<int64_t>(obj->getProperty("createdTimestamp")));
    }
    if (obj->hasProperty("modifiedTimestamp")) {
        project.setModifiedTimestamp(static_cast<int64_t>(obj->getProperty("modifiedTimestamp")));
    }

    if (obj->hasProperty("playheadTick")) {
        project.setPlayheadTick(static_cast<int64_t>(obj->getProperty("playheadTick")));
    }
    if (obj->hasProperty("cycleStartTick") && obj->hasProperty("cycleEndTick")) {
        project.setCycleRange(static_cast<int64_t>(obj->getProperty("cycleStartTick")),
                              static_cast<int64_t>(obj->getProperty("cycleEndTick")));
    }
    if (obj->hasProperty("isCycleActive")) {
        project.setCycleActive(static_cast<bool>(obj->getProperty("isCycleActive")));
    }
    if (obj->hasProperty("masterVolume")) {
        project.setMasterVolume(static_cast<float>(obj->getProperty("masterVolume")));
    }
    if (obj->hasProperty("masterPan")) {
        project.setMasterPan(static_cast<float>(obj->getProperty("masterPan")));
    }

    // Tracks array (Schema v2)
    if (obj->hasProperty("tracks")) {
        auto* tracksArray = obj->getProperty("tracks").getArray();
        if (!tracksArray) {
            outError = "Malformed project file: 'tracks' property must be a JSON array.";
            return std::nullopt;
        }

        for (int ti = 0; ti < tracksArray->size(); ++ti) {
            auto* tObj = (*tracksArray)[ti].getDynamicObject();
            if (!tObj) {
                outError = "Malformed track entry at index " + std::to_string(ti);
                return std::nullopt;
            }

            std::string tId = tObj->hasProperty("id") ? tObj->getProperty("id").toString().toStdString() : "track_" + std::to_string(ti);
            std::string tName = tObj->hasProperty("name") ? tObj->getProperty("name").toString().toStdString() : "Track " + std::to_string(ti + 1);
            
            int tTypeVal = static_cast<int>(tObj->getProperty("type"));
            if (tTypeVal < 0 || tTypeVal > 2) {
                outError = "Unknown track type identifier: " + std::to_string(tTypeVal);
                return std::nullopt;
            }
            auto tType = static_cast<domain::TrackType>(tTypeVal);

            auto track = std::make_shared<domain::Track>(tId, tName, tType);
            track->setVolume(static_cast<float>(tObj->getProperty("volume")));
            track->setPan(static_cast<float>(tObj->getProperty("pan")));
            track->setMuted(static_cast<bool>(tObj->getProperty("isMuted")));
            track->setSoloed(static_cast<bool>(tObj->getProperty("isSoloed")));
            track->setArmed(static_cast<bool>(tObj->getProperty("isArmed")));
            track->setInputMonitored(static_cast<bool>(tObj->getProperty("isInputMonitored")));
            track->setAutomationMode(static_cast<domain::AutomationMode>(static_cast<int>(tObj->getProperty("automationMode"))));
            if (tObj->hasProperty("colorArgb")) {
                track->setColor(static_cast<uint32_t>(static_cast<juce::int64>(tObj->getProperty("colorArgb"))));
            }
            if (tObj->hasProperty("height")) {
                track->setHeight(static_cast<int>(tObj->getProperty("height")));
            }

            // Regions
            if (tObj->hasProperty("regions")) {
                auto* regionsArray = tObj->getProperty("regions").getArray();
                if (!regionsArray) {
                    outError = "Malformed regions list in track '" + tName + "'.";
                    return std::nullopt;
                }

                for (int ri = 0; ri < regionsArray->size(); ++ri) {
                    auto* rObj = (*regionsArray)[ri].getDynamicObject();
                    if (!rObj) {
                        outError = "Malformed region entry in track '" + tName + "'";
                        return std::nullopt;
                    }

                    std::string rId = rObj->hasProperty("id") ? rObj->getProperty("id").toString().toStdString() : "reg_" + std::to_string(ri);
                    std::string rName = rObj->hasProperty("name") ? rObj->getProperty("name").toString().toStdString() : "Region " + std::to_string(ri + 1);
                    
                    int rTypeVal = static_cast<int>(rObj->getProperty("type"));
                    if (rTypeVal < 0 || rTypeVal > 1) {
                        outError = "Unknown region type identifier: " + std::to_string(rTypeVal);
                        return std::nullopt;
                    }
                    auto rType = static_cast<domain::RegionType>(rTypeVal);

                    int64_t startTick = static_cast<int64_t>(rObj->getProperty("startTick"));
                    int64_t lengthTicks = static_cast<int64_t>(rObj->getProperty("lengthTicks"));
                    if (startTick < 0 || lengthTicks <= 0) {
                        outError = "Invalid region bounds in track '" + tName + "': start=" + 
                                   std::to_string(startTick) + ", length=" + std::to_string(lengthTicks);
                        return std::nullopt;
                    }

                    auto region = std::make_shared<domain::Region>(rId, tId, rName, rType, startTick, lengthTicks);
                    region->setOffsetTicks(static_cast<int64_t>(rObj->getProperty("offsetTicks")));
                    region->setMuted(static_cast<bool>(rObj->getProperty("isMuted")));
                    region->setLooped(static_cast<bool>(rObj->getProperty("isLooped")));
                    if (rObj->hasProperty("colorArgb")) {
                        region->setColor(static_cast<uint32_t>(static_cast<juce::int64>(rObj->getProperty("colorArgb"))));
                    }
                    region->setGainDb(static_cast<float>(rObj->getProperty("gainDb")));
                    region->setFadeInTicks(static_cast<int64_t>(rObj->getProperty("fadeInTicks")));
                    region->setFadeOutTicks(static_cast<int64_t>(rObj->getProperty("fadeOutTicks")));

                    if (rType == domain::RegionType::Audio) {
                        if (rObj->hasProperty("audioFilePath")) {
                            std::string rawPath = rObj->getProperty("audioFilePath").toString().toStdString();
                            region->setAudioFilePath(FileSystem::toPosixPath(rawPath));
                        }
                        if (rObj->hasProperty("peaks")) {
                            auto* peaksArray = rObj->getProperty("peaks").getArray();
                            if (peaksArray) {
                                std::vector<std::pair<float, float>> peaks;
                                peaks.reserve(peaksArray->size());
                                for (int pi = 0; pi < peaksArray->size(); ++pi) {
                                    auto* pt = (*peaksArray)[pi].getArray();
                                    if (pt && pt->size() >= 2) {
                                        peaks.emplace_back(static_cast<float>((*pt)[0]),
                                                           static_cast<float>((*pt)[1]));
                                    }
                                }
                                region->setWaveformPeaks(std::move(peaks));
                            }
                        }
                    } else if (rType == domain::RegionType::Midi && rObj->hasProperty("notes")) {
                        auto* notesArray = rObj->getProperty("notes").getArray();
                        if (notesArray) {
                            std::vector<domain::MidiNote> notes;
                            notes.reserve(notesArray->size());
                            for (int ni = 0; ni < notesArray->size(); ++ni) {
                                auto* nObj = (*notesArray)[ni].getDynamicObject();
                                if (!nObj) continue;
                                domain::MidiNote note;
                                note.pitch = static_cast<int>(nObj->getProperty("pitch"));
                                note.startTick = static_cast<int64_t>(nObj->getProperty("startTick"));
                                note.lengthTicks = static_cast<int64_t>(nObj->getProperty("lengthTicks"));
                                note.velocity = static_cast<int>(nObj->getProperty("velocity"));
                                notes.push_back(note);
                            }
                            region->setNotes(std::move(notes));
                        }
                    }
                    track->addRegion(std::move(region));
                }
            }
            project.getTracks().push_back(track);
        }
    }

    if (!project.validate(outError)) {
        return std::nullopt;
    }

    return project;
}

bool ProjectRepository::saveToFile(const domain::Project& project, const std::string& filePath, std::string& outError) {
    if (filePath.empty()) {
        outError = "Target project file path cannot be empty.";
        return false;
    }

    if (!project.validate(outError)) {
        return false;
    }

    std::string json = serializeToJson(project);
    if (!FileSystem::writeTextAtomic(filePath, json)) {
        outError = "Failed to write project data to destination file: " + filePath;
        return false;
    }
    return true;
}

std::optional<domain::Project> ProjectRepository::loadFromFile(const std::string& filePath, std::string& outError) {
    if (filePath.empty()) {
        outError = "Project file path is empty.";
        return std::nullopt;
    }

    if (!FileSystem::fileExists(filePath)) {
        outError = "Project file does not exist: " + filePath;
        return std::nullopt;
    }

    bool success = false;
    std::string content = FileSystem::readTextFile(filePath, success);
    if (!success) {
        outError = "Failed to read project file from disk: " + filePath;
        return std::nullopt;
    }

    auto project = deserializeFromJson(content, outError);
    if (project) {
        project->setFilePath(filePath);
        std::string projectDir = FileSystem::getParentDirectory(filePath);
        for (auto& track : project->getTracks()) {
            for (auto& region : track->getRegions()) {
                if (region->getType() == domain::RegionType::Audio && !region->getAudioFilePath().empty()) {
                    region->setAudioFilePath(FileSystem::resolveRelative(projectDir, region->getAudioFilePath()));
                }
            }
        }
    }
    return project;
}

bool ProjectRepository::isValidProjectFile(const std::string& filePath) {
    std::string err;
    return loadFromFile(filePath, err).has_value();
}

} // namespace sonora::infrastructure
