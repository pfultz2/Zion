/*=============================================================================
    Copyright (c) 2012 Paul Fultz II
    test.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef ZION_GUARD_ZION_TEST_H
#define ZION_GUARD_ZION_TEST_H

#include <functional>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <zion/returns.h>

#define ZION_TEST_CASE(name) \
struct name \
{ void operator()(zion::test::failure_callback fail) const; }; \
static zion::test::auto_register name ## _register = zion::test::auto_register(#name, name()); \
void name::operator()(zion::test::failure_callback zion_test_fail) const

#define ZION_TEST_CHECK(...) [&]\
{ \
    try \
    { \
        auto result = zion::test::detail::capture() ->* __VA_ARGS__; \
        if (not result.value()) zion_test_fail(#__VA_ARGS__ + result.as_string(), __FILE__, __LINE__); \
    } \
    catch(const std::exception& zion_ex) { zion_test_fail(#__VA_ARGS__ + std::string("\nException thrown: ") + zion_ex.what(), __FILE__, __LINE__); throw; } \
    catch(...) { zion_test_fail(#__VA_ARGS__ + std::string("\nAn unknown exception has occured"), __FILE__, __LINE__); throw; } \
}()

#define ZION_TEST_FOREACH_OPERATOR(m) \
m(==, equal) \
m(!=, not_equal) \
m(<=, less_than_equal) \
m(>=, greater_than_equal) \
m(<, less_than) \
m(>, greater_than)

namespace zion { namespace test {

namespace detail {

#define ZION_TEST_EACH_OPERATOR_OBJECT(op, name) \
constexpr struct name ## _f \
{ \
    static std::string as_string() { return #op; } \
    template<class T, class U> \
    static auto call(T&& x, U&& y) ZION_RETURNS(x op y); \
} name {};

ZION_TEST_FOREACH_OPERATOR(ZION_TEST_EACH_OPERATOR_OBJECT)

template<class T, class U, class Operator>
struct expression
{
    T lhs;
    U rhs;

    expression(T lhs, U rhs) : lhs(lhs), rhs(rhs)
    {}

    std::string as_string() const
    {
        std::stringstream ss;
        ss << " [ " << lhs << " " << Operator::as_string() << " " << rhs << " ]";
        return ss.str();
    }

    T value() const
    {
        return Operator::call(lhs, rhs);
    }
};

template<class T, class U, class Operator>
expression<T, U, Operator> make_expression(const T& rhs, const U& lhs, Operator)
{
    return expression<T, U, Operator>(rhs, lhs);
}

template<class T>
struct lhs_expression;

template<class T>
lhs_expression<T> make_lhs_expression(const T& lhs)
{
    return lhs_expression<T>(lhs);
}

template<class T>
struct lhs_expression
{
    T lhs;
    lhs_expression(T e) : lhs(e)
    {}

    std::string as_string() const
    {
        std::stringstream ss;
        ss << lhs;
        return ss.str();
    }

    T value() const
    {
        return lhs;
    }

#define ZION_TEST_LHS_OPERATOR(op, name) \
    template<class U> \
    auto operator op(const U& rhs) const ZION_RETURNS(make_expression(lhs, rhs, name)); 

ZION_TEST_FOREACH_OPERATOR(ZION_TEST_LHS_OPERATOR)

#define ZION_TEST_LHS_REOPERATOR(op) \
    template<class U> auto operator op(const U& rhs) const ZION_RETURNS(make_lhs_expression(lhs op rhs));
ZION_TEST_LHS_REOPERATOR(+)
ZION_TEST_LHS_REOPERATOR(-)
ZION_TEST_LHS_REOPERATOR(*)
ZION_TEST_LHS_REOPERATOR(/)
ZION_TEST_LHS_REOPERATOR(%)
ZION_TEST_LHS_REOPERATOR(&)
ZION_TEST_LHS_REOPERATOR(|)
ZION_TEST_LHS_REOPERATOR(&&)
ZION_TEST_LHS_REOPERATOR(||)

};

struct capture 
{
    template<typename T>
    auto operator->* (const T& x) ZION_RETURNS(make_lhs_expression(x));
};

}

typedef std::function<void(std::string message, std::string file, long line)> failure_callback;
typedef std::function<void(failure_callback fail)> test_case;
static std::vector<std::pair<std::string, test_case> > test_cases;

struct auto_register
{
    auto_register(std::string name, test_case tc)
    {
        test_cases.push_back(std::make_pair(name, tc));
    }
};

void run()
{
    bool failed = false;
    for(const auto& tc: test_cases)
    {
        tc.second([&](std::string message, std::string file, long line)
        {
            std::cout << "*****FAILED: " << tc.first << " at: " << std::endl << file << ":" << line << std::endl << message << std::endl;
            failed = true;
        });
    }
    if (not failed) std::cout << "All " << test_cases.size() << " test cases passed." << std::endl;
}

}}

#endif
