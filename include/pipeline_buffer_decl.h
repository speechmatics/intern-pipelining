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

template <std::size_t N, typename C>
struct component_input_ref {
    C& comp_ref;
    static constexpr std::size_t input_num = N;
};

template <std::size_t N, typename C>
auto component_input_ref_factory(C& comp) {
    return component_input_ref<N, C>{comp};
}

template <typename C>
struct component_output_ref {
    C& prod_ref;
    using value_type = typename C::output_value_type;
};

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
        std::optional<T> data_copy;
        template <typename ProdRef, typename... CompRefs>
        PipelineBuffer(ProdRef producer,
                       CompRefs... consumers);
        
    public:
        template <typename ProdRef, typename... CompRefs>
        static std::shared_ptr<PipelineBuffer> PipelineBuffer_factory(ProdRef producer,
                                                                      CompRefs... consumers);
        
        using value_type = T;

        void push(T value);
        std::optional<T> pop(std::atomic_bool& sig);
};