#pragma once
#include <tuple>
#include "component_consume_only_decl.h"

template <typename... in>
ComponentConsumeOnly<in...>::ComponentConsumeOnly(function_type work_function) : work_function{std::move(work_function)} {};

template <typename... in>
void ComponentConsumeOnly<in...>::operator()(std::atomic_bool& sig) {
    while(sig) {
        std::apply([&](auto&... queues) {
            work_function(queues->pop(sig)...);
        }, inputs);
    }
}

template <typename... in>
template <typename CompRef>
void ComponentConsumeOnly<in...>::bindInput(
    std::tuple_element_t<CompRef::input_num, decltype(inputs)> i) {
    std::get<CompRef::input_num>(inputs) = std::move(i);
}