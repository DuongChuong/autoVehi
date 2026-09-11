import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():

    # Constants for paths to different packages
    package_name_perception = 'av_perception'
    package_name_processor = 'yolo_processor'

    use_sim_time = LaunchConfiguration('use_sim_time')

    declare_use_sim_time_argument = DeclareLaunchArgument(
        name='use_sim_time',
        default_value='true',
        description='Use simulation (Gazebo) clock if true')

    # Node for YOLO Detector
    yolo_detector_node = Node(
        package=package_name_perception,
        executable='yolo_detector',
        name='yolo_detector',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}])

    # Node for Image Processor
    img_processor_node = Node(
        package=package_name_processor,
        executable='img_processor_node',
        name='img_processor_node',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}])

    # Create the launch description and populate
    ld = LaunchDescription()

    # Add all launch arguments
    ld.add_action(declare_use_sim_time_argument)
    ld.add_action(yolo_detector_node)
    ld.add_action(img_processor_node)

    return ld



