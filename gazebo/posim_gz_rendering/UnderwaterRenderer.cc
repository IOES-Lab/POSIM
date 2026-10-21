// SPDX-License-Identifier: Apache-2.0
#include "UnderwaterRenderer.hh"
#include <algorithm>
#include <cstring>
#include <gz/rendering/Camera.hh>
#include <gz/rendering/Visual.hh>
#include <gz/rendering/Geometry.hh>
#include <gz/rendering/ogre2/Ogre2Material.hh>
#include <gz/rendering/ogre2/Ogre2RenderEngine.hh>
#include <OgreArchiveManager.h>
#include <OgreCamera.h>
#include <OgreHlmsListener.h>
#include <OgreHlmsManager.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <Hlms/Pbs/OgreHlmsPbsDatablock.h>
namespace posim {
struct UnderwaterRenderer::Impl : Ogre::HlmsListener {
 Ogre::Hlms *pbs=nullptr;
 Ogre::ArchiveVec original;
 Ogre::HlmsListener *previous=nullptr;
 Appearance settings=Defaults();
 double time=0;
 unsigned cameras=0;
 explicit Impl(const std::string &pieces){
  auto root=gz::rendering::Ogre2RenderEngine::Instance()->OgreRoot();
  pbs=root->getHlmsManager()->getHlms(Ogre::HLMS_PBS);
  if(!pbs)throw std::runtime_error("underwater_pbs_unavailable");
  previous=pbs->getListener();
  if(dynamic_cast<Impl *>(previous))throw std::runtime_error("underwater_already_installed");
  original=pbs->getPiecesLibraryAsArchiveVec();auto extended=original;
  extended.push_back(Ogre::ArchiveManager::getSingleton().load(pieces,"FileSystem",true));
  pbs->setListener(this);
  try {pbs->reloadFrom(pbs->getDataFolder(),&extended);}
  catch(...){pbs->setListener(previous);throw;}
 }
 ~Impl() {
  if(pbs&&pbs->getListener()==this){pbs->setListener(previous);pbs->reloadFrom(pbs->getDataFolder(),&original);}
 }
 void propertiesMergedPreGenerationStep(Ogre::Hlms *h,const Ogre::HlmsCache &c,const Ogre::HlmsPropertyVec &r,const Ogre::PiecesMap p[Ogre::NumShaderTypes],const Ogre::HlmsPropertyVec &v,const Ogre::QueuedRenderable &q) override {if(previous)previous->propertiesMergedPreGenerationStep(h,c,r,p,v,q);}
 Ogre::uint16 getNumExtraPassTextures(const Ogre::HlmsPropertyVec &p,bool c) const override {return previous?previous->getNumExtraPassTextures(p,c):0;}
 void setupRootLayout(Ogre::RootLayout &r,const Ogre::HlmsPropertyVec &p) const override {if(previous)previous->setupRootLayout(r,p);}
 void shaderCacheEntryCreated(const Ogre::String &s,const Ogre::HlmsCache *h,const Ogre::HlmsCache &c,const Ogre::HlmsPropertyVec &p,const Ogre::QueuedRenderable &q) override {if(previous)previous->shaderCacheEntryCreated(s,h,c,p,q);}
 void preparePassHash(const Ogre::CompositorShadowNode *s,bool c,bool d,Ogre::SceneManager *m,Ogre::Hlms *h) override {if(previous)previous->preparePassHash(s,c,d,m,h);}
 void hlmsTypeChanged(bool c,Ogre::CommandBuffer *b,const Ogre::HlmsDatablock *d,size_t t) override {if(previous)previous->hlmsTypeChanged(c,b,d,t);}
 Ogre::uint32 getPassBufferSize(const Ogre::CompositorShadowNode *s,bool c,bool d,
                                Ogre::SceneManager *m) const override {return (previous?previous->getPassBufferSize(s,c,d,m):0)+(c?0:128);}
 float *preparePassBuffer(const Ogre::CompositorShadowNode *s,bool caster,bool d,
                         Ogre::SceneManager *scene,float *out) override {
  if(previous)out=previous->preparePassBuffer(s,caster,d,scene,out);
  if(caster)return out;
  auto camera=scene->getCamerasInProgress().renderingCamera;
  auto inverse=camera->getViewMatrix(true).inverse();
  for(unsigned i=0;i<3;++i)for(unsigned j=0;j<4;++j)*out++=inverse[i][j];
  for(auto i:{2,4,5,3})*out++=settings[i];
  for(auto i:{6,7,8,9})*out++=settings[i];
  *out++=settings[11];*out++=settings[12];*out++=time;*out++=settings[10];
  for(float v:{.18f,.09f,.06f,0.f})*out++=v;
  for(float v:{.006f,.055f,.065f,1.f})*out++=v;
  return out;
 }
};
UnderwaterRenderer::UnderwaterRenderer(const std::string &pieces):impl(new Impl(pieces)){}
UnderwaterRenderer::~UnderwaterRenderer()=default;
unsigned UnderwaterRenderer::Cameras() const {return impl->cameras;}
void UnderwaterRenderer::Update(const gz::rendering::ScenePtr &scene,const Appearance &settings,double time){
 impl->settings=settings;impl->time=time;impl->cameras=0;
 for(unsigned i=0;i<scene->SensorCount();++i)
  if(std::dynamic_pointer_cast<gz::rendering::Camera>(scene->SensorByIndex(i)))++impl->cameras;
 for(unsigned i=0;i<scene->VisualCount();++i){
  auto v=scene->VisualByIndex(i);const auto &name=v->Name();
  const auto scope=name.rfind("::");
  const auto leaf=name.substr(scope==std::string::npos?0:scope+2);
  const bool seabed=leaf=="seabed"||leaf=="shallow";
  if(!seabed)continue;
  for(unsigned g=0;g<v->GeometryCount();++g){
   auto m=std::dynamic_pointer_cast<gz::rendering::Ogre2Material>(v->GeometryByIndex(g)->Material());
   if(m&&m->Datablock()&&m->Datablock()->getUserValue(0).x!=6421)m->Datablock()->setUserValue(0,Ogre::Vector4(6421,0,0,0));
  }
 }
}
}
extern "C" void *posimCreateUnderwater(const char *path){return new posim::UnderwaterRenderer(path);}
extern "C" unsigned posimUpdateUnderwater(void *p,const gz::rendering::ScenePtr &s,const posim::Appearance &a,double t){
 auto r=static_cast<posim::UnderwaterRenderer *>(p);r->Update(s,a,t);return r->Cameras();
}
extern "C" void posimDestroyUnderwater(void *p){delete static_cast<posim::UnderwaterRenderer *>(p);}
