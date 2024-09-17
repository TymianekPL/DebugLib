//
// Created by Tymianek on 17.09.2024.
//

#ifndef DEBUGLIB_H
#define DEBUGLIB_H
#include <concepts>
#include <source_location>

#ifdef DEBUG
#ifndef DEBUG_PRINT
#include <print>

template<typename T, typename = void>
struct auto_const_reference
{
     using type = T;
};

template<typename T>
struct auto_const_reference <T, std::enable_if_t <(sizeof(T) > 16 || !std::is_trivially_copyable_v <T>)> >
{
     using type = std::add_lvalue_reference_t <std::add_const_t <T> >;
};

template<typename TCurrent, typename... TRest>
void DEBUG_PRINT_impl(const TCurrent& current, const TRest&... rest)
{
     std::print("{} ", current);

     if constexpr (sizeof...(rest) > 0) DEBUG_PRINT_impl(rest...);
}
template<typename TCurrent, typename... TRest>
void DEBUG_PRINT_impl(TCurrent&& current, const TRest&... rest)
{
     std::print("{} ", current);

     if constexpr (sizeof...(rest) > 0) DEBUG_PRINT_impl(rest...);
}

template<typename TLast>
void DEBUG_PRINT_impl(const TLast& last)
{
     std::print("{}", last);
}
template<typename TLast>
void DEBUG_PRINT_impl(TLast&& last)
{
     std::print("{}", last);
}

#define DEBUG_PRINT DEBUG_PRINT_impl
#endif

template<typename T>
concept boolean_testable = std::convertible_to <T, bool> and not std::is_class_v <std::remove_cvref_t <T> >;

template<typename T, boolean_testable TBoolTestable = bool>
void runtime_assert_impl(const TBoolTestable condition, auto_const_reference <T> message,
                         const std::source_location location = std::source_location::current())
{
     if (condition) return;
     DEBUG_PRINT("Runtime assertion failed. Reason: ", message, ".\nSource: ", location.file_name(), ":", location.line(), "@", location.function_name());
#ifdef __MSVCRT__
     __debugbreak();
#elif defined(__GNUC__) or defined(__clang__) or defined(__INTEL_COMPILER)
     __builtin_trap();
#endif
     std::abort();
}

template<typename T, boolean_testable TBoolTestable = bool>
constexpr void runtime_assert_impl(const TBoolTestable condition, T&& message,
                                   const std::source_location location = std::source_location::current())
{
     if (condition) return;
     DEBUG_PRINT("Runtime assertion failed. Reason: ", message, ".\nSource: ", location.file_name(), ":", location.line(), "@", location.function_name());

#ifdef __MSVCRT__
     __debugbreak();
#elif defined(__GNUC__) or defined(__clang__) or defined(__INTEL_COMPILER)
     __builtin_trap();
#endif
     std::abort();
}
template<boolean_testable TBoolTestable = bool>
constexpr void runtime_assert_impl(const TBoolTestable condition,
                                   const std::source_location location = std::source_location::current())
{
     if (condition) return;
     DEBUG_PRINT("Runtime assertion failed.\nSource: ", location.file_name(), ":", location.line(), "@", location.function_name());

#ifdef __MSVCRT__
     __debugbreak();
#elif defined(__GNUC__) or defined(__clang__) or defined(__INTEL_COMPILER)
     __builtin_trap();
#endif
     std::abort();
}

#ifdef __GNUC__
#define runtime_assert(condition, ...) do { static_assert(not __builtin_constant_p(condition), "Do not use this function at compile time."); runtime_assert_impl(condition, __VA_ARGS__); } while (0)
#else
#define runtime_assert runtime_assert_impl
#endif
#elif defined(NDEBUG)
#define runtime_assert(_condition, _message) // discard
#define runtime_assert(_condition) // discard
#elif defined(REMOVE_MY_DEBUG)
// undefined. Should be uncompilable by definition.
#else
#error "Could not decide whether it is a debug build or not. Consider using `DEBUG` or `NDEBUG` macros, or define `REMOVE_MY_DEBUG` to surpress this error."
#endif

#endif //DEBUGLIB_H
