#pragma once

#include <vector>
#include <list>

template<template<class> class ArrayType>
class ManagerBase
{
private:
    ArrayType<int> contants;

public:
    auto begin()
    {
        return contants.begin();
    }

    auto end()
    {
        return contants.end();
    }
};

//inline void ManagerBaseTest()
//{
//    ManagerBase<std::list> list_manager;
//    ManagerBase<std::vector> vector_manager;
//
//    for (auto& aabb0 : list_manager) {
//        for (auto& aabb1 : vector_manager) {
//
//        }
//    }
//}
