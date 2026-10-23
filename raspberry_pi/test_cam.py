import cv2
from ultralytics import YOLO

model_path = "smart_bin_brain_ncnn_model"
model = YOLO(model_path)
cap = cv2.VideoCapture(1)
print("🚀 เปิดกล้องพร้อมสแกน! กด q เพื่อออก")
while cap.isOpened():
    success, frame = cap.read()
    if not success:
        break
    results = model(frame, conf=0.4)
    annotated_frame = results[0].plot()
    cv2.imshow("Smart Bin AI - Live Test", annotated_frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
cap.release()
cv2.destroyAllWindows()
