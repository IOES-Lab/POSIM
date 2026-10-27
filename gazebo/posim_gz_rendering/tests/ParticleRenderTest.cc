// SPDX-License-Identifier: Apache-2.0
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <gz/math/Matrix3.hh>
#include <gz/rendering/Camera.hh>
#include <gz/rendering/DepthCamera.hh>
#include <gz/rendering/Geometry.hh>
#include <gz/rendering/Material.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Visual.hh>
#include <iostream>
#include <string>
#include <vector>
#include "UnderwaterRenderer.hh"

// Rows contain a name, time, camera translation and row-major camera rotation.
// Cameras look along their Gazebo +X axis. Images retain their native pixels.
int main(int argc, char ** argv)
{
  if (argc != 4 && argc != 5)
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
  auto scene = engine->CreateScene("posim-particle-attribution");
  scene->SetSkyEnabled(false);
  scene->SetAmbientLight(.4, .4, .4);
  scene->SetBackgroundColor(.01, .03, .04);
  auto backdrop = scene->CreateVisual("uniform-static-backdrop");
  backdrop->AddGeometry(scene->CreateBox());
  backdrop->SetLocalScale(100, 100, .1);
  backdrop->SetLocalPosition(0, 0, argc == 5 ? std::stod(argv[4]) : -3.7);
  auto material = scene->CreateMaterial();
  material->SetDiffuse(.3, .3, .3);
  material->SetRoughness(.9);
  backdrop->SetMaterial(material);
  scene->RootVisual()->AddChild(backdrop);

  constexpr unsigned width = 800, height = 600;
  const double hfov = 2 * std::atan(width / (2 * 474.8967170715332));
  auto camera = scene->CreateCamera();
  camera->SetImageWidth(width);
  camera->SetImageHeight(height);
  camera->SetAspectRatio(double(width) / height);
  camera->SetHFOV(hfov);
  camera->SetNearClipPlane(.05);
  camera->SetFarClipPlane(100);
  scene->RootVisual()->AddChild(camera);
  camera->Update();
  auto depth = scene->CreateDepthCamera();
  depth->SetImageWidth(width);
  depth->SetImageHeight(height);
  depth->SetAspectRatio(double(width) / height);
  depth->SetHFOV(hfov);
  depth->SetNearClipPlane(.05);
  depth->SetFarClipPlane(100);
  scene->RootVisual()->AddChild(depth);
  std::vector<float> measured, baseline;
  auto connection = depth->ConnectNewDepthFrame(
    [&](const float * data, unsigned w, unsigned h, unsigned, const std::string &)
    { measured.assign(data, data + w * h); });
  {
    posim::UnderwaterRenderer renderer(argv[1]);
    std::ifstream poses(argv[2]);
    std::string name;
    double time, x, y, z, r[9];
    unsigned rows = 0;
    while (poses >> name >> time >> x >> y >> z >> r[0] >> r[1] >> r[2] >> r[3] >> r[4] >> r[5] >>
           r[6] >> r[7] >> r[8])
    {
      if (++rows > 32)
      {
        return 4;
      }
      camera->SetLocalPosition(x, y, z);
      camera->SetLocalRotation(gz::math::Quaterniond(
        gz::math::Matrix3d(r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8])));
      depth->SetLocalPose(camera->LocalPose());
      auto settings = posim::Defaults();
      for (int mode = 0; mode < 2; ++mode)
      {
        settings[10] = mode;
        renderer.Update(scene, settings, time);
        auto image = camera->CreateImage();
        for (int warmup = 0; warmup < 3; ++warmup)
        {
          camera->Capture(image);
        }
        std::ofstream file(
          std::string(argv[3]) + "/" + name + "-" + std::to_string(mode) + ".ppm",
          std::ios::binary);
        file << "P6\n800 600\n255\n";
        file.write(reinterpret_cast<char *>(image.Data<unsigned char>()), width * height * 3);
        if (!file)
        {
          return 5;
        }
        depth->Update();
        if (measured.size() != width * height)
        {
          return 6;
        }
        if (mode == 0)
        {
          baseline = measured;
        }
        else if (std::memcmp(baseline.data(), measured.data(), measured.size() * sizeof(float)))
        {
          return 7;
        }
        auto start = std::chrono::steady_clock::now();
        for (int repeat = 0; repeat < 30; ++repeat)
        {
          camera->Capture(image);
        }
        std::cout << name << "," << mode << ","
                  << std::chrono::duration<double, std::milli>(
                       std::chrono::steady_clock::now() - start)
                         .count() /
                       30
                  << ",depth_unchanged" << std::endl;
      }
    }
    if (!poses.eof() || !rows)
    {
      return 4;
    }
  }
  engine->DestroyScene(scene);
  return 0;
}
