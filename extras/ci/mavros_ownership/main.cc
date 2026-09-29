#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <mavros/mavros_router.hpp>

namespace mavros::router
{
// Use the upstream test friend, not timers or a remote autopilot, to establish
// exactly which endpoints the Router owns before releasing its external owner.
class TestRouter
{
public:
  static std::weak_ptr<Endpoint> add(const Router::SharedPtr & router, bool ros)
  {
    auto request = std::make_shared<mavros_msgs::srv::EndpointAdd::Request>();
    auto response = std::make_shared<mavros_msgs::srv::EndpointAdd::Response>();
    request->type = ros ? request->TYPE_UAS : request->TYPE_FCU;
    request->url = ros ? "/posim_ownership" : "udp://127.0.0.1:0@127.0.0.1:19999";
    router->add_endpoint(request, response);
    if (!response->successful)
    {
      throw std::runtime_error(response->reason);
    }
    return router->endpoints.at(response->id);
  }
};
}  // namespace mavros::router

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
  int failures = 0;
  for (int mode = 0; mode < 3; ++mode)
  {
    auto router = std::make_shared<mavros::router::Router>();
    std::vector<std::weak_ptr<mavros::router::Endpoint>> endpoints;
    if (mode >= 1)
    {
      endpoints.push_back(mavros::router::TestRouter::add(router, true));
    }
    if (mode >= 2)
    {
      endpoints.push_back(mavros::router::TestRouter::add(router, false));
    }
    std::weak_ptr<mavros::router::Router> weak = router;
    router.reset();
    bool released = weak.expired();
    for (const auto & endpoint : endpoints)
    {
      released = released && endpoint.expired();
    }
    std::cout << "mode=" << mode << " router_expired=" << weak.expired()
              << " all_endpoints_expired=" << released << std::endl;
    failures += !released;
  }
  rclcpp::shutdown();
  return failures ? 1 : 0;
}
