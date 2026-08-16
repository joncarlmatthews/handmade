# macOS Platform Layer

## Supported Builds

- Architectures: `arm64`, `x86_64`, `universal`
- Configurations: `Debug`, `Release`
- Minimum macOS: 15.0

## Build With CLion

Open the repo root as an existing CMake project and run the `HandmadeMac` target.
This builds the C game layer first, then links/runs the macOS app bundle.

## Build Game Dylib Directly

From `Game/`:

```sh
./build.sh arm64 Debug
./build.sh x86_64 Release
./build.sh universal Release
```
