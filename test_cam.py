import cv2

cap = cv2.VideoCapture(2, cv2.CAP_V4L2)

cap.set(cv2.CAP_PROP_FRAME_WIDTH, 840)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 620)

# 1. ลด FPS กลับมาที่ 30 เพื่อให้เซ็นเซอร์รับแสงได้เต็มที่ในแต่ละเฟรม
cap.set(cv2.CAP_PROP_FPS, 30)

# 2. สั่งปรับความสว่าง (Brightness) ที่ตัวฮาร์ดแวร์กล้อง
# ค่าปกติมักจะอยู่ที่ราวๆ 128 (ปรับได้ตั้งแต่ 0-255 ลองเพิ่ม/ลดตัวเลข 150 ดูได้ครับ)
cap.set(cv2.CAP_PROP_BRIGHTNESS, 80)

if not cap.isOpened():
    print("ไม่สามารถเปิดกล้องได้ครับ")
    exit()

print("เปิดกล้องสำเร็จ! (กดปุ่ม 'q' บนคีย์บอร์ด เพื่อปิดหน้าต่าง)")

while True:
    ret, frame = cap.read()

    if not ret:
        print("ดึงภาพจากกล้องไม่ได้ครับ")
        break

    frame = cv2.flip(frame, 1)

    # 3. [ไม้ตายสำรอง] ปรับความสว่างด้วย Software
    # ถ้าปรับบรรทัดที่ 11 แล้วกล้องไม่ยอมสว่างขึ้น ให้เอาเครื่องหมาย # หน้าบรรทัดล่างนี้ออกครับ
    # alpha คือ Contrast (1.0-3.0) / beta คือ Brightness (0-100)
    # frame = cv2.convertScaleAbs(frame, alpha=1.2, beta=40)

    cv2.imshow('Camera Test', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()