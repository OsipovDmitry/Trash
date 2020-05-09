#ifndef GAMEPRIVATE_H
#define GAMEPRIVATE_H

#include <array>
#include <memory>
#include <vector>

namespace trash
{
namespace game
{

class Scene;
class Person;
class Floor;
class WayPoint;

class GamePrivate
{
public:
    GamePrivate();

    std::shared_ptr<Scene> scene;
    std::vector<std::shared_ptr<WayPoint>> waypoints;

    static const int numPersons = 5;
    static const std::array<std::string, numPersons> personsNames;
    std::array<std::shared_ptr<Person>, numPersons> persons;
    std::weak_ptr<Person> acivePerson;

    std::shared_ptr<Floor> floor;
};

} // namespace
} // namespace

#endif // GAMEPRIVATE_H
