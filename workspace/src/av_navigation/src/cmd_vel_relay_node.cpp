#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"

class CmdVelRelay : public rclcpp::Node {
    public:
        CmdVelRelay() : Node("cmd_vel_relay") {
            subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
                "/cmd_vel", 10, std::bind(&CmdVelRelay::cmd_vel_callback, this, std::placeholders::_1)
            );
            publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(
                "/skid_steer_controller/cmd_vel", 10
            );
            RCLCPP_INFO(this->get_logger(), "CmdVelRelay node has been started.");
        }
    private:
        void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg) {
            auto stamped_msg = std::make_unique<geometry_msgs::msg::TwistStamped>();
            stamped_msg->header.stamp = this->now();
            stamped_msg->header.frame_id = "chassis_link";
            stamped_msg->twist = *msg;
            publisher_->publish(std::move(stamped_msg));
        }

        // Declare class member variables for the subscriber and publisher
        rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
        rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr publisher_;
};

int main(int argc, char *argv[]) {
    // Init Ros2
    rclcpp::init(argc, argv);

    auto node = std::make_shared<CmdVelRelay>();
    // Spin the node
    rclcpp::spin(node);
    // Clear up ros2 resources
    rclcpp::shutdown();

    return 0;
}