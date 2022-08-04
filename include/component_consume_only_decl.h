#pragma once
#include <memory>
#include <tuple>
#include <functional>
#include <atomic>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"

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