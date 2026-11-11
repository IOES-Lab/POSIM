// SPDX-License-Identifier: Apache-2.0
#pragma once

namespace posim
{
// Call on the render thread before releasing depth-camera materials.
void ClearOgreCompositorDatablocks();
}  // namespace posim
