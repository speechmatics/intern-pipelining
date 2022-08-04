#pragma once
#include <tuple>
#include <cstdio>
#include "component_decl.h"

template <typename out, typename... in>
Component<out, in...>::Component(function_type work_function) : work_function{std::move(work_function)} {};

template <typename out, typename... in>
void Component<out, in...>::operator()(std::atomic_bool& sig) {
    while(sig) {
        std::apply([&](auto&... queues) {
            auto args = std::make_tuple(queues->pop(sig)...);
            bool all_have_values = std::apply([&](auto&... data_optionals) {
                return (data_optionals.has_value() && ...);
            }, args);
            if (!all_have_values) {
                return;
            }
            std::apply([&](auto&... data) {
                output->push(work_function(*data...));
            }, args);
        }, inputs);
    }
}

template <typename out, typename... in>
void Component<out, in...>::bindOutput(std::shared_ptr<out> o) {
    output = std::move(o);
}

template <typename out, typename... in>
template <typename CompRef>
void Component<out, in...>::bindInput(
    std::tuple_element_t<CompRef::input_num, decltype(inputs)> i) {
    std::get<CompRef::input_num>(inputs) = std::move(i);
}