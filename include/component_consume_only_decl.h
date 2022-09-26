// Copyright 2022 Cantab Research Ltd.
// Licensed under the MIT license. See LICENSE.txt in the project root for details.
#pragma once
#include <memory>
#include <tuple>
#include <functional>
#include <atomic>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"

// The design of the pipeline is such that the pipeline_buffers are between components
// This means that we need a component at either end (at least)
// The component(s) at the beginning of the pipeline take no inputs and produce a single output
// These beginning components can be regular components as the number of input arguments to a component
// can be 0, however since every component must produce an output, we have the fact that we cannot use
// a regular component at the end of the pipeline
// As a result, we have a special ComponentConsumeOnly which take 0 or more inputs, but produce no output
template <typename... in>
class ComponentConsumeOnly {
    private:
        using function_type = std::function<
            void(
                const typename in::value_type&...
            )
        >;

        function_type work_function;

        std::tuple<std::shared_ptr<in>...> inputs;

    public:
        ComponentConsumeOnly(function_type work_function);

        void operator()(std::atomic_bool& sig);

        template <typename CompRef>
        void bindInput(std::tuple_element_t<CompRef::input_num, decltype(inputs)> i);
};