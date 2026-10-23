# Hardware wiring and message contract

**Controller:** LOLIN32 Lite (ESP32), Arduino / PlatformIO. Confirm wiring against your own physical prototype before powering anything.

| Part | ESP32 pins | Behavior |
| --- | --- | --- |
| Organic servo | 13 | MQTT `Organic` -> 90°, close at 0° |
| Recycle servo | 14 | MQTT `Recycle` -> 90°, close at 0° |
| General servo | 25 | MQTT `General` -> 90°, close at 0° |
| Hazardous servo | 26 | MQTT `Hazardous` -> 90°, close at 0° |
| HC-SR04 TRIG | 5 | ultrasonic trigger |
| HC-SR04 ECHO | 18 | ultrasonic echo; level shift 5 V to 3.3 V |

MQTT: Subscribe `smartbin/command`; payloads: `Organic`, `Recycle`, `General`, `Hazardous`, `CloseAll`. Publish `Organic_Full` on `smartbin/status` if distance <= 10 cm; check interval 50 seconds. Lid auto-close after 7000 ms.

Known constraints from original implementation: no ESP32 command acknowledgement, one ultrasonic sensor monitoring Organic only, one tracked active servo, potentially blocking Wi-Fi/MQTT reconnect. The supplied Raspberry Pi script does not include the publisher.
