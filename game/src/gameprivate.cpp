#include "gameprivate.h"

namespace trash
{
namespace game
{

const int GamePrivate::numPersons;
const std::array<std::string, GamePrivate::numPersons> GamePrivate::personsNames {
    "malcolm.mdl",
    "shae.mdl",
    "liam.mdl",
    "stefani.mdl",
    "regina.mdl"
};

GamePrivate::GamePrivate()
{
}

} // namespace
} // namespace
