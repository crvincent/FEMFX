# Build

```sh
cmake -S . -B build -DFEMFX_BACKEND=SSE2
cmake --build build
cmake --install build --prefix <install-directory>
```

Backend values are `SCALAR`, `SSE2`, and `AVX2`.

`AUTO` selects SCALAR for ARM64 targets and SSE2 for x86/x64 targets.

`NEON` unimplemented.

## Presets

`windows-x64 windows-arm64 linux-x64 linux-arm64 macos-arm64`

```sh
cmake --preset preset
cmake --build --preset preset
```