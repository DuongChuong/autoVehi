#ifndef av_controller__AV_SKID_STEER_CONTROLLER_HPP_
#define av_controller__AV_SKID_STEER_CONTROLLER_HPP_

#include <memory>
#include <string>
#include <vector>

#include "controller_interface/controller_interface.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"

namespace av_controller
{

class AvSkidSteerController : public controller_interface::ControllerInterface
{
public:
  // These are the standard lifecycle functions required by ROS 2 Control
  controller_interface::InterfaceConfiguration command_interface_configuration() const override;
  controller_interface::InterfaceConfiguration state_interface_configuration() const override;
  
  controller_interface::return_type update(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

  controller_interface::CallbackReturn on_init() override;
  controller_interface::CallbackReturn on_configure(
    const rclcpp_lifecycle::State & previous_state) override;
  controller_interface::CallbackReturn on_activate(
    const rclcpp_lifecycle::State & previous_state) override;
  controller_interface::CallbackReturn on_deactivate(
    const rclcpp_lifecycle::State & previous_state) override;

private:
  // Subscriber to listen to keyboard/joystick commands
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velocity_command_subscriber_;
  
  // Variables to hold the current requested speed
  double linear_command_ = 0.0;
  double angular_command_ = 0.0;
};

}  // namespace av_controller

#endif  // av_controller__AV_SKID_STEER_CONTROLLER_HPP_