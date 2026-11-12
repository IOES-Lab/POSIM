// SPDX-License-Identifier: Apache-2.0
// Exercise real RViz/Qt close events and timers, not a source-text approximation.
#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <QPointer>
#include <QTimer>
#include <QUuid>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "rviz_common/display.hpp"
#include "rviz_common/display_group.hpp"
#include "rviz_common/properties/status_list.hpp"
#include "rviz_common/ros_integration/ros_client_abstraction.hpp"
#include "rviz_common/ros_integration/ros_node_abstraction_iface.hpp"
#include "rviz_common/view_manager.hpp"
#include "rviz_common/visualization_frame.hpp"
#include "rviz_common/visualization_manager.hpp"
#include "sensor_msgs/msg/image.hpp"

namespace
{
void require(bool condition, const std::string & message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

void pump(int milliseconds)
{
  QEventLoop loop;
  QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
  loop.exec();
}

void checkLibrary(const std::filesystem::path & prefix)
{
  std::ifstream maps("/proc/self/maps");
  std::string line;
  bool found = false;
  while (std::getline(maps, line))
  {
    if (line.find("/librviz_common.so") == std::string::npos)
    {
      continue;
    }
    auto path = std::filesystem::canonical(line.substr(line.find('/')));
    const auto relative = path.lexically_relative(std::filesystem::canonical(prefix));
    require(!relative.empty() && *relative.begin() != "..", "Wrong RViz library: " + path.string());
    found = true;
  }
  require(found, "No loaded librviz_common found");
}

bool closeWithAnswer(rviz_common::VisualizationFrame & frame, QMessageBox::StandardButton answer)
{
  bool answered = false;
  QTimer responder;
  QObject::connect(
    &responder, &QTimer::timeout,
    [&]()
    {
      auto * box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
      if (box && box->standardButtons().testFlag(answer))
      {
        answered = true;
        responder.stop();
        box->done(answer);
      }
    });
  responder.start(25);
  const bool closed = frame.close();
  require(answered, "Expected unsaved-changes confirmation was not shown");
  return closed;
}

int imagesReceived(rviz_common::VisualizationManager * manager, int index)
{
  auto * display = manager->getRootDisplayGroup()->getDisplayAt(index);
  require(display != nullptr, "Image display was not created");
  rviz_common::properties::StatusList * status = nullptr;
  for (int child = 0; child < display->numChildren(); ++child)
  {
    status = dynamic_cast<rviz_common::properties::StatusList *>(display->childAt(child));
    if (status)
    {
      break;
    }
  }
  require(status != nullptr, "Image display has no status list");
  const auto text = status->subProp("Topic")->getValue().toString();
  bool valid = false;
  const int count = text.section(' ', 0, 0).toInt(&valid);
  require(valid && count > 0, "Image display has not received images: " + text.toStdString());
  return count;
}
}  // namespace

int main(int argc, char ** argv)
{
  if (argc != 3)
  {
    return 2;
  }
  const std::string prefix = argv[1];
  const std::string scenario = argv[2];
  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(false);
  auto client = std::make_unique<rviz_common::ros_integration::RosClientAbstraction>();
  try
  {
    checkLibrary(prefix);
    const auto node_name =
      "posim_rviz_close_" + QUuid::createUuid().toString(QUuid::Id128).toStdString();
    const auto image_topic = "/" + node_name + "/image";
    auto node = client->init(0, nullptr, node_name, false);
    std::weak_ptr<rclcpp::Node> raw_node_weak;
    QPointer<rviz_common::ViewManager> view_owner;
    {
      auto raw_node = node.lock()->get_raw_node();
      raw_node_weak = raw_node;
      auto publisher = raw_node->create_publisher<sensor_msgs::msg::Image>(image_topic, 2);
      sensor_msgs::msg::Image image;
      image.header.frame_id = "map";
      image.height = 120;
      image.width = 160;
      image.encoding = "rgb8";
      image.step = image.width * 3;
      image.data.resize(image.height * image.step, 80);
      QTimer image_timer;
      QObject::connect(
        &image_timer, &QTimer::timeout,
        [&]()
        {
          image.header.stamp = raw_node->now();
          image.data[0]++;
          publisher->publish(image);
        });
      image_timer.start(50);
      // A real config keeps Save deterministic and never alters user files.
      const auto config = std::filesystem::path(std::getenv("HOME")) / "close-test.rviz";
      std::ofstream(config) << "Visualization Manager:\n"
                               "  Class: ''\n"
                               "  Global Options:\n"
                               "    Fixed Frame: map\n"
                               "    Frame Rate: 30\n"
                               "  Displays:\n"
                               "    - Class: rviz_default_plugins/Grid\n"
                               "      Name: Grid\n"
                               "      Enabled: true\n"
                               "    - Class: rviz_default_plugins/Image\n"
                               "      Name: Camera\n"
                               "      Enabled: true\n"
                               "      Topic:\n"
                               "        Value: "
                            << image_topic
                            << "\n"
                               "        Reliability Policy: Best Effort\n"
                               "        Depth: 2\n"
                               "    - Class: rviz_default_plugins/Image\n"
                               "      Name: Sonar\n"
                               "      Enabled: true\n"
                               "      Topic:\n"
                               "        Value: "
                            << image_topic
                            << "\n"
                               "        Reliability Policy: Best Effort\n"
                               "        Depth: 2\n";
      int updates = 0;
      rviz_common::VisualizationFrame frame(node);
      frame.setApp(&app);
      frame.initialize(node, QString::fromStdString(config.string()));
      frame.show();
      auto * manager = frame.getManager();
      view_owner = manager->getViewManager();
      QObject::connect(
        manager, &rviz_common::VisualizationManager::preUpdate, &frame,
        [&updates]() { ++updates; });
      pump(1800);  // Let the post-load timer complete before marking modified.
      require(updates >= 5, "RViz was not updating before close");
      require(
        imagesReceived(manager, 1) >= 3 && imagesReceived(manager, 2) >= 3,
        "Both Image displays must be receiving frames");
      if (const char * maps_path = std::getenv("POSIM_RVIZ_TEST_MAPS"))
      {
        // A finalizer crash can unload the offending DSO before a core captures
        // its mapping. Retain the live map for post-mortem symbol resolution.
        std::ifstream maps("/proc/self/maps");
        std::ofstream(maps_path) << maps.rdbuf();
      }

      frame.setWindowModified(scenario != "unchanged");
      if (scenario == "cancel")
      {
        require(!closeWithAnswer(frame, QMessageBox::Cancel), "Cancel accepted close");
        require(frame.isVisible(), "Cancel hid the frame");
        const int before = updates;
        const int camera_before = imagesReceived(manager, 1);
        const int sonar_before = imagesReceived(manager, 2);
        pump(400);
        require(updates >= before + 3, "Cancel stopped the rendering update timer");
        require(
          imagesReceived(manager, 1) > camera_before && imagesReceived(manager, 2) > sonar_before,
          "Cancel stopped live image processing");
        require(closeWithAnswer(frame, QMessageBox::Discard), "Second close was rejected");
      }
      else if (scenario == "discard")
      {
        require(closeWithAnswer(frame, QMessageBox::Discard), "Discard rejected close");
      }
      else if (scenario == "save")
      {
        require(closeWithAnswer(frame, QMessageBox::Save), "Save rejected close");
      }
      else
      {
        require(scenario == "unchanged", "Unknown scenario");
        frame.setWindowModified(false);
        require(frame.close(), "Unchanged close was rejected");
      }
      const int at_close = updates;
      pump(400);
      require(updates == at_close, "Rendering update timer continued after accepted close");
      require(!frame.isVisible(), "Accepted close left frame visible");
      // Match the stock VisualizerApp destructor: ROS shuts down before frame
      // and display destruction. Do not accidentally hide shutdown failures by
      // giving this test a safer lifecycle than the shipped rviz2 executable.
      image_timer.stop();
      client->shutdown();
    }
    client.reset();
    require(view_owner.isNull(), "ViewManager retained after frame destruction");
    require(node.expired(), "ROS node abstraction retained after client/frame destruction");
    require(raw_node_weak.expired(), "ROS node retained after client/frame destruction");
    for (const auto & task : std::filesystem::directory_iterator("/proc/self/task"))
    {
      std::ifstream stream(task.path() / "comm");
      std::string name;
      std::getline(stream, name);
      // FastDDS owns one process-static SHM watchdog, also present after a
      // plain rclcpp node exits. Participant/transport threads must be gone.
      if (name.rfind("dds.", 0) == 0 && name != "dds.shm.wdog")
      {
        throw std::runtime_error("DDS participant thread remains after owner teardown: " + name);
      }
    }
    std::cout << "PASS: " << scenario << " (selected library, live timer, close semantics)\n";
    return 0;
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL: " << scenario << ": " << error.what() << '\n';
    if (client)
    {
      client->shutdown();
    }
    return 1;
  }
}
