#include "domain/Track.h"
#include <algorithm>

namespace sonora::domain {

Track::Track(std::string id, std::string name, TrackType type)
    : id_(std::move(id)),
      name_(std::move(name)),
      type_(type) {
    if (type_ == TrackType::Audio) {
        colorArgb_ = 0xff2e86ab; // Electric Blue
    } else if (type_ == TrackType::SoftwareInstrument) {
        colorArgb_ = 0xff3a7d44; // Forest Green
    } else {
        colorArgb_ = 0xff705898; // Slate Violet
    }
}

void Track::setHeight(int h) noexcept {
    if (h < kMinTrackHeight) h = kMinTrackHeight;
    if (h > kMaxTrackHeight) h = kMaxTrackHeight;
    height_ = h;
}

void Track::addRegion(std::shared_ptr<Region> region) {
    if (region) {
        region->setTrackId(id_);
        regions_.push_back(std::move(region));
    }
}

bool Track::removeRegion(const std::string& regionId) {
    auto it = std::find_if(regions_.begin(), regions_.end(),
        [&regionId](const std::shared_ptr<Region>& r) {
            return r && r->getId() == regionId;
        });
    if (it != regions_.end()) {
        regions_.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<Region> Track::getRegion(const std::string& regionId) const {
    for (const auto& r : regions_) {
        if (r && r->getId() == regionId) {
            return r;
        }
    }
    return nullptr;
}

std::shared_ptr<Region> Track::splitRegion(const std::string& regionId, int64_t splitTick,
                                          const std::string& newRegionId) {
    auto region = getRegion(regionId);
    if (!region) return nullptr;

    auto rightPart = region->split(splitTick, newRegionId);
    if (rightPart) {
        regions_.push_back(rightPart);
    }
    return rightPart;
}

bool Track::isValid() const noexcept {
    return !id_.empty() && !name_.empty();
}

} // namespace sonora::domain
