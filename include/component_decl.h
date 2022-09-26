// Copyright 2022 Cantab Research Ltd.
// Licensed under the MIT license. See LICENSE.txt in the project root for details.
#pragma once
#include <memory>
#include <tuple>
#include <functional>
#include <atomic>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"

// A Component is a fundamental unit of work
// The out and in parameters are the types of the data containers passed
// into the Component e.g. BlockingQueue
// The type held by these containers can be extracted via value_type e.g.
// out::value_type - this means that value_type must be exposed in
// whatever is passed in as the out and in parameters
// Additionally, all ins must have the pop() method defined, which returns
// a value of the type given by that input's value_type
// and the out must have the push() method defined, taking a value of the
// type given by out's value_type
// The work function must be invocable, taking in the (in::value_type)s as arguments (in that specific order)
// and produces a value of type out::value_type
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
        // Expose the out::value_type
        using output_value_type = typename out::value_type;

        Component(function_type work_function);

        // This makes the component invocable
        // You can simply pass a component to std::thread and
        // it will start running in an infinite loop until sig is set to false
        // In the loop, it repeatedly takes data from the inputs (blocking if it is not available)
        // applies the work function and puts data into the output
        void operator()(std::atomic_bool& sig);

        // Instantiates the output member shared_ptr
        void bindOutput(std::shared_ptr<out> o);

        // Instantiates the correct member shared_ptr in the member inputs tuple
        // The position in the tuple is determined by CompRef::input_num
        template <typename CompRef>
        void bindInput(std::tuple_element_t<CompRef::input_num, decltype(inputs)> i);
};