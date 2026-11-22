# Build

Google Highway is pinned as a Git submodule. Initialize it after cloning:

```sh
git submodule update --init --recursive
```

Highway is compiled into the FEMFX static library; consumers do not need to
link Highway separately.

```sh
cmake -S . -B build -DFEMFX_BACKEND=RUNTIME
cmake --build build
cmake --install build --prefix <install-directory>
```

Backend values are `RUNTIME` and `SCALAR`. `RUNTIME` uses Highway dynamic
dispatch for SSE2/AVX2 on x86/x64 and NEON on ARM64. `SCALAR` retains the
portable reference implementation for comparison and debugging.

## Presets

`windows-x64 windows-arm64 linux-x64 linux-arm64 macos-arm64`

```sh
cmake --preset preset
cmake --build --preset preset
```