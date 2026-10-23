from ultralytics import YOLO

model = YOLO("yolov8n.pt")
results = model.train(
    data="Merged_SmartBin_Dataset/data.yaml",
    epochs=50,
    imgsz=640,
    batch=32,
    cache=False,
    workers=4,
    mixup=0.1,
    device=0,
    project="runs/detect",
    name="smartbin_pi4_turbo2"
)
