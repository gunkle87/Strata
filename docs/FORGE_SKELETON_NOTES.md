# Forge Skeleton Implementation Notes

Status
- Skeleton only. No real execution is implemented.

## What Is In This Pass

Files added:

```
include/forge_result.h
include/forge_types.h
include/forge_capabilities.h
include/forge_api.h
src/forge/forge_registry.c
src/forge/forge_api.c
src/forge/forge_internal.h
src/common/forge_diagnostic.h
src/common/forge_diagnostic.c
tests/forge/test_backend_enum.c
tests/forge/test_backend_info.c
tests/forge/test_capabilities.c
tests/forge/test_invalid_handle.c
tests/forge/test_artifact_stub.c
tests/forge/test_session_lifecycle.c
tests/forge/Makefile
```

## What Is Not In This Pass

- No real artifact decoding. `forge_artifact_load` only accepts a 4-byte
  stub magic sequence (`0x53 0x54 0x42 0x21`, i.e. `STB!`). All other data
  returns `FORGE_ERR_UNSUPPORTED`.
- No real session execution. `forge_session_create` always returns
  `FORGE_ERR_UNSUPPORTED`. Session reset and free reject NULL handles.
- No `Breadboard` code.
- No backend execution code.
- No `LXS` or `HighZ` internal coupling.

## How To Build And Test

```
cd C:\DEV\Strata\tests\forge
make clean
make
make test
```

All 6 test executables print `PASS: <name>` and exit 0.

To test with warnings as errors:

```
make CFLAGS="-Wall -Wextra -Werror -std=c99"
```

## How To Add A Backend Record

1. Open `src/forge/forge_registry.c`.
2. Add a new `#define FORGE_BACKEND_ID_<NAME>` constant with the next
   sequential non-zero value.
3. Append a new `ForgeBackendRecord` entry to `s_backends[]`.
4. Extend the capability payload inside the new `ForgeBackendRecord`.

No other files need changing for a basic stub backend addition.

## Backend ID Assignments

| Backend | ID |
|---------|----|
| LXS     | 1  |
| HighZ   | 2  |
