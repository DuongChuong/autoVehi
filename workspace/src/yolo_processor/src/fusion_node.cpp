#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "vision_msgs/msg/detection2_d_array.hpp"
#include "yolo_processor/lidar_clusterer.hpp"

class FusionNode : public rclcpp::Node
{
public:
  FusionNode() : Node("fusion_processor_node"), clusterer_(0.5, 3) 
  {
    // Config the properties of camera
    double image_width = 640.0;
    double hfov_rad = 1.5284;
    
    c_x_ = image_width / 2.0;
    focal_length_ = image_width / (2.0 * std::tan(hfov_rad / 2.0));

    // Config ROI of Lidar
    min_overlap_ray_ = 183;
    max_overlap_ray_ = 357;

    // Init sub and pub
    pub_ = this->create_publisher<std_msgs::msg::String>("/processed_detections", 10);
    
    sub_scan_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
      "/scan", 10, std::bind(&FusionNode::scan_callback, this, std::placeholders::_1));
      
    sub_yolo_ = this->create_subscription<vision_msgs::msg::Detection2DArray>(
      "/yolo/detections", 10, std::bind(&FusionNode::yolo_callback, this, std::placeholders::_1));
      
    RCLCPP_INFO(this->get_logger(), "Fusion Node has been started.");
  }

private:
  LidarClusterer clusterer_;
  double c_x_, focal_length_;
  int min_overlap_ray_;
  int max_overlap_ray_;
  sensor_msgs::msg::LaserScan::SharedPtr last_scan_;

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr sub_scan_;
  rclcpp::Subscription<vision_msgs::msg::Detection2DArray>::SharedPtr sub_yolo_;

  // Calculate the potential rays as intergers
  std::pair<int, int> calculate_potential_rays(double x_min, double x_max) const
  {
      double theta1_rad = std::atan(std::abs(x_min - c_x_) / focal_length_);
      double theta2_rad = std::atan(std::abs(x_max - c_x_) / focal_length_);

      double theta1_deg = theta1_rad * 180.0 / M_PI;
      double theta2_deg = theta2_rad * 180.0 / M_PI;

      int ray1 = -1; // Value default
      int ray2 = -1; // Value default

      // Left angle
      if (x_min < c_x_ && x_min > 0.0) {
          ray1 = std::floor(2 * (135.0 - theta1_deg));
      } else if (x_min > c_x_ && x_min < 640.0) {
          ray1 = std::floor(2 * (135.0 + theta1_deg));
      }

      // Right angle
      if (x_max < c_x_ && x_max > 0.0) {
          ray2 = std::ceil(2 * (135.0 - theta2_deg));
      } else if (x_max > c_x_ && x_max < 640.0) {
          ray2 = std::ceil(2 * (135.0 + theta2_deg));
      }
    
      return {std::min(ray1, ray2), std::max(ray1, ray2)};
  }

  void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
    last_scan_ = msg; 
  }

  void yolo_callback(const vision_msgs::msg::Detection2DArray::SharedPtr msg) {
    if (!last_scan_) return; 

    // Cutting ROI and convert Polar to Cartesian 
    std::vector<Point2D> points;
    size_t start_idx = static_cast<size_t>(min_overlap_ray_);
    size_t end_idx = std::min(static_cast<size_t>(max_overlap_ray_), last_scan_->ranges.size() - 1);

    for (size_t i = start_idx; i <= end_idx; ++i) {
      double r = last_scan_->ranges[i];
      if (std::isinf(r) || std::isnan(r) || r < last_scan_->range_min || r > last_scan_->range_max) {
        continue;
      }
      
      double angle = last_scan_->angle_min + i * last_scan_->angle_increment;
      points.push_back({r * std::cos(angle), r * std::sin(angle), static_cast<int>(i), -1});
    }

    // Running BDSCAN Algorithm
    clusterer_.process(points);

    // Combine Lidar and Camera 
    std::string result_str = "Fusion result:\n";
    
    for (const auto & detection : msg->detections) {
      std::string class_id = detection.results.empty() ? "unknown" : detection.results[0].hypothesis.class_id;
      
      double center_x = detection.bbox.center.position.x;
      double size_x = detection.bbox.size_x;
      double x_min = center_x - (size_x / 2.0);
      double x_max = center_x + (size_x / 2.0);

      // Take potential rays
      std::pair<int, int> potential_rays = calculate_potential_rays(x_min, x_max);
      
      if (potential_rays.first == -1 || potential_rays.second == -1) continue;

      int matched_min_ray = 9999;
      int matched_max_ray = -1;

      // Find the most suitable cluster in angle
      for (const auto& p : points) {
        if (p.cluster_id <= 0) continue;
        
        // Calibration angle of camera and lidar
        if (p.ray_index >= potential_rays.first && p.ray_index <= potential_rays.second) {
          matched_min_ray = std::min(matched_min_ray, p.ray_index);
          matched_max_ray = std::max(matched_max_ray, p.ray_index);
        }
      }

      // Save result if found object
      if (matched_max_ray != -1) {
        result_str += " - [" + class_id + "]" + std::to_string(matched_min_ray) + " -> " + std::to_string(matched_max_ray) + "\n";
      }
    }

    // Pub result
    auto output_msg = std_msgs::msg::String();
    output_msg.data = result_str;
    pub_->publish(output_msg);
  }
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<FusionNode>());
  rclcpp::shutdown();
  return 0;
}