#include <atomic>
#include <gz/plugin/Register.hh>
#include <gz/sim/EventManager.hh>
#include <gz/sim/System.hh>
#include <gz/sim/rendering/Events.hh>
#include <iostream>

class RenderDemand : public gz::sim::System,
                     public gz::sim::ISystemConfigure,
                     public gz::sim::ISystemPostUpdate
{
  gz::sim::EventManager * events{};
  gz::common::ConnectionPtr connection;
  std::atomic<bool> reported{false};
  bool configureDemand{};

public:
  void Configure(
    const gz::sim::Entity &, const std::shared_ptr<const sdf::Element> & sdf,
    gz::sim::EntityComponentManager &, gz::sim::EventManager & manager) override
  {
    events = &manager;
    configureDemand = sdf->Get<std::string>("phase", "post").first == "configure";
    connection = manager.Connect<gz::sim::events::PreRender>(
      [this]()
      {
        if (!reported.exchange(true))
        {
          std::cout << "RENDER_DEMAND_SERVED" << std::endl;
        }
      });
    if (configureDemand)
    {
      events->Emit<gz::sim::events::ForceRender>();
    }
  }
  void PostUpdate(const gz::sim::UpdateInfo &, const gz::sim::EntityComponentManager &) override
  {
    if (!configureDemand && !reported)
    {
      events->Emit<gz::sim::events::ForceRender>();
    }
  }
};
GZ_ADD_PLUGIN(RenderDemand, gz::sim::System, gz::sim::ISystemConfigure, gz::sim::ISystemPostUpdate)
