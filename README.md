# Automation_System_NA22OL

## Getting started (run the simulation locally)

This project uses [Wokwi](https://wokwi.com) to simulate an ESP32 circuit (DHT22 sensor, LDR, LED, servo). It's set up to run **locally in VS Code**, which is much faster than the wokwi.com website.

### 1. One-time setup (only needed the first time on a new computer)

1. Install [VS Code](https://code.visualstudio.com/).
2. Install these two VS Code extensions (Extensions panel, `Ctrl+Shift+X`, search by name):
   - **Wokwi Simulator**
   - **PlatformIO IDE**
3. Restart VS Code if prompted (PlatformIO asks for this after install).
4. Open this project folder in VS Code (`File > Open Folder`).
5. Click the **Wokwi icon** in the sidebar and follow the prompt to request a free license at wokwi.com (needs a free Wokwi account, one-time, tied to you).

### 2. Every time you want to run it

1. Open this folder in VS Code.
2. Build the code: click the **checkmark (✓)** in the blue status bar at the bottom (PlatformIO: Build). Wait for "SUCCESS".
3. Press `F1`, type **Wokwi: Start Simulator**, press Enter.

That's it — the circuit runs locally, no browser needed.

### No time to set up the extensions?

You can still run it the simple way: go to [wokwi.com](https://wokwi.com), create a new ESP32 project, and paste in the contents of `diagram.json` and `src/main.cpp`. Slower, but zero setup.