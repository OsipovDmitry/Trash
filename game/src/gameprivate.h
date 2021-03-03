#ifndef GAMEPRIVATE_H
#define GAMEPRIVATE_H

#include <array>
#include <memory>
#include <vector>

namespace trash
{
namespace game
{

class Person;
class Level;
class Floor;

class GamePrivate
{
public:
    GamePrivate();

    std::shared_ptr<Level> scene;

    static const int numPersons = 1;
    static const std::array<std::string, numPersons> personsNames;
    std::array<std::shared_ptr<Person>, numPersons> persons;
    std::weak_ptr<Person> acivePerson;

    std::shared_ptr<Floor> floor;
};

} // namespace
} // namespace

#endif // GAMEPRIVATE_H
