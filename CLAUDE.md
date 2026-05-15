# CLAUDE.md - Password

Working context for Claude, Codex, Gemini, or another coding agent modifying the Password Cybershow app.

Do not duplicate the README here. The README is the developer handoff for building, running, launch modes, and packaging. This file is for implementation rules and project intent.

## Purpose

This app was cloned from the shared Qt Widgets runtime shell for Cybershow apps.

It currently has placeholder screens and should be turned into the real Password module without breaking the shared runtime contract.

- no Setup screen
- no dependencies on sibling projects

## Current Contract

- Executable target: `password`
- Deployed executable: `under_attack_password_qt.exe`
- Deploy zip prefix: `bajo-ataque-password`
- No arguments mean live mode.
- Supported modes are only `--live` and `--demo`.
- Runtime screens are operator-controlled.
- Bottom navigation starts hidden and toggles with `F9`.
- Mode badge starts hidden and toggles with `F10`.
- Screen navigation uses number keys and left/right arrows.
- Setup/configure flow is intentionally absent.

## Source Map

- `src/main.cpp`: launch parsing, app setup, screen-aware window placement, stdout status lifecycle.
- `src/PasswordWindow.*`: two-screen runtime shell, keyboard navigation, bottom nav, mode badge.
- `src/ScreenPage.*`: reusable page chrome.
- `src/cybershow/common/CyberAppMode.*`: launch mode parser.
- `src/cybershow/common/CyberOperationalLog.*`: operational log writer.
- `src/cybershow/common/CyberOrchestratorProtocol.h`: `CYBERSHOW_*` stdout lines.
- `src/cybershow/ui/*`: theme, background, and navigation widgets.

## Implementation Rules

1. Keep the shared launch contract unless the Cybershow family standard changes.
2. Replace placeholder pages in `PasswordWindow::buildUi()` or split them into app-specific screen builders.
3. Keep read-only display widgets `Qt::NoFocus`.
4. Keep `F9`, `F10`, number keys, and arrows consistent across apps.
5. Keep screen-aware startup sizing and shared theme scaling.
6. Keep stdout protocol lines stable for orchestration.
7. Keep operator-facing text in Spanish unless a specific scene requires otherwise.

## Things Not To Reintroduce

- Setup screen.
- `--configure`.
- automatic demo screen cycling.
- letter-based navigation shortcuts.
- focusable read-only dashboards.
- app-specific standards markdown copied from older refactors.

## Validation Expectations

The user is responsible for compiling, running, packaging, and deployment tests.

For code changes, make the change and stop with a clear note that it is ready for the user's build/test pass. Do not run the app unless explicitly asked.

