#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"
#include "component.h"

// A component_input_ref is just a struct
// holding a reference to a Component and
// a number indicating on which input to connect
// to on this Component
// typename C is the type of the Component
// We need the type of the Component as it holds information
// about the type of the work function inside the Component
// and also what type of out or in is used by the component
// e.g. BlockingQueue
template <std::size_t N, typename C>
struct component_input_ref {
    C& comp_ref;
    static constexpr std::size_t input_num = N;
};

// A factory method to create a component_input_ref
// N is a non-type template parameter indicating
// which input on the Component of type C to connect to
template <std::size_t N, typename C>
auto component_input_ref_factory(C& comp) {
    return component_input_ref<N, C>{comp};
}

// A component_output_ref holds a reference to the producer component
// and the type of the output values produced by the work function
// inside this producer Component
template <typename C>
struct component_output_ref {
    C& prod_ref;
    using value_type = typename C::output_value_type;
};

// A deduction guide for the constructor of component_output_ref
template <typename C>
component_output_ref(C& producer) -> component_output_ref<C>;

template <typename T>
class PipelineBuffer {
    private:
        std::shared_ptr<BlockingQueue<T>> queue;
        // How to make no_subscribers const?
        std::size_t no_subscribers;
        std::size_t cur_count;
        std::size_t generation;
        std::mutex mut;
        std::condition_variable cond_var;
        // This data copy is due to the implementation of pop
        std::optional<T> data_copy;
        // A PipelineBuffer is created by giving it ProdRefs and CompRefs
        // It uses this to build-up some metadata such as the number of
        // subscribing Components, no_subscribers
        // This constructor is private, since we want to return a std::shared_ptr to this PipelineBuffer
        // This std::shared_ptr is then used in the binding of data inputs and outputs to each Component
        // in the pipeline_orchestrator
        template <typename ProdRef, typename... CompRefs>
        PipelineBuffer(ProdRef producer,
                       CompRefs... consumers);
        
    public:
        // We use this factor method trick to create a PipelineBuffer, using a private constructor
        // and return a std::shared_ptr to it
        // Additionally, it is bound as the output buffer for ProdRef's producer Component and plugged into the
        // correct input slot for each Component requiring it as input by using the data in the CompRefs
        template <typename ProdRef, typename... CompRefs>
        static std::shared_ptr<PipelineBuffer> PipelineBuffer_factory(ProdRef producer,
                                                                      CompRefs... consumers);
        
        using value_type = T;

        void push(T value);
        std::optional<T> pop(std::atomic_bool& sig);
};