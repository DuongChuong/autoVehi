import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    # Constants for paths to different files and folders
    package_name = 'av_localization'

    ekf_config_file_path = 'config/ekf.yaml'

    pkg_share = FindPackageShare(package=package_name).find(package_name)

    # Set path to config file
    default_ekf_config_path = os.path.join(pkg_share, ekf_config_file_path)

    # Launch configuration variables
    ekf_config_file = LaunchConfiguration('ekf_config_file')
    use_sim_time = LaunchConfiguration('use_sim_time')

    # Declare launch arguments
    declare_ekf_config_file_cmd = DeclareLaunchArgument(
        name='ekf_config_file',
        default_value=default_ekf_config_path,
        description='Path to the EKF configuration file'
    )

    declare_use_sim_time_cmd = DeclareLaunchArgument(
        name='use_sim_time',
        default_value='true',
        description='Use simulation time'
    )

    # Specify the EKF node
    start_ekf_node_cmd = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[ekf_config_file, {'use_sim_time': use_sim_time}]
    )

    # create the launch description and populate
    ld = LaunchDescription()

    ld.add_action(declare_ekf_config_file_cmd)
    ld.add_action(declare_use_sim_time_cmd)

    ld.add_action(start_ekf_node_cmd)

    return ld