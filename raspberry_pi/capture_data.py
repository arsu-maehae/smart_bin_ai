import cv2
import os
import time
save_folder = "new_trash_images"
os.makedirs(save_folder, exist_ok=True)
cap = cv2.VideoCapture(1)
print("📸 Space: capture image; q: quit")
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break
    cv2.imshow("Capture Data", frame)
    key = cv2.waitKey(1)
    if key == 32:
        timestamp = int(time.time() * 1000)
        filename = f"{save_folder}/new_trash_{timestamp}.jpg"
        cv2.imwrite(filename, frame)
        print(f"✅ Saved {filename}")
    elif key == ord('q'):
        break
cap.release()
cv2.destroyAllWindows()
