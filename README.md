# scenelab

## Building

### Linux / macOS

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j
./scenelab
```

### Windows (PowerShell)

```powershell
mkdir build; cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
./scenelab.exe
```

**Note for Windows:** This project uses MinGW-w64. If you don't have MinGW installed:

- Install MSYS2: https://www.msys2.org/
- Or install MinGW via Chocolatey: `choco install mingw`
- Or use CMake Tools extension in VSCode
