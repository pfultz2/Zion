/*=============================================================================
    Copyright (c) 2012 Paul Fultz II
    returns.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/


#ifndef ZION_GUARD_RETURNS_H
#define ZION_GUARD_RETURNS_H

#include <utility>

// Returns from Dave Abrahams
#define ZION_RETURNS(...) \
       noexcept(noexcept(decltype(__VA_ARGS__)(std::move(__VA_ARGS__)))) \
       -> decltype(__VA_ARGS__) \
       { return (__VA_ARGS__); } static_assert(true, "")


#endif