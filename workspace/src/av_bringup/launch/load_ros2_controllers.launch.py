#!/usr/bin/env python3

from launch import LaunchDescription
from launch.actions import ExecuteProcess, RegisterEventHandler, TimerAction
from launch.event_handlers import OnProcessExit

def generate_launch_description():
    # start skid_steer_controller
    start_skid_steer_controller_cmd = ExecuteProcess(
        cmd=['ros2', 'control', 'load_controller', '--set-state', 'active', 'skid_steer_controller'],
        output='screen'
    )

    # start joint state broadcaster
    start_joint_state_broadcaster = ExecuteProcess(
        cmd=['ros2', 'control', 'load_controller', '--set-state', 'active', 'joint_state_broadcaster'],
        output='screen'
    )

    # add delay to joint_state_broadcaster
    delayed_start = TimerAction(
        period=25.0,
        actions=[start_joint_state_broadcaster]
    )

    # Register event handler for sequencing
    load_joint_state_broadcaster_cmd = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=start_joint_state_broadcaster,
            on_exit=[start_skid_steer_controller_cmd]
        )
    )

    # create and populate the launch description
    ld = LaunchDescription()

    ld.add_action(delayed_start)
    ld.add_action(load_joint_state_broadcaster_cmd)

    return ld

