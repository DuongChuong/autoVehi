#!/usr/bin/env python3
import sys
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from vision_msgs.msg import Detection2DArray, Detection2D, ObjectHypothesisWithPose
from cv_bridge import CvBridge
sys.path.insert(0, '/home/chuongday/autoVehi/venv/lib/python3.12/site-packages')
from ultralytics import YOLO

class YoloDetectorNode(Node):
    def __init__(self):
        super().__init__('yolo_detector')
        self.get_logger().info('Yolo Detector Node has been started.')
        # Load the yolo11 model 
        self.model = YOLO('yolo11n.pt')
        self.bridge = CvBridge()

        # subscribe to the camera topic
        self.subscription = self.create_subscription(
            Image, '/cam_1/color/image_raw', self.image_callback, 10)

        # publish the annotated image for visualization in rviz
        self.publisher =self.create_publisher(Image, '/yolo/annotated_image', 10)
        # publish the bounding boxes of detected objects
        self.bbox_publisher = self.create_publisher(Detection2DArray, '/yolo/detections', 10)
        
        self.get_logger().info("perception node started")


    def image_callback(self,msg):
        try:
            # convert ROS image to opencv frame
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')

            # run inference
            results = self.model(cv_image, verbose=False)

            # synchronize the detection results with the original image
            detection_array_msg = Detection2DArray()
            detection_array_msg.header = msg.header

            for box in results[0].boxes:
                # Take the center point (x, y), width (w), and height (h) of the bounding box
                cx, cy, w, h = box.xywh[0].tolist()

                conf = box.conf[0].item() # confidence score
                cls_id = int(box.cls[0].item()) # class id
                class_name = self.model.names[cls_id] # class name
                
                detection = Detection2D()
                detection.header = msg.header
                detection.bbox.center.position.x = float(cx)
                detection.bbox.center.position.y = float(cy)
                detection.bbox.size_x = float(w)
                detection.bbox.size_y = float(h)

                hypothesis = ObjectHypothesisWithPose()
                hypothesis.hypothesis.class_id = class_name
                hypothesis.hypothesis.score = float(conf)
                detection.results.append(hypothesis)

                detection_array_msg.detections.append(detection)

            # publish the detection results
            self.bbox_publisher.publish(detection_array_msg)


            # plot bounding boxes on the image
            annotated_frame = results[0].plot()

            # convert back to ROS image and publish
            img_msg = self.bridge.cv2_to_imgmsg(annotated_frame, 'bgr8')
            self.publisher.publish(img_msg)

        except Exception as e:
            self.get_logger().error(f"Inference error: {e}")
        
def main(args=None):
    rclpy.init(args=args)
    node = YoloDetectorNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

    
if __name__ == '__main__':
    main()