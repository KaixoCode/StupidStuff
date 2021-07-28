#pragma once
#include <iostream>

struct Thing
{
    Thing(int v) : v(v) { std::cout << "Construct" << std::endl; }
    Thing(Thing&& t) { v = t.v; std::cout << "Move" << std::endl; }
    Thing(const Thing& t) { v = t.v; std::cout << "Copy" << std::endl; }
    ~Thing() { std::cout << "Destroy" << std::endl; };
    int v = 0;

    bool operator==(const Thing& other) const
    {
        return v == other.v;
    }

    bool operator==(const int& other) const
    {
        return v == other;
    }

    Thing operator+(const Thing& other) const
    {
        return { v + other.v };
    }
};