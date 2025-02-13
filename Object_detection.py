import cv2
import numpy as np

# File paths for the YOLO model
cfg_path = "yolov2-tiny.cfg"
weights_path = "yolov2-tiny.weights"
coco_names_path = "coco.names"
  # Path to coco.names file

# Load YOLO model
net = cv2.dnn.readNet(weights_path, cfg_path)

# Load the COCO class labels
with open(coco_names_path, "r") as f:
    classes = [line.strip() for line in f.readlines()]

# Get YOLO layer names
layer_names = net.getLayerNames()
output_layers = [layer_names[i - 1] for i in net.getUnconnectedOutLayers()]

# Start video capture (0 for webcam, replace with file path for a video)q
cap = cv2.VideoCapture(0)

while True:
    # Capture frame-by-frame
    ret, frame = cap.read()
    
    if not ret:
        break

    # Prepare the image for YOLO (resize, normalize, and convert to blob)
    blob = cv2.dnn.blobFromImage(frame, 0.00392, (416, 416), (0, 0, 0), True, crop=False)
    net.setInput(blob)
    outs = net.forward(output_layers)

    # Process detections
    class_ids = []
    confidences = []
    boxes = []
    height, width, channels = frame.shape

    # Iterate through detections and filter weak ones
    for out in outs:
        for detection in out:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]
            
            # Filter weak detections by confidence threshold
            if confidence > 0.5:
                # Get the coordinates of the bounding box
                center_x = int(detection[0] * width)
                center_y = int(detection[1] * height)
                w = int(detection[2] * width)
                h = int(detection[3] * height)
                
                # Rectangle coordinates (top-left corner)
                x = int(center_x - w / 2)
                y = int(center_y - h / 2)

                # Add to the lists
                boxes.append([x, y, w, h])
                confidences.append(float(confidence))
                class_ids.append(class_id)

    # Apply Non-Maximum Suppression to eliminate redundant overlapping boxes
    indexes = cv2.dnn.NMSBoxes(boxes, confidences, 0.5, 0.4)

    # Draw bounding boxes and labels
    if len(indexes) > 0:
        for i in indexes.flatten():
            x, y, w, h = boxes[i]
            label = str(classes[class_ids[i]])
            confidence = confidences[i]

            # Draw bounding box and label
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            cv2.putText(frame, f"{label} {confidence:.2f}", (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

    # Show the frame with detections
    cv2.imshow("YOLO Object Detection", frame)

    # Exit on pressing 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release resources
cap.release()
cv2.destroyAllWindows()