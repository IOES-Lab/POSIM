// SPDX-License-Identifier: Apache-2.0
#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#endif
#include <chrono>
#include <gz/common/Event.hh>
#include <gz/rendering/Geometry.hh>
#include <gz/rendering/Material.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/Sensor.hh>
#include <gz/rendering/Visual.hh>
#include <iostream>
#include <sdf/Link.hh>
#include <sdf/Model.hh>
#include <sdf/Root.hh>
#include <sdf/Sensor.hh>
#include <set>
#include <thread>
#include "MultibeamSonarSensor.hh"

int main(int argc, char ** argv)
{
  if (argc != 2)
  {
    return 2;
  }
#ifdef __APPLE__
  @autoreleasepool
#endif
  {
#ifdef __APPLE__
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyProhibited];
    auto * engine = gz::rendering::engine("ogre2", {{"metal", "1"}});
#else
    auto * engine = gz::rendering::engine("ogre2", {{"headless", "1"}});
#endif
    if (!engine)
    {
      return 3;
    }
    auto makeScene = [&](const std::string & name)
    {
      auto scene = engine->CreateScene(name);
      scene->SetAmbientLight(1, 1, 1);
      auto wall = scene->CreateVisual("wall");
      wall->AddGeometry(scene->CreateBox());
      wall->SetLocalPosition(4.5, 0, 0);
      wall->SetLocalScale(1, 30, 30);
      auto material = scene->CreateMaterial();
      material->SetAmbient(1, 1, 1);
      material->SetDiffuse(1, 1, 1);
      wall->SetMaterial(material);
      scene->RootVisual()->AddChild(wall);
      return scene;
    };
    auto a = makeScene("lifecycle_a"), b = makeScene("lifecycle_b");
    sdf::Root root;
    if (!root.Load(argv[1]).empty() || !root.Model())
    {
      return 4;
    }
    auto config = *root.Model()->LinkByIndex(0)->SensorByIndex(0);
    config.SetTopic(
      "/sonar_lifecycle_" +
      std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    {
      gz::sensors::MultibeamSonarSensor pending;
      pending.SetScene(a);
      if (!pending.RenderingSensors().empty())
      {
        return 6;
      }
      pending.SetScene(nullptr);
      pending.SetScene(a);
      if (!pending.Load(config) || pending.RenderingSensors().empty())
      {
        return 7;
      }
      pending.SetScene(nullptr);
      if (!pending.RenderingSensors().empty() || a->SensorCount() != 0)
      {
        return 8;
      }
      std::cout << "RESULT {\"phase\":\"scene_before_load\",\"passed\":1}" << std::endl;
    }
    {
      auto invalidElement = config.Element()->Clone();
      auto properties = invalidElement->GetElement("gz:multibeam_sonar");
      properties->RemoveChild(properties->GetElement("ray"));
      sdf::Sensor invalid;
      if (!invalid.Load(invalidElement).empty())
      {
        return 9;
      }
      gz::sensors::MultibeamSonarSensor failed;
      if (!failed.Load(invalid))
      {
        return 10;
      }
      failed.SetScene(a);
      if (!failed.RenderingSensors().empty() || a->SensorCount() != 0)
      {
        return 11;
      }
      failed.SetScene(nullptr);
      std::cout << "RESULT {\"phase\":\"invalid_configuration\",\"passed\":1}" << std::endl;
    }
    auto context = std::make_shared<rclcpp::Context>();
    context->init(0, nullptr);
    auto observer =
      std::make_shared<rclcpp::Node>("lifecycle_observer", rclcpp::NodeOptions().context(context));
    rclcpp::ExecutorOptions options;
    options.context = context;
    rclcpp::executors::SingleThreadedExecutor executor(options);
    executor.add_node(observer);
    std::set<int64_t> stamps;
    int64_t minimumStamp = 0;
    bool valid = true;
    auto sub = observer->create_subscription<marine_acoustic_msgs::msg::ProjectedSonarImage>(
      config.Topic() + "/sonar_image_raw", rclcpp::SensorDataQoS(),
      [&](marine_acoustic_msgs::msg::ProjectedSonarImage::ConstSharedPtr msg)
      {
        valid &=
          msg->image.beam_count > 0 && !msg->ranges.empty() &&
          msg->image.data.size() == msg->image.beam_count * msg->ranges.size() * sizeof(float);
        const int64_t stamp =
          int64_t(msg->header.stamp.sec) * 1000000000 + msg->header.stamp.nanosec;
        if (stamp > minimumStamp)
        {
          stamps.insert(stamp);
        }
      });
    auto sensor = std::make_unique<gz::sensors::MultibeamSonarSensor>();
    gz::sensors::WorldState state;
    if (!sensor->Load(config))
    {
      return 5;
    }
    sensor->SetEntity(1);
    sensor->SetWorldState(state);
    int64_t sim = 0;
    auto attach = [&](gz::rendering::ScenePtr scene)
    {
      for (auto camera : sensor->RenderingSensors())
      {
        scene->RootVisual()->AddChild(camera);
      }
    };
    auto collect = [&](const char * phase)
    {
      stamps.clear();
      minimumStamp = sim;
      bool updated = true;
      const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(8);
      while (stamps.size() < 3 && std::chrono::steady_clock::now() < deadline)
      {
        sim += 500000000;
        updated = static_cast<gz::sensors::Sensor *>(sensor.get())
                    ->Update(std::chrono::nanoseconds(sim), false);
        if (!updated)
        {
          break;
        }
        sensor->PostUpdate(std::chrono::nanoseconds(sim));
        executor.spin_some();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
      std::cout << "RESULT {\"phase\":\"" << phase << "\",\"updated\":" << updated
                << ",\"raw_stamps\":" << stamps.size() << ",\"valid\":" << valid
                << ",\"has_connections\":" << sensor->HasConnections()
                << ",\"a_sensors\":" << a->SensorCount() << ",\"b_sensors\":" << b->SensorCount()
                << "}" << std::endl;
      return updated && stamps.size() >= 3 && valid;
    };
    sensor->SetScene(a);
    attach(a);
    bool passed = collect("initial_raw_only");
    auto same = sensor->RenderingSensors().front().get();
    sensor->SetScene(a);
    passed &= sensor->RenderingSensors().front().get() == same;
    std::weak_ptr<gz::rendering::Sensor> old;
    if (!sensor->RenderingSensors().empty())
    {
      old = sensor->RenderingSensors().front();
    }
    sensor->SetScene(b);
    attach(b);
    passed = collect("scene_swap_raw_only") && passed;
    std::cout << "RESULT {\"phase\":\"old_sensor_released\",\"expired\":" << old.expired() << "}"
              << std::endl;
    passed &= old.expired();
    sensor->SetScene(nullptr);
    bool stopped = !sensor->Update(std::chrono::nanoseconds(++sim));
    std::cout << "RESULT {\"phase\":\"null_scene\",\"stopped\":" << stopped
              << ",\"a_sensors\":" << a->SensorCount() << ",\"b_sensors\":" << b->SensorCount()
              << "}" << std::endl;
    sensor->SetScene(a);
    attach(a);
    passed = collect("restored_raw_only") && stopped && passed;
    sensor.reset();
    std::cout << "RESULT {\"phase\":\"destroyed\",\"a_sensors\":" << a->SensorCount()
              << ",\"b_sensors\":" << b->SensorCount() << "}" << std::endl;
    passed &= a->SensorCount() == 0 && b->SensorCount() == 0;
    executor.remove_node(observer);
    sub.reset();
    observer.reset();
    context->shutdown("control complete");
    engine->DestroyScene(a);
    engine->DestroyScene(b);
    return passed ? 0 : 1;
  }
}
