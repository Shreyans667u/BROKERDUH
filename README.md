# BROKERDUH

A live MQTT pub/sub dashboard — a "patch bay" for watching your ESP32/ESP8266 boards
talk in real time, and talking back to them.

Open `index.html` and it boots straight into **demo mode**: fake devices publish fake
telemetry every second so you can see the whole UI work with zero setup. Flip it to
**live mode** and point it at a real broker to watch your actual hardware.

## Features

- **Saved broker profiles** — save name/URL/credentials per broker, switch between them with one click
- **Auth** — username/password over TLS (`wss://`) works out of the box. Mutual-TLS
  (client certificates) is *not* something browser JS can do directly — if you need it,
  terminate it at a reverse proxy in front of the broker's WebSocket listener instead
- **Message history + search** — every message is logged with a search box to filter by topic or payload
- **Real value charts** — click any channel card to open a detail chart plotting actual numeric values over time (not just a pulse animation)
- **Device management** — devices are auto-grouped from `devices/<id>/...` topics; rename, mute alerts, or remove a device
- **Mobile-friendly** — single-column layout and larger tap targets under 640px width
- **Alerts** — set a min/max threshold per channel (in the chart modal); a toast fires when a value crosses it
- **JSON tools** — payloads that are valid JSON are pretty-printed automatically; toggle "JSON" in the publish console to validate before sending

Note: profiles, devices, and history live in memory for the current browser session —
there's intentionally no `localStorage` call in this file so it stays safe to preview
in sandboxed environments. If you're self-hosting this permanently, it's a small addition
to persist `state.profiles` to `localStorage` yourself.

## Quickstart (demo mode, zero setup)

Just open `index.html` in a browser. That's it — it's a single static file.

## Quickstart (live mode, real broker)

1. Start the broker:
   ```
   docker compose up -d
   ```
   This runs Mosquitto with two listeners:
   - `1883` — raw MQTT, for your ESP32/ESP8266 boards
   - `9001` — MQTT-over-WebSocket, for the browser dashboard

2. Flash `firmware/esp32_publisher/esp32_publisher.ino` to a board:
   - Fill in your WiFi credentials and your broker's LAN IP
   - It publishes fake telemetry to `devices/<id>/telemetry` every 3s
   - It also sets up a Last-Will-and-Testament so the dashboard knows instantly
     if the board drops offline

3. Open `index.html`, paste `ws://<broker-ip>:9001` into the connect box, hit
   **Connect broker**. You'll see your board's topic appear as a live channel card.

4. Use the **Publish** panel to send commands back to a device
   (e.g. topic `devices/eyebot/commands`, payload `blink`).

## Project structure

```
brokerduh/
├── index.html                        # the dashboard (demo + live mode)
├── docker-compose.yml                # one-command broker
├── broker/mosquitto.conf             # broker config (MQTT + WebSocket listeners)
├── firmware/esp32_publisher/         # example ESP32 firmware
└── README.md
```

## Pushing this to GitHub

```bash
cd brokerduh
git init
git add .
git commit -m "Initial commit: BROKERDUH"
gh repo create brokerduh --public --source=. --push
```

(No `gh` CLI? Create an empty repo named `brokerduh` on github.com first, then:)

```bash
git remote add origin https://github.com/<your-username>/brokerduh.git
git branch -M main
git push -u origin main
```

## Ideas to extend it

- Add a history backend (SQLite/Postgres) so charts survive a page refresh
- Add topic wildcard subscriptions in the UI (`devices/+/telemetry`)
- Add TLS + auth on the broker before exposing it beyond your LAN
- Multiple dashboards open at once — they all see the same live traffic
