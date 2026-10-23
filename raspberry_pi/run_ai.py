import os
import time
import cv2
from ultralytics import YOLO

# Historic Raspberry Pi camera + ONNX inference; no MQTT publish in this script.
os.environ["OMP_NUM_THREADS"] = "2"
os.environ["MKL_NUM_THREADS"] = "2"

print("⏳ กำลังโหลดสมอง AI...")
model = YOLO('smart_bin_brain.onnx', task='detect')

print("📸 กำลังเปิดกล้อง USB...")
cap = cv2.VideoCapture(2, cv2.CAP_V4L2)
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 4)

print("⏳ กำลังวอร์มกล้องให้แสงสว่าง...")
time.sleep(2)

try:
    while cap.isOpened():
        for _ in range(5):
            cap.grab()
        ret, frame = cap.retrieve()
        if not ret:
            break

        results = model(frame, imgsz=640, conf=0.6, verbose=False)
        if len(results[0].boxes) > 0:
            for box in results[0].boxes:
                class_name = model.names[int(box.cls[0])]
                print(f"👀 มองเห็นขยะ: {class_name} (วินาทีที่ {time.strftime('%S')})")
        time.sleep(0.1)
except KeyboardInterrupt:
    print("\n🛑 หยุดทำงาน")
finally:
    cap.release()
