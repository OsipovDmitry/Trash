#ifndef SCENEROOTNODEPRIVATE_H
#define SCENEROOTNODEPRIVATE_H

#include "nodeprivate.h"

namespace trash
{
namespace core
{

class Scene;

class SceneRootNodePrivate : public NodePrivate
{
public:
    SceneRootNodePrivate(Node&, Scene*);

    void dirtyLocalBoundingBox();

    const utils::BoundingBox& getLocalBoundingBox() override;
    void doAfterChangingTransformation() override;

    Scene *scene;
    utils::BoundingBox localBoundingBox;
    bool isLocalBoundingBoxDirty;

};

#endif // SCENEROOTNODEPRIVATE_H

} // namespace
} // namespace
