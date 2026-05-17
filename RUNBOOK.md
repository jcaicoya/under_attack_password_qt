# RUNBOOK - Password Qt

## Deploy

### Requisitos

- Qt 6.7.3 para MSVC 2022 64-bit
- CMake 3.28+
- Visual Studio 2022 C++ toolchain
- Visual C++ Redistributable en la máquina destino

Si hace falta, ajustar `CMAKE_PREFIX_PATH` al Qt instalado.

### Build manual

```powershell
cmake -S . -B build\msvc2022 -DCMAKE_BUILD_TYPE=Release
cmake --build build\msvc2022 --config Release
```

### Empaquetado de release

```powershell
.\package-release.ps1
.\package-release.ps1 -Force
```

- Genera `dist\bajo-ataque-password-vNN.zip`.
- El artefacto desplegable es el zip dentro de `dist\`.

## Arranque

Modo live por defecto:

```powershell
under_attack_password_qt.exe
```

Otras variantes:

```powershell
under_attack_password_qt.exe --live
under_attack_password_qt.exe --demo
under_attack_password_qt.exe --fullscreen
under_attack_password_qt.exe --windowed
under_attack_password_qt.exe --screen 1
under_attack_password_qt.exe --debug
```

## Manejo de la aplicación

### Navegación

- `1`: ir a la pantalla 1
- `2`: ir a la pantalla 2
- `Left Arrow`: pantalla anterior
- `Right Arrow`: pantalla siguiente
- `F9`: mostrar u ocultar la barra inferior de navegación
- `F10`: mostrar u ocultar el indicador `DEMO` / `LIVE`

### Flujo de operación

1. Esperar la contraseña enviada desde `password_android`.
2. Revisar el panel de `FUERZA BRUTA`.
3. Si falla, continuar con `DICCIONARIO`.
4. Si también falla, usar `ORÁCULO`.
5. Marcar el resultado en el panel correspondiente.
6. Si ninguna vía la descubre, enviar resultado segura.

## Conectividad con Android

- La app usa WebSocket en `8767`.
- Al arrancar ejecuta `adb reverse tcp:8767 tcp:8767`.
- Si `adb` no está en `PATH`, usa como fallback:

```text
C:/Users/caico/AppData/Local/Android/Sdk/platform-tools/adb.exe
```

- El dispositivo debe estar conectado antes de arrancar la app, o el túnel debe prepararse por otro medio.

## Consideraciones operativas

- Si se ejecuta sin argumentos, entra en modo `LIVE`.
- No hay pantalla de setup.
- La navegación está pensada para hacerse desde teclado.
- La barra inferior empieza oculta.
- El indicador `DEMO` / `LIVE` empieza oculto.
