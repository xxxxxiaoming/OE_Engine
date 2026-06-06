#pragma once
#include <random>

namespace Engine
{
    class Random
    {
    private:
        static std::mt19937 randomEngine;
    public:
        static void Init() { randomEngine.seed(std::random_device()()); }
        static int Int(int min, int max) { return std::uniform_int_distribution<int>(min, max)(randomEngine); }
        static float Float(float min, float max) { return std::uniform_real_distribution<float>(min, max)(randomEngine); }
    };
}