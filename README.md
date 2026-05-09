# Password

Password is a Qt Widgets Cybershow application.

The app currently contains the common runtime shell and placeholder screens:

- two placeholder screens
- shared Cybershow look and feel
- bottom navigation, hidden by default
- `F9` toggles the bottom navigation row
- `F10` toggles the `DEMO` / `LIVE` badge
- number keys and arrows navigate between screens
- no Setup screen

## Launch Modes

No arguments means live mode.

```powershell
password.exe
password.exe --live
password.exe --demo
password.exe --fullscreen
password.exe --windowed
password.exe --screen 1
```

Supported launch modes:

- `--live`
- `--demo`

Supported display flags:

- `--fullscreen`
- `--windowed`
- `--screen <n>`
- `--debug`

## Requirements

- Qt 6.7.3 for MSVC 2022 64-bit
- CMake 3.28+
- Visual Studio 2022 C++ toolchain
- Windows target machine with the Visual C++ Redistributable

`CMakeLists.txt` currently points to:

```cmake
set(CMAKE_PREFIX_PATH "C:/Qt/6.7.3/msvc2022_64")
```

Change that path if Qt is installed somewhere else.

## Build And Run

Configure and build manually:

```powershell
cmake -S . -B build\msvc2022 -DCMAKE_BUILD_TYPE=Release
cmake --build build\msvc2022 --config Release
```

Run the built executable:

```powershell
.\build\msvc2022\Release\password.exe
.\build\msvc2022\Release\password.exe --demo --windowed
.\build\msvc2022\Release\password.exe --live --fullscreen
```

During runtime:

- `1` and `2` switch screens
- `Left Arrow` and `Right Arrow` move between screens
- `F9` shows or hides the bottom navigation row
- `F10` shows or hides the `DEMO` / `LIVE` badge

## Development Notes

The first implementation step for this app is to replace the two placeholder screens in `PasswordWindow.cpp`.

Keep the shared Cybershow contract:

- no Setup screen
- no arguments mean `--live`
- `F9` toggles the bottom navigation row
- `F10` toggles the `DEMO` / `LIVE` badge
- number keys and arrows navigate between screens
- read-only display widgets should stay non-focusable

## Release Packaging

Use the same deploy flow as the other Cybershow apps:

```powershell
.\package-release.ps1
.\package-release.ps1 -Force
```

The script builds Release, stages the executable and required Qt runtime files, creates `dist\cybershow-password-vNN.zip`, updates `releases.json`, and creates a matching git tag when the project is inside a git repository.

The deployable artifact is the zip under `dist\`. It contains `password.exe`, required Qt DLLs, the Windows platform plugin, this README, and `RUNBOOK.md`.

