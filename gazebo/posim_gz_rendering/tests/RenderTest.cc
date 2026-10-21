// SPDX-License-Identifier: Apache-2.0
#include <chrono>
#include <cstring>
#include <fstream>
#include <gz/rendering/Camera.hh>
#include <gz/rendering/DepthCamera.hh>
#include <gz/rendering/Geometry.hh>
#include <gz/rendering/Light.hh>
#include <gz/rendering/Material.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Visual.hh>
#include <iostream>
#include <vector>
#include "UnderwaterRenderer.hh"
int main(int argc, char ** argv)
{
  if (argc != 4)
  {
    return 2;
  }
#ifdef __APPLE__
  auto engine = gz::rendering::engine("ogre2", {{"metal", "1"}});
#else
  auto engine = gz::rendering::engine("ogre2", {{"headless", "1"}});
#endif
  if (!engine)
  {
    return 3;
  }
  auto scene = engine->CreateScene("underwater-test");
  scene->SetAmbientLight(.4, .4, .4);
  scene->SetBackgroundColor(.01, .03, .04);
  auto light = scene->CreateDirectionalLight();
  light->SetDirection(.3, .3, -1);
  light->SetDiffuseColor(1, 1, 1);
  scene->RootVisual()->AddChild(light);
  auto seabed = scene->CreateVisual("seabed");
  seabed->AddGeometry(scene->CreateBox());
  seabed->SetLocalScale(100, 100, .2);
  seabed->SetLocalPosition(0, 0, -8);
  auto sand = scene->CreateMaterial();
  sand->SetDiffuse(.6, .5, .3);
  sand->SetRoughness(.9);
  sand->SetTexture(std::string(argv[3]) + "/aerial_beach_01_diff_2k.jpg");
  sand->SetNormalMap(std::string(argv[3]) + "/aerial_beach_01_nor_gl_2k.jpg");
  seabed->SetMaterial(sand);
  scene->RootVisual()->AddChild(seabed);
  auto target = scene->CreateVisual();
  target->AddGeometry(scene->CreateBox());
  target->SetLocalPosition(5, 0, -6);
  auto red = scene->CreateMaterial();
  red->SetDiffuse(1, .05, .05);
  target->SetMaterial(red);
  scene->RootVisual()->AddChild(target);
  auto camera = scene->CreateCamera();
  camera->SetImageWidth(640);
  camera->SetImageHeight(360);
  camera->SetHFOV(1.05);
  camera->SetAspectRatio(640. / 360);
  camera->SetLocalPosition(-2, 0, -3);
  camera->SetLocalRotation(0, .3, 0);
  scene->RootVisual()->AddChild(camera);
  auto depth = scene->CreateDepthCamera();
  depth->SetImageWidth(640);
  depth->SetImageHeight(360);
  depth->SetHFOV(1.05);
  depth->SetAspectRatio(640. / 360);
  depth->SetNearClipPlane(.1);
  depth->SetFarClipPlane(100);
  depth->SetLocalPose(camera->LocalPose());
  scene->RootVisual()->AddChild(depth);
  std::vector<float> measured, baseline;
  std::vector<unsigned char> rgb(640 * 360 * 3);
  auto connection = depth->ConnectNewDepthFrame(
    [&](const float * data, unsigned w, unsigned h, unsigned, const std::string &)
    { measured.assign(data, data + w * h); });
  auto colors = depth->ConnectNewRgbPointCloud(
    [&](const float * data, unsigned w, unsigned h, unsigned, const std::string &)
    {
      for (unsigned i = 0; i < w * h; ++i)
      {
        uint32_t c;
        std::memcpy(&c, data + i * 4 + 3, 4);
        rgb[i * 3] = (c >> 24) & 255;
        rgb[i * 3 + 1] = (c >> 16) & 255;
        rgb[i * 3 + 2] = (c >> 8) & 255;
      }
    });
  camera->Update();
  depth->Update();
  if (measured.empty())
  {
    return 4;
  }
  baseline = measured;
  {
    posim::UnderwaterRenderer renderer(argv[1]);
    auto settings = posim::Defaults();
    std::vector<unsigned char> untreated, waterOnly;
    for (int mode = 0; mode < 5; ++mode)
    {
      settings[2] = mode == 0 ? 0 : 1;
      settings[4] = mode == 2 ? 1 : 0;
      settings[10] = mode == 3 ? 1 : 0;
      settings[3] = mode == 4 ? 0 : 1;
      settings[11] = 5;
      renderer.Update(scene, settings, 10.);
      auto image = camera->CreateImage();
      for (int i = 0; i < 3; ++i)
      {
        camera->Capture(image);
      }
      std::ofstream file(
        std::string(argv[2]) + "/native-" + std::to_string(mode) + ".ppm", std::ios::binary);
      file << "P6\n640 360\n255\n";
      file.write(reinterpret_cast<char *>(image.Data<unsigned char>()), 640 * 360 * 3);
      depth->Update();
      if (
        measured.size() != baseline.size() ||
        std::memcmp(measured.data(), baseline.data(), baseline.size() * sizeof(float)))
      {
        return 5;
      }
      auto pixels = image.Data<unsigned char>();
      unsigned long delta = 0, agreement = 0;
      for (size_t i = 0; i < rgb.size(); ++i)
      {
        agreement += std::abs(int(pixels[i]) - int(rgb[i]));
        if (mode > 0)
        {
          delta += std::abs(int(pixels[i]) - int(mode == 1 ? untreated[i] : waterOnly[i]));
        }
      }
      if (agreement > rgb.size() * 2)
      {
        return 6;
      }
      if (mode == 0)
      {
        untreated.assign(pixels, pixels + rgb.size());
      }
      if (mode == 1)
      {
        waterOnly.assign(pixels, pixels + rgb.size());
      }
      if (mode > 0 && delta < 100)
      {
        return 7;
      }
      std::ofstream rgbFile(
        std::string(argv[2]) + "/rgbd-" + std::to_string(mode) + ".ppm", std::ios::binary);
      rgbFile << "P6\n640 360\n255\n";
      rgbFile.write(reinterpret_cast<char *>(rgb.data()), rgb.size());
      auto start = std::chrono::steady_clock::now();
      for (int i = 0; i < 40; ++i)
      {
        camera->Capture(image);
        depth->Update();
      }
      std::cout << "mode " << mode << " RGB + RGBD ms "
                << std::chrono::duration<double, std::milli>(
                     std::chrono::steady_clock::now() - start)
                       .count() /
                     40
                << " depth unchanged" << std::endl;
    }
  }
  engine->DestroyScene(scene);
  return 0;
}
