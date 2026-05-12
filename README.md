# CuarzoPolar — Password Qt

Operator console for the `password` cybershow module. Receives passwords from the `password-android` app and runs theatrical attack simulations that the operator controls in real time.

Communicates with `password-android` over WebSocket on port `8767`, tunnelled via ADB reverse (`adb reverse tcp:8767 tcp:8767`) so the Android always connects to `localhost`.

## Show flow

1. `password-android` sends a password → appears on the operator screen.
2. **Fuerza Bruta** panel activates automatically and starts cycling combinations.
3. Operator clicks **ENCONTRADA** (found) or **FALLIDA** (failed) on each panel.
4. If brute force fails → **Diccionario** activates. If that fails → **Oráculo** activates.
5. For Oráculo the operator types the word and date the hook said aloud; the panel cycles combinations based on those inputs.
6. Once a panel is marked found, the verdict (cracked + password) is sent to Android.
7. If all three panels fail, operator clicks **ENVIAR SEGURA** → Android shows safe result.

## Single screen: ANÁLISIS DE CONTRASEÑA

Three attack panels side by side:

| Panel | Method | Animation |
|-------|--------|-----------|
| FUERZA BRUTA | Short numeric / alphanumeric sequences | Random 4–5 digit strings |
| DICCIONARIO | Common password wordlist | Cycles through ~40 common passwords |
| ORÁCULO | Word + date combinations | Built from operator-entered inputs |

Top bar shows live connection status and the received password (operator-visible only).

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

- `F9` shows or hides the bottom navigation row
- `F10` shows or hides the `DEMO` / `LIVE` badge

## ADB tunnel

On startup the app runs `adb reverse tcp:8767 tcp:8767` automatically. If `adb` is not in PATH it falls back to the standard Android SDK location:

```
C:/Users/caico/AppData/Local/Android/Sdk/platform-tools/adb.exe
```

The device must already be connected (USB or wireless ADB) before starting the app, or the tunnel can be set up manually via `adb-bridge`.

## Protocol

**Android → Qt:**
```json
{"type": "password", "value": "the-password"}
```

**Qt → Android:**
```json
{"type": "verdict", "cracked": true,  "password": "the-password"}
{"type": "verdict", "cracked": false}
```

## Source map

| File | Purpose |
|------|---------|
| `PasswordWindow.h/.cpp` | Main window, WS server wiring, ADB tunnel setup |
| `AttackScreen.h/.cpp` | Operator screen with three panels and verdict controls |
| `AttackPanel.h/.cpp` | Single reusable attack panel (brute force / dictionary / oracle) |
| `PasswordWsServer.h/.cpp` | WebSocket server on port 8767 |

## Release Packaging

Use the same deploy flow as the other Cybershow apps:

```powershell
.\package-release.ps1
.\package-release.ps1 -Force
```

The script builds Release, stages the executable and required Qt runtime files, creates `dist\bajo-ataque-password-vNN.zip`, updates `releases.json`, and creates a matching git tag when the project is inside a git repository.

The deployable artifact is the zip under `dist\`. It contains `password.exe`, required Qt DLLs, the Windows platform plugin, this README, and `RUNBOOK.md`.

