#pragma once
#include <memory>
#include <tuple>
#include <functional>
#include <atomic>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"

template <typename out, typename... in>
class Component {
    private:
        using function_type = std::function<
            typename out::value_type(
                const typename in::value_type&...
            )
        >;

        function_type work_function;

        std::shared_ptr<out> output;
        std::tuple<std::shared_ptr<in>...> inputs;

    public:
        using output_value_type = typename out::value_type;

        Component(function_type work_function);

        void operator()(std::atomic_bool& sig);

        void bindOutput(std::shared_ptr<out> o);

        template <typename CompRef>
        void bindInput(std::tuple_element_t<CompRef::input_num, decltype(inputs)> i);
};