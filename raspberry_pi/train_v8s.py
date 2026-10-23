from ultralytics import YOLO

model = YOLO("yolov8s.pt")
results = model.train(
    data="Merged_SmartBin_Dataset/data.yaml",
    epochs=50,
    imgsz=640,
    batch=16,
    cache=False,
    workers=4,
    device=0,
    project="runs/detect",
    name="smartbin_pi4_v8s"
)
