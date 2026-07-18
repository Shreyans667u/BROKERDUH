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

## More features

- **Drag-and-drop** — reorder channel cards and control cards by dragging them
- **📹 Camera streaming** — add a "Camera" control, paste your ESP32-CAM's MJPEG URL (usually `http://cam-ip:81/stream`)
- **🗺️ GPS tracking** — publish `{"lat":37.77,"lon":-122.41}` to any topic containing "gps" and it plots live on the map
- **⚡ Visual automation editor** — drag trigger/action node pairs around a canvas; each pair is "if topic crosses value → publish payload"
- **📲 Installable PWA** — has a manifest, service worker, and icons. **Install only works when served over http/https** — just opening `index.html` by double-click won't trigger the install prompt. Serve it locally with e.g. `python3 -m http.server` from the project folder, or host it (GitHub Pages works great for this)
- **Light/dark theme**, **keyboard shortcuts** (`/` search, `t` theme, `l` lock, `Esc` close dialogs), **device icons**, **global search bar**, **config import/export** (exports profiles/controls/rules as JSON — note this can include broker passwords in plain text, so treat exported files carefully)

## Setting up the NodeMCU relay toggle (D4)

1. Wire a relay module: **VCC → 3V3 or 5V** (check your relay module's rating), **GND → GND**, **IN → D4**.
   Read the caution comment at the top of the `.ino` — D4/GPIO2 is also a boot-mode
   pin, so if you hit random boot failures, switch `RELAY_PIN` to D1 (GPIO5) instead.
2. Open `firmware/nodemcu_relay/nodemcu_relay.ino` in Arduino IDE, install the
   **PubSubClient** library if you don't have it, fill in your WiFi name/password
   and your broker's LAN IP, then flash it to the board.
3. In BROKERDUH, connect to your broker (Quick Connect or a saved profile).
4. Click **+ Add control**, set:
   - Label: `Relay`
   - Topic: `devices/relay1/commands`
   - Type: `Toggle switch`
   - ON payload: `on`, OFF payload: `off`
5. Flip the switch — the relay should click, and `devices/relay1/status` will show
   up as a live channel confirming the real state.

## Making the live connection reliable

A few things that commonly look like bugs but are actually browser/network behavior:
- **Switches/sliders/buttons feeling flaky** — fixed in this version: control cards
  used to be draggable as a whole element, which intermittently ate clicks on the
  inputs inside them. Now only the small `⠿` handle is draggable.
- **"Connected" but no messages/charts show up** — almost always one of: the device
  hasn't actually published anything yet, the broker's WebSocket path needs `/mqtt`
  appended to the URL, or (very common) this page loaded over **https** while your
  broker is plain **ws://** — browsers silently block that as mixed content. Use
  `wss://` in that case, or serve BROKERDUH itself over plain http on the same LAN.
- There's now a **Quick Connect** field (no saved profile needed) and a visible
  **Disconnect** button, plus toasts confirming each stage: connecting → connected →
  subscribed → first message. If it stalls at "subscribed," the problem is on the
  device/broker side, not the dashboard.

## Project structure

```
brokerduh/
├── index.html                        # the dashboard (demo + live mode)
├── manifest.json                     # PWA manifest
├── service-worker.js                 # PWA offline caching
├── icon-192.png / icon-512.png       # PWA icons
├── docker-compose.yml                # one-command broker
├── broker/mosquitto.conf             # broker config (MQTT + WebSocket listeners)
├── firmware/esp32_publisher/         # example ESP32 firmware
├── firmware/nodemcu_relay/           # NodeMCU relay toggle on D4
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
