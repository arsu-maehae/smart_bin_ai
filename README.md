# Smart Bin AI — Automated Waste Sorting Prototype

A historical working student project combining **YOLO object detection, Raspberry Pi, ESP32, MQTT, servo-controlled bin lids, an HC-SR04 ultrasonic sensor, and LINE Messaging API alerts**.

The original author reports the prototype ran successfully. This repository preserves the recovered implementation for demonstration, future development, and portfolio use. The imported files were inspected statically, but not independently tested on the original hardware.

## How it works

```text
USB camera -> Raspberry Pi -> YOLO (4 waste classes)
                                     |
                          MQTT publisher / bridge*
                                     |
                           MQTT broker :1883
                                     |
                                 ESP32
                           /          \
                     4 servo lids   HC-SR04 sensor
                                      |
                               LINE alert + MQTT status
```

*The recovered `raspberry_pi/run_ai.py` prints detected categories but does not include the MQTT publisher. The ESP32 MQTT subscriber **is implemented**. The author may have used a separate integration script or test controller that was not included in the recovered files.*

## Waste categories and commands

| YOLO class | Waste type | MQTT payload | ESP32 servo GPIO |
| --- | --- | --- | --- |
| 0 | Organic | `Organic` | 13 |
| 1 | Recycle | `Recycle` | 14 |
| 2 | General | `General` | 25 |
| 3 | Hazardous | `Hazardous` | 26 |

The ESP32 subscribes to `smartbin/command` on TCP port 1883. `CloseAll` closes all four lids. Each opened lid auto-closes after **7 seconds**. The ultrasonic sensor uses TRIG GPIO 5 and ECHO GPIO 18; it checks **the organic bin only** every 50 seconds, with a 10 cm full threshold, publishing `Organic_Full` on `smartbin/status` and optionally sending a LINE push notification.

## Files

- `raspberry_pi/run_ai.py` — historical ONNX camera inference for Raspberry Pi (prints detections).
- `raspberry_pi/test_cam.py` — live camera detection using NCNN model export.
- `raspberry_pi/train_yolo.py`, `train_v8s.py`, `train_finetune.py` — original training scripts.
- `raspberry_pi/smart_bin_final.py` — four-class sample inference.
- `esp32/src/main.cpp` — recovered ESP32 firmware, with sensitive values externalized.
- `esp32/platformio.ini` — LOLIN32 Lite / Arduino / PlatformIO configuration.
- `esp32/include/secrets.h.example` — local credentials template.
- `model/ncnn_metadata.yaml` — model export metadata and original class labels.
- Root `find_cam.py` and `test_cam.py` were already in this repository; they are intentionally preserved.

The complete redacted historical source package is available separately from the project author; excluded here are trained weight binaries, raw image datasets, virtual environments and credentials.

## Raspberry Pi / computer vision

Requirements: Python, OpenCV, Ultralytics YOLO, an appropriate USB camera, and the original trained model.

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
# copy your ONNX model to raspberry_pi/smart_bin_brain.onnx
cd raspberry_pi
python run_ai.py
```

The old script assumes V4L2 camera index **2**, an ONNX file named `smart_bin_brain.onnx`, input size 640, and confidence 0.60. Adjust the camera and model paths on your hardware. The NCNN viewer requires both `model.ncnn.param` and `model.ncnn.bin`; the binary weights are not part of the repository.

Training scripts require the original `Merged_SmartBin_Dataset/data.yaml` and compatible pretrained weights/GPU.

## ESP32 setup

Firmware uses LOLIN32 Lite, Arduino, ESP32Servo, and PubSubClient.

```bash
cd esp32
cp include/secrets.h.example include/secrets.h
# Edit include/secrets.h locally with your own Wi-Fi/MQTT/LINE values.
pio run
pio run --target upload
pio device monitor --baud 115200
```

A local broker and same-network access are required. Servo motors may need an external supply with shared ground. Protect the ESP32 3.3 V echo input from the 5 V HC-SR04 echo signal with a level shifter or divider.

## Security and historical limitations

- The original ESP32 source contained Wi-Fi credentials, a local broker address, a LINE channel access token, and a LINE user ID. **These were removed**; do not commit `secrets.h`.
- Rotate any credentials previously shared in source code or public links.
- ESP32 firmware uses unauthenticated MQTT on port 1883 and `setInsecure()` for HTTPS, as in the historical build. Harden before deploying on untrusted networks.
- Firmware tracks one active servo; back-to-back commands may not close the previous lid correctly. Keep commands separated or add proper queueing/interlocking.
- No benchmark scores or end-to-end test results are claimed in this import.

## Tech stack

Python · Ultralytics YOLOv8 · OpenCV · ONNX/NCNN · Raspberry Pi · ESP32 · Arduino · PlatformIO · MQTT · PubSubClient · ESP32Servo · HC-SR04 · LINE Messaging API

Original source recovered October 2026. No licence is asserted for third-party datasets or model weights.
