#pragma once
#include <iostream>

struct Thing
{
    static inline int refcount = 0;
    Thing(int v) : v(v) { std::cout << "Construct" << std::endl; refcount++; }
    Thing(Thing&& t) { v = t.v; std::cout << "Move" << std::endl; refcount++;  }
    Thing(const Thing& t) { v = t.v; std::cout << "Copy" << std::endl; refcount++; }
    ~Thing() { std::cout << "Destroy" << std::endl; refcount--; };
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