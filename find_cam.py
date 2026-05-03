import cv2

print("กำลังสแกนหากล้องที่เชื่อมต่ออยู่...")
working_cameras = []

for i in range(10): # สแกนหาตั้งแต่พอร์ต 0 ถึง 9
    cap = cv2.VideoCapture(i, cv2.CAP_V4L2)
    if cap.isOpened():
        ret, _ = cap.read()
        if ret:
            working_cameras.append(i)
        cap.release()

if working_cameras:
    print(f"เจอกล้องที่พอร์ตหมายเลข: {working_cameras}")
    print(f"-> ให้นำเลข {working_cameras[0]} ไปใช้ในไฟล์ test_cam.py ได้เลยครับ!")
else:
    print("ไม่เจอกล้องเลยครับ ลองถอดสาย USB แล้วเสียบใหม่ หรือเช็คว่ามีโปรแกรมอื่นดึงกล้องไปใช้หรือเปล่า")