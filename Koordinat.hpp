#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <optional>
#include <string>
#include <cmath>
#include <vector>
using std::cout;
using std::endl;

class Koordinat {
    public:
    float x;
    float y;
    public:
    Koordinat(float x = 0.0f, float y=0.0f) : x(x), y(y) {}
};

