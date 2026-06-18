#!/usr/bin/env python3
import argparse
import math
import sys

import rclpy
from geometry_msgs.msg import PoseStamped
from nav2_simple_commander.robot_navigator import BasicNavigator, TaskResult


def yaw_to_quaternion(yaw_rad):
    """Convert a yaw angle (radians) into a quaternion (qx, qy, qz, qw)."""
    return 0.0, 0.0, math.sin(yaw_rad / 2.0), math.cos(yaw_rad / 2.0)


def build_goal_pose(navigator, x, y, yaw_deg, frame_id):
    """Build a geometry_msgs/PoseStamped for the requested goal."""
    qx, qy, qz, qw = yaw_to_quaternion(math.radians(yaw_deg))

    goal_pose = PoseStamped()
    goal_pose.header.frame_id = frame_id
    goal_pose.header.stamp = navigator.get_clock().now().to_msg()
    goal_pose.pose.position.x = x
    goal_pose.pose.position.y = y
    goal_pose.pose.position.z = 0.0
    goal_pose.pose.orientation.x = qx
    goal_pose.pose.orientation.y = qy
    goal_pose.pose.orientation.z = qz
    goal_pose.pose.orientation.w = qw
    return goal_pose


def main():
    parser = argparse.ArgumentParser(description="Send a single Nav2 goal pose.")
    parser.add_argument("--x", type=float, required=True, help="Goal x position (m)")
    parser.add_argument("--y", type=float, required=True, help="Goal y position (m)")
    parser.add_argument("--yaw", type=float, default=0.0,
                         help="Goal heading in degrees (0 = facing +x)")
    parser.add_argument("--frame", type=str, default="map",
                         help="Reference frame for the goal (default: map)")
    parser.add_argument("--localizer", type=str, default="amcl",
                         help="Lifecycle node Nav2 should wait on before navigating: "
                              "'amcl' (default, map + AMCL setups) or 'slam_toolbox' "
                              "if you are mapping live with SLAM instead")
    args = parser.parse_args()

    rclpy.init()
    navigator = BasicNavigator()

    # Wait for Nav2 (planner, controller, localizer, BT navigator, etc.) to be
    # fully active before sending anything. If you're running SLAM Toolbox
    # live instead of AMCL + a saved map, pass --localizer slam_toolbox.
    navigator.waitUntilNav2Active(localizer=args.localizer)

    goal_pose = build_goal_pose(navigator, args.x, args.y, args.yaw, args.frame)

    print(f"Sending goal -> x={args.x}, y={args.y}, yaw={args.yaw} deg, "
          f"frame='{args.frame}'")
    navigator.goToPose(goal_pose)

    try:
        while not navigator.isTaskComplete():
            feedback = navigator.getFeedback()
            if feedback:
                remaining = feedback.distance_remaining
                eta_sec = feedback.estimated_time_remaining.sec
                print(f"  distance remaining: {remaining:.2f} m | "
                      f"ETA: {eta_sec} s", end="\r")
    except KeyboardInterrupt:
        print("\nCtrl+C received, cancelling the goal...")
        navigator.cancelTask()
        navigator.destroy_node()
        rclpy.shutdown()
        sys.exit(0)

    result = navigator.getResult()
    print()  # newline after the feedback line
    if result == TaskResult.SUCCEEDED:
        print("Goal reached successfully.")
    elif result == TaskResult.CANCELED:
        print("Goal was canceled.")
    elif result == TaskResult.FAILED:
        print("Goal failed.")
    else:
        print("Goal returned an unknown result.")

    navigator.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()