// SPDX-License-Identifier: Apache-2.0
#include <gz/msgs/double_v.pb.h>
#include <GeographicLib/Geoid.hpp>
#include <gz/transport/Node.hh>
#include <memory>
#include <mutex>
#include <rclcpp/rclcpp.hpp>
#include "GnssInput.hh"

namespace posim
{
class MarineGnssInput : public rclcpp::Node
{
  struct State
  {
    std::mutex mutex;
    GnssInput input;
  };

public:
  MarineGnssInput() : Node("marine_gnss_input"), geoid("egm96-5", "", true, true)
  {
    const auto topic = declare_parameter<std::string>("gz_topic", "/model/bluerov2/gnss");
    maxAge = declare_parameter("max_age", 1.0);
    if (!std::isfinite(maxAge) || maxAge <= 0)
    {
      throw std::invalid_argument("max_age must be finite and positive");
    }
    publisher = create_publisher<mavros_msgs::msg::GPSINPUT>("/mavros/gps_input/gps_input", 1);
    const auto state = receiver;
    std::function<void(const gz::msgs::Double_V &)> receive = [state](const auto & message)
    {
      std::lock_guard<std::mutex> lock(state->mutex);
      if (message.data_size() != 15)
      {
        state->input.Invalidate();
        return;
      }
      GnssInput::Packet packet;
      std::copy(message.data().begin(), message.data().end(), packet.begin());
      state->input.Receive(packet);
    };
    if (!transport.Subscribe(topic, receive))
    {
      throw std::runtime_error("Cannot subscribe to marine GNSS topic: " + topic);
    }
    timer = create_wall_timer(std::chrono::milliseconds(50), [this] { Publish(); });
  }

private:
  void Publish()
  {
    std::lock_guard<std::mutex> lock(receiver->mutex);
    auto & input = receiver->input;
    const double time = now().seconds();
    if (time < lastTime)
    {
      input.Invalidate();
      lastSent = -1;
    }
    lastTime = time;
    if (lastSent >= 0 && time - lastSent < 0.1)
    {
      return;
    }
    double height = 0;
    if (input.HasFix(time, maxAge))
    {
      const auto & packet = input.Measurement();
      height = geoid(packet[4], packet[5]);
    }
    auto message = input.Message(time, maxAge, height);
    // Preserve one transmission per real measurement; publish explicit no-fix
    // on startup, submersion, stale data, malformed packets and clock rewind.
    const double stamp = message.header.stamp.sec + message.header.stamp.nanosec * 1e-9;
    if (
      message.fix_type != message.GPS_FIX_TYPE_3D_FIX || stamp != sentStamp ||
      input.Epoch() != sentEpoch)
    {
      publisher->publish(message);
      sentStamp = stamp;
      sentEpoch = input.Epoch();
    }
    lastSent = time;
  }
  std::shared_ptr<State> receiver = std::make_shared<State>();
  GeographicLib::Geoid geoid;
  double maxAge = 1, lastTime = 0, lastSent = -1, sentStamp = -1, sentEpoch = -1;
  rclcpp::Publisher<mavros_msgs::msg::GPSINPUT>::SharedPtr publisher;
  rclcpp::TimerBase::SharedPtr timer;
  // In-flight transport callbacks retain their own state during shutdown.
  gz::transport::Node transport;
};
}  // namespace posim

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  try
  {
    rclcpp::spin(std::make_shared<posim::MarineGnssInput>());
  }
  catch (const std::exception & error)
  {
    RCLCPP_FATAL(rclcpp::get_logger("marine_gnss_input"), "%s", error.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
