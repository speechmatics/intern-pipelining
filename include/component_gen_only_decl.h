#pragma once
#include <memory>
#include <tuple>
#include <functional>
#include <atomic>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"

template <typename out>
class ComponentGenOnly {
    private:
        using function_type = std::function<
            typename out::value_type()
        >;

        function_type work_function;

        std::shared_ptr<out> output;

    public:
        ComponentGenOnly(function_type work_function);

        void operator()(std::atomic_bool& sig);

        void bindOutput(std::shared_ptr<out> o);
};