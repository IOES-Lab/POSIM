#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gz/msgs/stringmsg.pb.h>
#include <gz/transport/Node.hh>

using namespace std::chrono_literals;

struct Counts
{
  std::atomic<unsigned long> anchor{0};
  std::atomic<unsigned long> churn{0};
};

int main(int argc, char ** argv)
{
  if (argc != 3)
  {
    return 2;
  }
  const std::string mode(argv[1]);
  if (mode != "publish" && mode != "subscribe")
  {
    return 2;
  }
  const bool publish = mode == "publish";
  const int seconds = std::stoi(argv[2]);
  if (seconds < 3 || seconds > 300)
  {
    return 2;
  }
  const auto stop = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
  constexpr int topics = 24;
  auto topic = [](int i) { return "/posim/churn/topic/" + std::to_string(i); };
  // Queued callbacks retain only the counters, never the Node or stack locals.
  const auto received = std::make_shared<Counts>();
  const std::function<void(const gz::msgs::StringMsg &)> anchorCallback =
    [received](const gz::msgs::StringMsg &) { ++received->anchor; };
  const std::function<void(const gz::msgs::StringMsg &)> churnCallback =
    [received](const gz::msgs::StringMsg &) { ++received->churn; };
  unsigned long rounds = 0;
  if (publish)
  {
    gz::transport::Node node;
    std::vector<gz::transport::Node::Publisher> publishers;
    for (int i = 0; i < topics; ++i)
    {
      publishers.push_back(node.Advertise<gz::msgs::StringMsg>(topic(i)));
      if (!publishers.back())
      {
        return 3;
      }
    }
    gz::msgs::StringMsg msg;
    msg.set_data(std::string(2048, 'x'));
    while (std::chrono::steady_clock::now() < stop)
    {
      for (auto & pub : publishers)
      {
        if (!pub.Publish(msg))
        {
          return 4;
        }
      }
      ++rounds;
      std::this_thread::sleep_for(100us);
    }
  }
  else
  {
    // Keep a transport connection while destroying the last subscriber to
    // each churn topic. Separate publisher process prevents local delivery.
    gz::transport::Node anchor;
    if (!anchor.Subscribe(topic(0), anchorCallback))
    {
      return 3;
    }
    std::this_thread::sleep_for(2s);
    while (std::chrono::steady_clock::now() < stop)
    {
      {
        gz::transport::Node node;
        for (int i = 1; i < topics; ++i)
        {
          if (!node.Subscribe(topic(i), churnCallback))
          {
            return 3;
          }
        }
        std::this_thread::sleep_for(100us);
      }
      ++rounds;
    }
  }
  std::cout << "rounds=" << rounds << " anchor_received=" << received->anchor
            << " churn_received=" << received->churn << std::endl;
  return rounds > 0 && (publish || (received->anchor > 0 && received->churn > 0)) ? 0 : 1;
}
