// SPDX-License-Identifier: Apache-2.0
#include <OgreHlms.h>
#include <OgreItem.h>
#include <OgreLogManager.h>
#include <OgreMaterialManager.h>
#include <OgreSubItem.h>
#include <Pbs/OgreHlmsPbsDatablock.h>
#include <cstdlib>
#include <fstream>
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/ogre2/Ogre2DynamicRenderable.hh>
#include <gz/rendering/ogre2/Ogre2Material.hh>
#include <iostream>
#include <stdexcept>
#ifdef POSIM_TEST_WAVES
#include <gz/rendering/ogre2/Ogre2RenderEngine.hh>
#include "Ogre2DynamicMesh.hh"
using DynamicGeometry = gz::rendering::Ogre2DynamicMesh;
#else
using DynamicGeometry = gz::rendering::Ogre2DynamicRenderable;
#endif

class GeometryLog : public Ogre::LogListener
{
public:
  bool failed = false;
  void messageLogged(
    const Ogre::String & message, Ogre::LogMessageLevel, bool, const Ogre::String &,
    bool &) override
  {
    if (
      message.find("Deleting mapped buffer") != std::string::npos ||
      message.find("fixed function") != std::string::npos)
    {
      failed = true;
      std::cerr << message << std::endl;
    }
  }
};

int main()
{
#ifdef POSIM_TEST_WAVES
  // This class test links Wave Sim directly. Both libraries export GzPluginHook,
  // so initialise the engine directly instead of testing plugin discovery here.
  auto * engine = gz::rendering::Ogre2RenderEngine::Instance();
  if (!engine->Load() || !engine->Init())
  {
    throw std::runtime_error("Ogre2 could not start");
  }
#else
  auto * engine = gz::rendering::engine("ogre2");
  if (!engine)
  {
    throw std::runtime_error("Ogre2 could not start");
  }
#endif
  auto scene = engine->CreateScene("material_regression");
  if (const char * expected = std::getenv("POSIM_EXPECT_RENDERING_PREFIX"))
  {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    unsigned int found = 0;
    while (std::getline(maps, line))
    {
      if (line.find("/libgz-rendering") == std::string::npos)
      {
        continue;
      }
      if (line.find(std::string(expected) + "/lib/libgz-rendering") == std::string::npos)
      {
        throw std::runtime_error("Unexpected loaded renderer: " + line);
      }
      ++found;
    }
    if (!found)
    {
      throw std::runtime_error("Renderer process mappings were not found");
    }
    std::cout << "Loaded rendering libraries from " << expected << std::endl;
  }
  auto material =
    std::dynamic_pointer_cast<gz::rendering::Ogre2Material>(scene->CreateMaterial("dynamic_pbs"));
  auto * pbs = material->Datablock();
  // Deliberately collide with the active PBS name. Name lookup must not change
  // the material type while resizing or updating a dynamic renderable.
  auto pointMaterial = Ogre::MaterialManager::getSingleton().getByName("PointCloudPoint");
  if (!pointMaterial)
  {
    throw std::runtime_error("PointCloudPoint shader is missing");
  }
  auto legacy = pointMaterial->clone(*pbs->getNameStr());
  GeometryLog geometryLog;
  auto * ogreLog = Ogre::LogManager::getSingleton().getDefaultLog();
  ogreLog->addListener(&geometryLog);
  {
    DynamicGeometry geometry(scene);
    geometry.SetOperationType(gz::rendering::MT_TRIANGLE_LIST);
    geometry.SetMaterial(material, false);
    auto * item = dynamic_cast<Ogre::Item *>(geometry.OgreObject());
    for (unsigned int count : {3u, 12u, 3u, 24u})
    {
      geometry.Clear();
      for (unsigned int i = 0; i < count; ++i)
      {
        geometry.AddPoint(i % 3, (i / 3) % 3, 0);
      }
      geometry.Update();
      if (item->getSubItem(0)->getDatablock() != pbs)
      {
        throw std::runtime_error("PBS material changed during buffer update");
      }
    }
    item->getSubItem(0)->setMaterial(legacy);
    item->setCastShadows(false);
    geometry.SetPoint(0, {0, 0, 1});
    geometry.Update();
    auto * sub = item->getSubItem(0);
    if (
      sub->getDatablock()->getCreator()->getType() != Ogre::HLMS_LOW_LEVEL ||
      sub->getMaterial() != legacy || item->getCastShadows())
    {
      throw std::runtime_error("Legacy material or shadow state was lost");
    }
    // Return to PBS, then destroy with a persistently mapped vertex buffer.
    geometry.SetMaterial(material, false);
    geometry.SetPoint(0, {0, 0, 2});
    geometry.Update();
    if (item->getSubItem(0)->getDatablock() != pbs)
    {
      throw std::runtime_error("Legacy-to-PBS transition failed");
    }
  }
  ogreLog->removeListener(&geometryLog);
  if (geometryLog.failed)
  {
    throw std::runtime_error("Dynamic geometry logged a material/buffer failure");
  }
  scene->DestroyMaterial(material);
  material.reset();
  Ogre::MaterialManager::getSingleton().remove(legacy->getName());
  legacy.reset();
  pointMaterial.reset();
  engine->DestroyScene(scene);
  scene.reset();
#ifdef POSIM_TEST_WAVES
  engine->Destroy();
#else
  gz::rendering::unloadEngine("ogre2");
#endif
  std::cout << "PBS collision, resize, legacy transition and teardown passed\n";
}
