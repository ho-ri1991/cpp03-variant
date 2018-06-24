#ifndef STATIC_ASSERT_HPP
#define STATIC_ASSERT_HPP

#define STATIC_ASSERT(expr, msg) \
    typedef char STATIC_ASSERT_##msg[(expr) ? 1 : -1]

#endif

