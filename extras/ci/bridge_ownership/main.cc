#include <iostream>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <ros_gz_bridge/bridge_config.hpp>
#include <ros_gz_bridge/ros_gz_bridge.hpp>

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
  using ros_gz_bridge::BridgeDirection;
  int failures = 0;
  int index = 0;
  for (auto direction :
       {BridgeDirection::GZ_TO_ROS, BridgeDirection::ROS_TO_GZ, BridgeDirection::BIDIRECTIONAL})
  {
    auto node = std::make_shared<ros_gz_bridge::RosGzBridge>();
    ros_gz_bridge::BridgeConfig config;
    config.ros_topic_name = "/posim_ownership_" + std::to_string(index++);
    config.gz_topic_name = config.ros_topic_name;
    config.ros_type_name = "std_msgs/msg/Float64";
    config.gz_type_name = "gz.msgs.Double";
    config.direction = direction;
    config.is_lazy = false;
    node->add_bridge(config);
    std::weak_ptr<ros_gz_bridge::RosGzBridge> weak = node;
    node.reset();
    std::cout << "direction=" << index << " node_expired=" << weak.expired() << std::endl;
    failures += !weak.expired();
  }
  rclcpp::shutdown();
  return failures ? 1 : 0;
}
