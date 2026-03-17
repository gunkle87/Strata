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
- bootstrap phase
- planning and protocol baseline installed
- backend code migration has not started yet

Initial repo layout
- `docs/`
  - architecture and planning documents
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
- `PROBE_PROTOCOL.md`
- `TAB_PROTOCOL.md`
- the portable engine-evolution protocol bundle

The next intended major document is:
- `STRATA_FORGE_RUNTIME_API_PLAN.md`
