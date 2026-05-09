# RUNBOOK - Password

Este documento es para el operador. Resume como arrancar y manejar la aplicacion durante una prueba o una funcion.

## Arranque

Modo live por defecto:

```powershell
password.exe
```

Modo live explicito:

```powershell
password.exe --live
```

Modo demo:

```powershell
password.exe --demo
```

Pantalla completa:

```powershell
password.exe --live --fullscreen
```

Ventana:

```powershell
password.exe --demo --windowed
```

Seleccionar monitor:

```powershell
password.exe --live --fullscreen --screen 1
```

## Navegacion

- `1`: ir a la pantalla 1
- `2`: ir a la pantalla 2
- `Left Arrow`: pantalla anterior
- `Right Arrow`: pantalla siguiente
- `F9`: mostrar u ocultar la barra inferior de navegacion
- `F10`: mostrar u ocultar el indicador `DEMO` / `LIVE`

## Durante El Show

- La barra inferior empieza oculta.
- El indicador `DEMO` / `LIVE` empieza oculto.
- La navegacion esta pensada para hacerse desde teclado.
- No hay pantalla de Setup.
- Si se ejecuta sin argumentos, la aplicacion entra en modo `LIVE`.

## Pantallas

1. `Pantalla base`
   - pantalla placeholder para sustituir por la primera experiencia real de la app.

2. `Segunda pantalla`
   - pantalla placeholder para sustituir por la segunda experiencia real de la app.

## Cierre

Cerrar la ventana de la aplicacion normalmente.

Si se esta probando desde consola, revisar despues los logs en:

```text
logs\password.log
```

