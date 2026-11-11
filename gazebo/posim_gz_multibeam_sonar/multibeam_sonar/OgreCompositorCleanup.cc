// SPDX-License-Identifier: Apache-2.0
#include "OgreCompositorCleanup.hh"
#include <Compositor/OgreCompositorManager2.h>
#include <OgreRectangle2D.h>
#include <OgreRoot.h>

namespace posim
{
void ClearOgreCompositorDatablocks()
{
  auto * compositor = Ogre::Root::getSingleton().getCompositorManager2();
  compositor->getSharedFullscreenQuad()->_setNullDatablock();
  compositor->getSharedFullscreenTriangle()->_setNullDatablock();
}
}  // namespace posim
