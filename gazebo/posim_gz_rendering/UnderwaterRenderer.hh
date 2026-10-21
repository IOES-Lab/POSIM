// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <memory>
#include <string>
#include <gz/rendering/Scene.hh>
#include "Appearance.hh"
namespace posim {
class UnderwaterRenderer {
 struct Impl;
 std::unique_ptr<Impl> impl;
 public:
 explicit UnderwaterRenderer(const std::string &pieces);
 ~UnderwaterRenderer();
 void Update(const gz::rendering::ScenePtr &, const Appearance &, double simulationTime);
 unsigned Cameras() const;
};
}
