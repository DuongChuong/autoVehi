#include <string>
#include <cmath>
#include <algorithm>
#include <memory>
#include <utility>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "vision_msgs/msg/detection2_d_array.hpp"

class ImgProcessorNode : public rclcpp::Node
{
public:
    ImgProcessorNode() : Node("img_processor_node")
    {
        // config properties of camera
        double image_width = 640;
        double hfov_rad = 1.5284;

        // calculate cx and f
        cx_ = image_width / 2.0;
        f_ = image_width / (2.0 * std::tan(hfov_rad / 2.0));

        // create publisher for detection messages
        publisher_ = this->create_publisher<std_msgs::msg::String>("processed_detections", 10);

        // create subscriber for detection messages
        subscription_ = this->create_subscription<vision_msgs::msg::Detection2DArray>(
            "/yolo/detections", 10,
        std::bind(&ImgProcessorNode::detection_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Image Processor Node has been started.");
    }

private:
    // status of objects
    double cx_;
    double f_;

    // encapsulated
    std::pair<int, int> calculate_potential_rays(double x_min, double x_max) const
    {
        double theta1_rad = std::atan(std::abs(x_min - cx_) / f_);
        double theta2_rad = std::atan(std::abs(x_max - cx_) / f_);

        // return the angles in degrees
        double theta1_deg = theta1_rad * 180.0 / M_PI;
        double theta2_deg = theta2_rad * 180.0 / M_PI;

        int ray1 = -1; // default invalid
        int ray2 = -1; // default invalid

        // calculate the potential rays as integers
        if (x_min < cx_ && x_min > 0.0)
        {
            ray1 = std::floor(2*(135 - theta1_deg));
        } else if (x_min > cx_ && x_min < 640.0)
        {
            ray1 = std::floor(2*(135 + theta1_deg));
        }

        if (x_max < cx_ && x_max > 0.0)
        {
            ray2 = std::ceil(2*(135 - theta2_deg));
        } else if (x_max > cx_ && x_max < 640.0)
        {
            ray2 = std::ceil(2*(135 + theta2_deg));
        }
      
        return {std::min(ray1, ray2), std::max(ray1, ray2)};

    }

    // Callback function for processing detection messages
    void detection_callback(const vision_msgs::msg::Detection2DArray::SharedPtr msg) const
    {
        std::string result_log = "Lidar rays: ";

        // Process each detection in the message
        for (const auto & detection : msg->detections){
            // Extract class_id of objects
            std::string class_id = "unknown";
            if (!detection.results.empty()) {
                class_id = detection.results[0].hypothesis.class_id;
            }

            // Extract bounding box coordinates
            double center_x = detection.bbox.center.position.x;
            double size_x = detection.bbox.size_x;

            double x_min = center_x - (size_x / 2.0);
            double x_max = center_x + (size_x / 2.0);

            std::pair<int, int> ray_range = calculate_potential_rays(x_min, x_max);

            // RCLCPP_INFO(this->get_logger(), "Detected object: %s, Bounding Box: [%.2f, %.2f], Potential Rays: [%d, %d]",
            //             class_id.c_str(), x_min, x_max, ray_range.first, ray_range.second);
            
            // Save the processed detection information to the output string    
            result_log += "[" + class_id + ": " + std::to_string(ray_range.first) + " - " + std::to_string(ray_range.second) + "] ";
        }

        // (Implementation for publishing would go here)
        auto processed_msg = std_msgs::msg::String();
        processed_msg.data = result_log;
        publisher_->publish(processed_msg);
    }
    // Smart pointers for publisher and subscriber
    rclcpp::Subscription<vision_msgs::msg::Detection2DArray>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ImgProcessorNode>());
    rclcpp::shutdown();
    return 0;
}
