#! /usr/bin/env python3

import os
from pathlib import Path
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration, Command, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare

def process_ros2_controllers_config(context):

    # Get the configuration values
    prefix = LaunchConfiguration('prefix').perform(context)
    robot_name = LaunchConfiguration('robot_name').perform(context)
    enable_odom_tf = LaunchConfiguration('enable_odom_tf').perform(context)

    home = str(Path.home())

    # Define both source and install paths
    src_config_path = os.path.join(
        home,
        'autoVehi/workspace/src/av_description/config',
        robot_name
    )
    install_config_path = os.path.join(
        home,
        'autoVehi/workspace/install/av_description/share/av_description/config',
        robot_name
    )

    # Read from source template
    template_path = os.path.join(src_config_path, 'ros2_controller_templates.yaml')
    with open(template_path, 'r', encoding='utf-8') as file:
        template_content = file.read()

    # Create processed content (leaving template untouched)
    processed_content = template_content.replace('${prefix}', prefix)
    processed_content = processed_content.replace(
        'enable_odom_tf: true', f'enable_odom_tf: {enable_odom_tf}')

    # Write processed content to both source and install directories
    for config_path in [src_config_path, install_config_path]:
        os.makedirs(config_path, exist_ok=True)
        output_path = os.path.join(config_path, 'ros2_controllers.yaml')
        with open(output_path, 'w', encoding='utf-8') as file:
            file.write(processed_content)

    return []


# Define the arguments for the xacro file
ARGUMENTS = [
    DeclareLaunchArgument('robot_name', default_value='autoVehi', description='Name of the robot'),
    DeclareLaunchArgument('prefix', default_value='', description='Prefix joint'),
    DeclareLaunchArgument('enable_odom_tf', default_value='false', choices=['true', 'false'], description='Enable odometry transform broadcasting via ros2 controller'),
    DeclareLaunchArgument('use_gazebo', default_value='false',choices=['true', 'false'], description='use gazebo simulation')
]

def generate_launch_description():
    # Define filenames
    urdf_package = 'av_description'
    urdf_filename = 'autoVehi.urdf.xacro'
    rviz_filename = 'default.rviz'

    pkg_share_description = FindPackageShare(urdf_package)
    default_urdf_model_path = PathJoinSubstitution([pkg_share_description, 'urdf', urdf_filename])
    default_rviz_config_path = PathJoinSubstitution([pkg_share_description, 'rviz', rviz_filename])

    jsp_gui = LaunchConfiguration('jsp_gui')
    rviz_config_file = LaunchConfiguration('rviz_config_file')
    urdf_model = LaunchConfiguration('urdf_model')
    use_jsp = LaunchConfiguration('use_jsp')
    use_rviz = LaunchConfiguration('use_rviz')
    use_sim_time = LaunchConfiguration('use_sim_time')

    # declare the launch arguments
    declare_jsp_gui_cmd = DeclareLaunchArgument(name='jsp_gui', 
                                                default_value='true', 
                                                choices=['true', 'false'], 
                                                description='joint_state_publisher_gui')
    declare_rviz_config_file_cmd = DeclareLaunchArgument(name='rviz_config_file',
                                                    default_value=default_rviz_config_path)
    declare_urdf_model_cmd = DeclareLaunchArgument(name='urdf_model',
                                                   default_value=default_urdf_model_path)
    declare_use_jsp_cmd = DeclareLaunchArgument(name='use_jsp',
                                                default_value='true',
                                                choices=['true', 'false'],
                                                description='Enable the joint state publisher')
    declare_use_rviz_cmd = DeclareLaunchArgument(name='use_rviz',
                                                default_value='true',
                                                description='Whether to start RVIZ')

    declare_use_sim_time_cmd = DeclareLaunchArgument(name='use_sim_time',
                                                default_value='false',
                                                description='Use simulation (Gazebo) clock if true')
    
    robot_description_content = ParameterValue(Command([
        'xacro', ' ', urdf_model, ' ',
        'robot_name:=', LaunchConfiguration('robot_name'), ' ',
        'prefix:=', LaunchConfiguration('prefix'), ' ',
        'enable_odom_tf:=', LaunchConfiguration('enable_odom_tf'), ' ',
        'use_gazebo:=', LaunchConfiguration('use_gazebo')
    ]), value_type=str)

    start_robot_state_publisher_cmd = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'use_sim_time': use_sim_time,
            'robot_description': robot_description_content}])
    
    start_joint_state_publisher_cmd = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        name='joint_state_publisher',
        parameters=[{'use_sim_time': use_sim_time}],
        condition=IfCondition(use_jsp))
    
    start_joint_state_publisher_gui_cmd = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui',
        parameters=[{'use_sim_time': use_sim_time}],
        condition=IfCondition(jsp_gui))
    
    start_rviz_cmd = Node(
        condition = IfCondition(use_rviz),
        package='rviz2',
        executable='rviz2',
        output='screen',
        arguments=['-d', rviz_config_file],
        parameters=[{'use_sim_time': use_sim_time}])
    
    ld = LaunchDescription(ARGUMENTS)

    ld.add_action(OpaqueFunction(function=process_ros2_controllers_config))


    # Declare the launch options
    ld.add_action(declare_jsp_gui_cmd)
    ld.add_action(declare_rviz_config_file_cmd)
    ld.add_action(declare_urdf_model_cmd)
    ld.add_action(declare_use_jsp_cmd) 
    ld.add_action(declare_use_rviz_cmd)
    ld.add_action(declare_use_sim_time_cmd)

    # Add any actions
    ld.add_action(start_joint_state_publisher_cmd)
    ld.add_action(start_joint_state_publisher_gui_cmd)
    ld.add_action(start_robot_state_publisher_cmd)
    ld.add_action(start_rviz_cmd)

    return ld