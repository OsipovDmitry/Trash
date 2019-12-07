#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <unordered_set>

#include <utils/noncopyble.h>

class Node;
class Object;

class Scene
{
    NONCOPYBLE(Scene)

public:
    Scene();
    ~Scene();

    void detachObject(std::shared_ptr<Object>);
    void attachObject(std::shared_ptr<Object>, std::shared_ptr<Node> = nullptr);

    void update(uint64_t, uint64_t);

    std::shared_ptr<Object> findObject(Node*) const;

protected:
    std::unordered_set<std::shared_ptr<Object>> m_objects;
    std::shared_ptr<Node> m_groupNode;

};

#endif // SCENE_H
