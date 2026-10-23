import cv2
import time
print("📸 กำลังเปิดกล้อง...")
cap = cv2.VideoCapture(2, cv2.CAP_V4L2)
cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 3)
for _ in range(30):
    cap.read()
    time.sleep(0.1)
ret, frame = cap.read()
if ret:
    cv2.imwrite('debug_camera.jpg', frame)
    print("✅ Saved debug_camera.jpg")
else:
    print("❌ Camera capture failed")
cap.release()
