#pragma once

#include <string>

struct Restaurant {
    int id;
    std::string name;
    double x;
    double y;

    Restaurant() : id(-1), name(""), x(0.0), y(0.0) {}
    Restaurant(int id_, std::string name_, double x_, double y_)
        : id(id_), name(std::move(name_)), x(x_), y(y_) {}
};
