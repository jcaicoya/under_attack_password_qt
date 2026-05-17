# CuarzoPolar — Password Qt

Consola de operador del módulo teatral `password`. Recibe contraseñas desde `password_android` y ejecuta simulaciones teatrales de ataque que el operador controla en tiempo real.

## Qué es

Esta app forma pareja con `password_android`. Su papel es mostrar la contraseña recibida, guiar el flujo de los tres métodos de ataque y devolver el veredicto al teléfono.

## Flujo funcional

1. `password_android` envía una contraseña.
2. La contraseña aparece en la consola del operador.
3. Se simulan ataques de fuerza bruta, diccionario y oráculo.
4. El operador marca cada resultado.
5. Si una vía tiene éxito, se envía veredicto comprometido.
6. Si todas fallan, se envía veredicto segura.

## Pantalla principal

Pantalla única de `ANÁLISIS DE CONTRASEÑA` con tres paneles de ataque:

| Panel | Método | Comportamiento |
|---|---|---|
| `FUERZA BRUTA` | combinaciones cortas | animación de combinaciones |
| `DICCIONARIO` | contraseñas comunes | ciclo de wordlist |
| `ORÁCULO` | palabra + fecha | combinaciones a partir de entradas del operador |

La barra superior muestra estado de conexión y la contraseña recibida.

## Arquitectura y comunicación

- Aplicación Qt para Windows.
- Servidor WebSocket en el puerto `8767`.
- Comunicación con `password_android` a través de ADB reverse.
- Flujo orientado a operación manual por parte del operador.

Protocolo:

**Android -> Qt**

```json
{"type": "password", "value": "the-password"}
```

**Qt -> Android**

```json
{"type": "verdict", "cracked": true,  "password": "the-password"}
{"type": "verdict", "cracked": false}
```

## Tecnología

| Capa | Tecnología |
|---|---|
| Plataforma | Windows |
| Framework | Qt 6.7.3 |
| Build | CMake |
| Compilador | MSVC 2022 |
| Comunicación | WebSocket |

## Mapa de código

| Archivo | Propósito |
|---|---|
| `PasswordWindow.h/.cpp` | ventana principal, conexión WS y preparación ADB |
| `AttackScreen.h/.cpp` | pantalla de operador con los tres paneles |
| `AttackPanel.h/.cpp` | panel reutilizable de ataque |
| `PasswordWsServer.h/.cpp` | servidor WebSocket en `8767` |

## Estado actual

- Consola de operador para el módulo `password`.
- Emparejada con `password_android`.
- Flujo de análisis manual definido.
- Servidor WebSocket previsto sobre `8767`.
