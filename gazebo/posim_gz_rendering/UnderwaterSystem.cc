// SPDX-License-Identifier: Apache-2.0
#include "Appearance.hh"
#include <atomic>
#include <filesystem>
#include <mutex>
#include <sstream>
#include <gz/plugin/Register.hh>
#include <gz/sim/System.hh>
#include <gz/sim/rendering/Events.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/transport/Node.hh>
#include <gz/msgs/double_v.pb.h>
#include <gz/msgs/empty.pb.h>
#include <gz/common/Console.hh>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
namespace posim {
class Underwater final : public gz::sim::System,public gz::sim::ISystemConfigure,public gz::sim::ISystemPostUpdate {
 std::mutex mutex;
 Appearance requested=Defaults(),applied=Defaults(),staged=Defaults();
 bool stagedReady=false;
 unsigned stagedCameras=0;
 double time=0;
 unsigned cameras=0;
 bool ready=false,failed=false;
 void *library=nullptr,*renderer=nullptr;
 using Create=void *(*)(const char *);
 using Update=unsigned (*)(void *,const gz::rendering::ScenePtr &,const Appearance &,double);
 using Destroy=void (*)(void *);
 Create create=nullptr;Update update=nullptr;Destroy destroy=nullptr;
 std::string path,pieces;
 gz::transport::Node node;
 gz::common::ConnectionPtr renderConnection,postRenderConnection,teardownConnection;
 void Close(){if(renderer&&destroy){destroy(renderer);renderer=nullptr;}}
 public:
 ~Underwater() override {
  renderConnection.reset();postRenderConnection.reset();teardownConnection.reset();
  // The rendering resources are destroyed by RenderTeardown, on their owner thread.
  // Keep the helper mapped until process exit if the engine omitted teardown.
  if(!renderer&&library){
#ifdef _WIN32
   FreeLibrary(static_cast<HMODULE>(library));
#else
   dlclose(library);
#endif
  }
 }
 void Configure(const gz::sim::Entity &,const std::shared_ptr<const sdf::Element> &sdf,
                gz::sim::EntityComponentManager &,gz::sim::EventManager &events) override {
  if(!sdf->HasElement("renderer_library")||!sdf->HasElement("shader_directory")||!sdf->HasElement("session_nonce")){
   gzerr<<"Underwater requires renderer_library, shader_directory and session_nonce\n";return;
  }
  path=sdf->Get<std::string>("renderer_library");pieces=sdf->Get<std::string>("shader_directory");
  if(sdf->HasElement("initial_settings")){
   std::istringstream stream(sdf->Get<std::string>("initial_settings"));Appearance initial;
   for(auto &v:initial)if(!(stream>>v))throw std::runtime_error("invalid_initial_appearance");
   if(!Valid(initial))throw std::runtime_error("invalid_initial_appearance");
   requested=applied=initial;
  }
  auto service="/posim/appearance/"+sdf->Get<std::string>("session_nonce");
  node.Advertise(service+"/set",&Underwater::Set,this);
  node.Advertise(service+"/state",&Underwater::State,this);
  renderConnection=events.Connect<gz::sim::events::PreRender>([this]{Render();});
  postRenderConnection=events.Connect<gz::sim::events::PostRender>([this]{
   std::lock_guard<std::mutex> lock(mutex);
   if(stagedReady&&!failed){applied=staged;cameras=stagedCameras;ready=cameras>0;stagedReady=false;}
  });
  teardownConnection=events.Connect<gz::sim::events::RenderTeardown>([this]{Close();std::lock_guard<std::mutex> lock(mutex);ready=false;});
 }
 bool Set(const gz::msgs::Double_V &request,gz::msgs::Double_V &response){
  std::lock_guard<std::mutex> lock(mutex);
  if(request.data_size()!=13)return false;
  Appearance a;std::copy(request.data().begin(),request.data().end(),a.begin());
  if(!Valid(a)||a[1]<requested[1]||a[1]>requested[1]+1||(a[1]==requested[1]&&a!=requested))return false;
  requested=a;for(auto v:requested)response.add_data(v);return true;
 }
 bool State(const gz::msgs::Empty &,gz::msgs::Double_V &response){
  std::lock_guard<std::mutex> lock(mutex);
  for(auto v:applied)response.add_data(v);
  response.add_data(ready?1:0);response.add_data(cameras);response.add_data(failed?1:0);return true;
 }
 void PostUpdate(const gz::sim::UpdateInfo &info,const gz::sim::EntityComponentManager &) override {
  std::lock_guard<std::mutex> lock(mutex);time=std::chrono::duration<double>(info.simTime).count();
 }
 void Render(){
  if(failed)return;
  auto scene=gz::rendering::sceneFromFirstRenderEngine();if(!scene)return;
  try{
   if(scene->Engine()->Name()!="ogre2")throw std::runtime_error("underwater_requires_ogre2");
   if(!renderer){
#ifdef _WIN32
    library=LoadLibraryA(path.c_str());
    auto symbol=[this](const char *name){return reinterpret_cast<void *>(GetProcAddress(static_cast<HMODULE>(library),name));};
#else
    library=dlopen(path.c_str(),RTLD_NOW|RTLD_LOCAL);
    auto symbol=[this](const char *name){return dlsym(library,name);};
#endif
    if(!library)throw std::runtime_error("underwater_render_library_unavailable");
    create=reinterpret_cast<Create>(symbol("posimCreateUnderwater"));
    update=reinterpret_cast<Update>(symbol("posimUpdateUnderwater"));
    destroy=reinterpret_cast<Destroy>(symbol("posimDestroyUnderwater"));
    if(!create||!update||!destroy)throw std::runtime_error("underwater_render_abi_mismatch");
    renderer=create(pieces.c_str());
   }
   Appearance a;double t;
   {std::lock_guard<std::mutex> lock(mutex);a=requested;t=time;}
   auto count=update(renderer,scene,a,t);
   {std::lock_guard<std::mutex> lock(mutex);staged=a;stagedCameras=count;stagedReady=true;}
  }catch(const std::exception &error){
   std::lock_guard<std::mutex> lock(mutex);failed=true;ready=false;gzerr<<"Underwater disabled: "<<error.what()<<"\n";
  }
 }
};
}
GZ_ADD_PLUGIN(posim::Underwater,gz::sim::System,posim::Underwater::ISystemConfigure,posim::Underwater::ISystemPostUpdate)
GZ_ADD_PLUGIN_ALIAS(posim::Underwater,"posim::Underwater")
