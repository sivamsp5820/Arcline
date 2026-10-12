---
name: "Shared Interface RFC"
about: "Propose a modification or extension to a shared interface header or central architectural contract."
title: "[RFC] <Interface / Header Name>: <Proposal Summary>"
labels: ["rfc", "architecture"]
assignees: []
---

### 1. Interface / Header Affected
- **Header File:** `<!-- e.g. src/app/AppContext.h, src/future_audio/IAudioEngine.h, src/core/CoreDefines.h -->`
- **Architectural Layer:** `<!-- UI / Application / Domain / Audio / Infrastructure -->`

### 2. Motivation & Problem Statement
`<!-- Why is this interface modification necessary? What task or subsystem is currently blocked? -->`

### 3. Proposed Interface Changes (Diff / API Signature)
```cpp
// Proposed changes or additions to the header:
```

### 4. Impact Analysis
- **Affected Subsystems:** `<!-- Which UI panels, audio callbacks, or repositories are affected? -->`
- **Breaking Changes:** `<!-- Does this change existing method signatures or binary compatibility? -->`
- **Thread-Safety Implications:** `<!-- Does this interface cross thread boundaries (e.g. UI to real-time audio)? -->`

### 5. Migration Strategy
`<!-- How will existing implementations or tests be updated? -->`
