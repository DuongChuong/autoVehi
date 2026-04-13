#include "av_controller/av_skid_steer_controller.hpp"
#include "pluginlib/class_list_macros.hpp"

namespace av_controller
{

controller_interface::CallbackReturn AvSkidSteerController::on_init()
{
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::InterfaceConfiguration AvSkidSteerController::command_interface_configuration() const
{
  // We tell the hardware we want to command the velocity of 4 wheels
  controller_interface::InterfaceConfiguration conf;
  conf.type = controller_interface::interface_configuration_type::INDIVIDUAL;
  conf.names = {
    "front_left_wheel/velocity",
    "rear_left_wheel/velocity",
    "front_right_wheel/velocity",
    "rear_right_wheel/velocity"
  };
  return conf;
}

controller_interface::InterfaceConfiguration AvSkidSteerController::state_interface_configuration() const
{
  return controller_interface::InterfaceConfiguration{
    controller_interface::interface_configuration_type::NONE};
}

controller_interface::CallbackReturn AvSkidSteerController::on_configure(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  // Setup the subscriber to listen to your keyboard teleop
  velocity_command_subscriber_ = get_node()->create_subscription<geometry_msgs::msg::Twist>(
    "/cmd_vel", 10,
    [this](const geometry_msgs::msg::Twist::SharedPtr msg) {
      linear_command_ = msg->linear.x;   // Forward/Backward
      angular_command_ = msg->angular.z; // Left/Right rotation
    });
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn AvSkidSteerController::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn AvSkidSteerController::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::return_type AvSkidSteerController::update(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // ------------------------------------------------------------------
  // THIS IS THE MATH FOR YOUR SKID STEER AV
  // Forward: Left and Right spin same way. 
  // Turn: Left and Right spin opposite ways.
  // ------------------------------------------------------------------
  
  double left_wheels_speed = linear_command_ - angular_command_;
  double right_wheels_speed = linear_command_ + angular_command_;

  // Command the front left (Index 0) and rear left (Index 1)
  command_interfaces_[0].set_value(left_wheels_speed);
  command_interfaces_[1].set_value(left_wheels_speed);

  // Command the front right (Index 2) and rear right (Index 3)
  command_interfaces_[2].set_value(right_wheels_speed);
  command_interfaces_[3].set_value(right_wheels_speed);

  return controller_interface::return_type::OK;
}

}  // namespace av_controller

// This macro exports the class as a plugin so ROS 2 can load it
PLUGINLIB_EXPORT_CLASS(
  av_controller::AvSkidSteerController,
  controller_interface::ControllerInterface)