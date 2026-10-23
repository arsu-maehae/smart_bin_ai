from ultralytics import YOLO

model = YOLO("runs/detect/runs/detect/smartbin_pi4_v8s/weights/best.pt")
results = model.train(
    data="Merged_SmartBin_Dataset/data.yaml",
    epochs=20,
    imgsz=640,
    batch=16,
    device=0,
    project="runs/detect",
    name="smartbin_pi4_v8s_finetune"
)
