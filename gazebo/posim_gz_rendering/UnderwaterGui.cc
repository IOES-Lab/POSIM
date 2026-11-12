// SPDX-License-Identifier: Apache-2.0
#include <gz/msgs/double_v.pb.h>
#include <gz/msgs/empty.pb.h>
#include <QLibrary>
#include <QThread>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <gz/common/Console.hh>
#include <gz/gui/Application.hh>
#include <gz/gui/GuiEvents.hh>
#include <gz/gui/MainWindow.hh>
#include <gz/plugin/Register.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Scene.hh>
#include <gz/sim/gui/GuiSystem.hh>
#include <gz/transport/Node.hh>
#include <memory>
#include <mutex>
#include "Appearance.hh"
#include "RuntimePaths.hh"

namespace posim
{
class UnderwaterGui final : public gz::sim::GuiSystem
{
  struct State
  {
    std::mutex mutex;
    Appearance appearance = Defaults();
  };
  std::shared_ptr<State> state = std::make_shared<State>();
  std::atomic<double> time{0};
  std::chrono::steady_clock::time_point nextRequest{};
  std::string service;
  gz::transport::Node node;
  std::weak_ptr<void> renderer;
  using UpdateRenderer =
    unsigned (*)(void *, const gz::rendering::ScenePtr &, const Appearance &, double);
  UpdateRenderer updateRenderer = nullptr;
  bool failed = false;

  void LoadConfig(const tinyxml2::XMLElement * config) override
  {
    auto nonce = config->FirstChildElement("session_nonce");
    if (!nonce || !nonce->GetText())
    {
      gzerr << "Underwater GUI requires session_nonce\n";
      return;
    }
    service = std::string("/posim/appearance/") + nonce->GetText() + "/state";
    gz::gui::App()->findChild<gz::gui::MainWindow *>()->installEventFilter(this);
  }

  void Update(const gz::sim::UpdateInfo & info, gz::sim::EntityComponentManager &) override
  {
    time = std::chrono::duration<double>(info.simTime).count();
    const auto now = std::chrono::steady_clock::now();
    if (service.empty() || now < nextRequest)
    {
      return;
    }
    nextRequest = now + std::chrono::seconds(1);
    // Copy the server's applied settings; no separate GUI optics controls.
    const auto shared = state;
    std::function<void(const gz::msgs::Double_V &, bool)> callback =
      [shared](const auto & response, bool success)
    {
      if (!success || response.data_size() != 16)
      {
        return;
      }
      Appearance appearance;
      std::copy_n(response.data().begin(), appearance.size(), appearance.begin());
      if (Valid(appearance))
      {
        std::lock_guard<std::mutex> lock(shared->mutex);
        shared->appearance = appearance;
      }
    };
    node.Request(service, gz::msgs::Empty(), callback);
  }

  bool eventFilter(QObject * object, QEvent * event) override
  {
    if (event->type() == gz::gui::events::PreRender::kType && !failed)
    {
      Render();
    }
    return QObject::eventFilter(object, event);
  }

  void Render()
  {
    auto scene = gz::rendering::sceneFromFirstRenderEngine();
    if (!scene)
    {
      return;
    }
    try
    {
      if (scene->Engine()->Name() != "ogre2")
      {
        throw std::runtime_error("underwater_requires_ogre2");
      }
      auto handle = renderer.lock();
      if (!handle)
      {
        const auto directory = LibraryDirectory();
        auto library = std::make_shared<QLibrary>(QString::fromStdString(
          (directory / "posim_underwater_render" POSIM_LIBRARY_SUFFIX).string()));
        auto create =
          reinterpret_cast<void * (*)(const char *)>(library->resolve("posimCreateUnderwater"));
        auto destroy =
          reinterpret_cast<void (*)(void *)>(library->resolve("posimDestroyUnderwater"));
        updateRenderer =
          reinterpret_cast<UpdateRenderer>(library->resolve("posimUpdateUnderwater"));
        if (!create || !destroy || !updateRenderer)
        {
          throw std::runtime_error("underwater_render_library_unavailable");
        }
        const auto pieces = directory.parent_path() / "share/posim/underwater";
        handle = std::shared_ptr<void>(
          create(pieces.string().c_str()), [library, destroy](void * value) { destroy(value); });
        renderer = handle;
        // The scene restores its shader hooks during destruction. Keep the
        // helper alive until its render thread exits, even if the GUI plugin
        // is removed first. Do not destroy Ogre objects on Qt's UI thread.
        auto thread = QThread::currentThread();
        QObject::connect(
          thread, &QThread::finished, thread, [handle]() mutable { handle.reset(); },
          Qt::DirectConnection);
        gzmsg << "Underwater optics attached to the Gazebo world view\n";
      }
      Appearance appearance;
      {
        std::lock_guard<std::mutex> lock(state->mutex);
        appearance = state->appearance;
      }
      updateRenderer(handle.get(), scene, appearance, time.load());
    }
    catch (const std::exception & error)
    {
      failed = true;
      gzerr << "Underwater GUI disabled: " << error.what() << "\n";
    }
  }
};
}  // namespace posim
GZ_ADD_PLUGIN(posim::UnderwaterGui, gz::gui::Plugin)
