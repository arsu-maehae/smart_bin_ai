from ultralytics import YOLO

model = YOLO("runs/detect/train-3/weights/best.pt")
bin_names = {
    0: "🟩 ถังสีเขียว (Organic Waste)",
    1: "🟨 ถังสีเหลือง (Recycle Waste)",
    2: "🟦 ถังสีน้ำเงิน (General Waste)",
    3: "🟥 ถังสีแดง (Hazardous Waste)"
}
image_path = "test_trash.jpg"
print(f"กำลังสแกนภาพ: {image_path} ...\n")
results = model.predict(source=image_path, save=True, verbose=False, conf=0.1)
for r in results:
    if len(r.boxes) == 0:
        print("❌ ไม่พบขยะในภาพนี้")
        continue
    for box in r.boxes:
        cls_id = int(box.cls[0])
        conf = float(box.conf[0]) * 100
        target_bin = bin_names.get(cls_id, "⚪ ไม่ทราบประเภท")
        print(f"🤖 AI ตรวจพบ ID: {cls_id} (มั่นใจ {conf:.1f}%)")
        print(f"👉 คำสั่งระบบ: กรุณาทิ้งลง {target_bin}")
        print("-" * 40)
