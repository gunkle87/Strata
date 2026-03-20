# Strata

Strata is the layered simulation platform that unifies:
- authored structure,
- structural compilation,
- a shared runtime API,
- multiple execution backends.

Current intended backend family:
- `LXS`
  - fast compiled 4-state backend
- `HighZ`
  - temporal richer-state backend

Core doctrine
- `structure is the authoring layer`
- `function is the execution layer`

Primary layer names
- `Breadboard`
  - structural compiler layer
- `Forge`
  - shared runtime API layer

Current repository status
- first executable fast-path phase completed
- pillar-based planning scaffold installed
- broader backend, artifact, and capability pillars still ahead

Initial repo layout
- `docs/`
  - categorized architecture, boundary, reference, notes, and process docs
- `planning/`
  - master pillar plan, per-pillar plans, trackers, and directive templates
- `protocol/`
  - portable development and governance protocols
- `src/authoring/`
  - future authoring-model code
- `src/breadboard/`
  - future structural compiler code
- `src/forge/`
  - future shared runtime API code
- `src/backends/lxs/`
  - future fast backend code
- `src/backends/highz/`
  - future temporal backend code
- `src/common/`
  - future shared low-level support code
- `include/`
  - future exported/public headers
- `tools/`
  - tooling and utility code
- `tests/`
  - tests across layers

Immediate source documents copied from LXS
- combined-engine planning docs
- hierarchy docs
- cross-engine strength matrix
- `docs/reference/PROBE_PROTOCOL.md`
- `docs/reference/TAB_PROTOCOL.md`
- the portable engine-evolution protocol bundle

Documentation entry points
- `docs/README.md`
  - category guide for the documentation tree
- `docs/architecture/COMBINED_ENGINE_ARCHITECTURE_PLAN.md`
  - top-level architecture intent
- `planning/README.md`
  - how the pillar-based planning/build process works
- `planning/pillar_master_plan.md`
  - top-level execution roadmap

