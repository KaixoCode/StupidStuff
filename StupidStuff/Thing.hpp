#pragma once
#include <iostream>

struct Thing
{
    static inline int refcount = 0;
    static inline int constrcount = 0;
    static inline int movecount = 0;
    static inline int copycount = 0;
    static inline int destrcount = 0;
    Thing() {};
    Thing(int v) : v(v) { constrcount++; refcount++; }
    Thing(Thing&& t) { v = t.v; movecount++; refcount++;  }
    Thing(const Thing& t) { v = t.v; copycount++; refcount++; }
    ~Thing() { destrcount++; refcount--; };
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