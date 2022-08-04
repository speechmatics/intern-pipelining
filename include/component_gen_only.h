#pragma once
#include <tuple>
#include "component_gen_only_decl.h"

template <typename out>
ComponentGenOnly<out>::ComponentGenOnly(function_type work_function) : work_function{std::move(work_function)} {};

template <typename out>
void ComponentGenOnly<out>::operator()(std::atomic_bool& sig) {
    while(sig) {
        output->push(work_function());
    }
}

template <typename out>
void ComponentGenOnly<out>::bindOutput(std::shared_ptr<out> o) {
    output = std::move(o);
}