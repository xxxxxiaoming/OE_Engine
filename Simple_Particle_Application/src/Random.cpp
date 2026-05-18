#include "Random.h"

namespace Engine
{
    std::mt19937 Random::randomEngine{ std::random_device{}() };
}