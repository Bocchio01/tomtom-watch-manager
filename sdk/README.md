You are asking the *right* question. The tension you describe (by file type vs by capability) is exactly where SDKs either scale cleanly or become unmaintainable.

Given:

* You already have a **clean low-level core** (`core::Watch`, transport, kernel, file ops)
* The SDK must:

  * Orchestrate **device → PC → internet**
  * Understand **heterogeneous formats** (binary ttbin, protobuf, XML)
  * Support **overlapping capabilities** (parse, convert, export, upload)
  * Remain extensible (new file IDs, new exporters, new services)

The correct answer is:
**neither purely “by file type” nor purely “by content type” — but a layered, capability-oriented architecture with file-type modules as vertical slices.**

Below is a concrete, opinionated structure that fits your use case and your existing `Watch` abstraction.

---

## 1. High-level architectural principles

### 1.1 Watch stays *dumb*

Your `core::Watch` is perfect as-is:

* It exposes **capabilities**, not semantics
* It does *not* know what an “activity” or “route” is
* It does not parse formats

This is exactly what you want.

> The SDK **depends on Watch**, never the other way around.

---

### 1.2 Separate **domain objects** from **formats**

You must distinguish:

* **What the data *means*** (Activity, Track, Route, Workout)
* **How it is encoded** (ttbin, protobuf, XML, GPX, KML)

This avoids the classic trap:

> “Activity parser outputs GPX directly”

Instead:

```
ttbin → Activity model → GPX/KML/Strava
protobuf route → Route model → GPX
```

---

### 1.3 Capabilities are orthogonal

Not every file:

* is parsable
* is exportable
* is uploadable
* lives on the watch

So capabilities must be **interfaces**, not assumptions.

---

## 2. Proposed top-level module layout

```text
tomtom/
├── core/              # already exists (transport, protocol, Watch)
├── sdk/
│   ├── device/
│   ├── domain/
│   ├── filesystem/
│   ├── formats/
│   ├── parsers/
│   ├── exporters/
│   ├── services/
│   ├── workflows/
│   └── util/
```

Let’s go through each, concretely.

---

## 3. `sdk/device` – Watch-aware orchestration

**Purpose:** high-level device logic that still talks to the watch

```text
sdk/device/
├── device_context.hpp
├── device_manager.hpp
├── file_catalog.hpp
└── sync_state.hpp
```

### Responsibilities

* Enumerate files by ID and type
* Map file IDs → logical file kinds
* Cache metadata (sizes, timestamps)
* Coordinate download/delete policies

Example:

```cpp
DeviceManager mgr(watch);
auto activities = mgr.list<ActivityFile>();
```

This layer:

* Uses `watch.files()`
* Does **not** parse contents

---

## 4. `sdk/domain` – Pure semantic models (no I/O)

**This is the most important layer.**

```text
sdk/domain/
├── activity/
│   ├── activity.hpp
│   ├── lap.hpp
│   ├── track_point.hpp
│   └── metrics.hpp
├── route/
│   ├── route.hpp
│   └── segment.hpp
├── workout/
│   └── workout.hpp
├── device/
│   └── device_info.hpp
└── common/
    ├── time.hpp
    ├── geo.hpp
    └── units.hpp
```

### Key rules

* **No protobuf**
* **No ttbin**
* **No XML**
* **No Watch**

Only:

* C++ structs / classes
* Domain invariants
* Validation

This allows:

* Unit testing without hardware
* Reuse across formats and services

---

## 5. `sdk/filesystem` – PC-side file management

```text
sdk/filesystem/
├── paths.hpp
├── local_store.hpp
├── cache.hpp
└── naming.hpp
```

Responsibilities:

* Decide where `.ttbin`, `.proto`, `.gpx` live
* Mirror TomTom Sports Connect layout (if desired)
* Provide stable paths to other modules

This avoids polluting exporters/parsers with filesystem logic.

---

## 6. `sdk/formats` – File-format definitions only

This is *schema-level* knowledge.

```text
sdk/formats/
├── ttbin/
│   ├── ttbin_defs.hpp
│   ├── record_tags.hpp
│   └── record_layouts.hpp
├── protobuf/
│   ├── activity.proto
│   ├── route.proto
│   └── workout.proto
├── xml/
│   └── preferences_schema.hpp
└── ids/
    └── file_ids.hpp
```

Responsibilities:

* Constants
* Struct layouts
* Generated protobuf bindings
* No logic

This is where your massive reverse-engineering document belongs conceptually.

---

## 7. `sdk/parsers` – Format → Domain

```text
sdk/parsers/
├── activity/
│   ├── ttbin_activity_parser.hpp
│   └── ttbin_activity_parser.cpp
├── route/
│   ├── protobuf_route_parser.hpp
│   └── protobuf_route_parser.cpp
├── tracking/
│   └── protobuf_tracking_parser.hpp
├── preferences/
│   └── xml_preferences_parser.hpp
└── common/
    └── parse_error.hpp
```

Each parser:

* Takes **bytes**
* Emits **domain objects**
* Knows **exactly one format**

Example:

```cpp
Activity parse_ttbin(const std::vector<uint8_t>& data);
```

No exporting. No uploading.

---

## 8. `sdk/exporters` – Domain → external formats

```text
sdk/exporters/
├── gpx/
│   ├── gpx_activity_exporter.hpp
│   └── gpx_route_exporter.hpp
├── kml/
│   └── kml_route_exporter.hpp
├── json/
│   └── activity_json_exporter.hpp
└── common/
    └── export_options.hpp
```

Important:

* **Exporters operate on domain models**
* Activity and Route share exporters where possible
* Different exporters can coexist for the same domain

---

## 9. `sdk/services` – External integrations (Strava, etc.)

```text
sdk/services/
├── strava/
│   ├── strava_client.hpp
│   ├── strava_uploader.hpp
│   └── auth.hpp
├── tomtom/
│   └── quickfix_service.hpp
└── common/
    └── http_client.hpp
```

Strava uploader example:

```cpp
void upload(const Activity&, const StravaCredentials&);
```

It does **not care** whether the Activity came from:

* ttbin
* protobuf
* GPX import

---

## 10. `sdk/workflows` – End-to-end orchestration

This is where everything comes together.

```text
sdk/workflows/
├── download_activities.hpp
├── export_activities.hpp
├── sync_strava.hpp
├── update_quickfix.hpp
└── firmware_update.hpp
```

Example:

```cpp
sync_strava(watch, credentials);
```

Internally:

1. Enumerates activity files
2. Downloads
3. Parses ttbin → Activity
4. Uploads Activity → Strava
5. Deletes file (optional)

This keeps *policy* out of parsers and exporters.

---

## 11. Why this structure works for your specific problems

### Your concern | Where it is solved

---|---
Activities and routes both need exporters | `domain + exporters`
Different parsers per file | `parsers/*`
Some files don’t need parsing | `device + filesystem`
Some files aren’t exportable | absence of exporter
Strava upload | `services`
USB stays low-level | `core::Watch`
Future BLE support | reuse SDK unchanged

---

## 12. Mental model to keep

> **Watch → bytes → parser → domain → exporter/service**

If a module violates this flow, it is probably misplaced.

---

If you want, next we can:

* Design **base interfaces** (`Parser<T>`, `Exporter<T>`)
* Define **file ID → parser registry**
* Walk through **one full activity pipeline in code**
* Review this structure against an existing SDK (Garmin, FIT SDK, etc.)

This is a solid foundation for a serious, long-lived SDK.
