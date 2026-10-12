#pragma once

#include <string_view>

namespace sonora::domain {

enum class WorkspaceType {
    StartStudio,
    Arrangement
};

inline std::string_view getWorkspaceName(WorkspaceType type) noexcept {
    switch (type) {
        case WorkspaceType::StartStudio: return "Start Studio";
        case WorkspaceType::Arrangement: return "Arrangement";
    }
    return "Unknown";
}

inline bool isWorkspaceImplementedInPhase1(WorkspaceType type) noexcept {
    return type == WorkspaceType::StartStudio || 
           type == WorkspaceType::Arrangement;
}

} // namespace sonora::domain
