// SPDX-License-Identifier: Apache-2.0
#include <gz/transport/NodeShared.hh>
#include <gz/transport/RepHandler.hh>
#include <gz/transport/TopicUtils.hh>
#include "../UnderwaterSystem.cc"

int main()
{
  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  auto config = std::make_shared<sdf::Element>();
  config->SetName("plugin");
  // No rendering event is emitted: service lifetime must not require a GPU.
  for (const auto & entry :
       {std::pair{"renderer_library", "unused"}, std::pair{"shader_directory", "unused"},
        std::pair{"session_nonce", "system_lifecycle_test"}})
  {
    auto value = std::make_shared<sdf::Element>();
    value->SetName(entry.first);
    value->AddValue("string", entry.second, true);
    config->InsertElement(value);
  }
  auto system = std::make_unique<posim::Underwater>();
  system->Configure(0, config, ecm, events);
  gz::msgs::Empty empty;
  gz::msgs::Double_V request, response;
  const auto acquire = [&](const std::string & suffix, const auto & message)
  {
    gz::transport::NodeOptions options;
    std::string topic;
    gz::transport::TopicUtils::FullyQualifiedName(
      options.Partition(), options.NameSpace(), "/posim/appearance/system_lifecycle_test/" + suffix,
      topic);
    auto transport = gz::transport::NodeShared::Instance();
    gz::transport::IRepHandlerPtr handler;
    std::lock_guard<std::recursive_mutex> lock(transport->mutex);
    transport->Repliers().FirstHandler(
      topic, std::string(message.GetTypeName()), std::string(response.GetTypeName()), handler);
    return handler;
  };
  const auto get = acquire("state", empty);
  const auto set = acquire("set", request);
  if (!get || !set || !get->RunLocalCallback(empty, response) || response.data_size() != 16)
  {
    return 1;
  }
  const auto defaults = posim::Defaults();
  for (unsigned i = 0; i < defaults.size(); ++i)
  {
    if (response.data(i) != defaults[i])
    {
      return 2;
    }
    request.add_data(defaults[i]);
  }
  request.set_data(1, 1);
  request.set_data(10, 1);
  response.Clear();
  if (
    !set->RunLocalCallback(request, response) || response.data_size() != 13 ||
    response.SerializeAsString() != request.SerializeAsString())
  {
    return 3;
  }
  response.Clear();
  if (!set->RunLocalCallback(request, response))
  {
    return 4;
  }
  request.set_data(10, 0);  // A conflicting request at the same revision is rejected.
  response.Clear();
  if (set->RunLocalCallback(request, response))
  {
    return 5;
  }
  system.reset();
  if (acquire("state", empty) || acquire("set", request))
  {
    return 6;
  }
  // Reproduce a request thread that already acquired handlers before shutdown.
  response.Clear();
  if (
    get->RunLocalCallback(empty, response) || set->RunLocalCallback(request, response) ||
    response.data_size() != 0)
  {
    return 7;
  }
  return 0;
}
