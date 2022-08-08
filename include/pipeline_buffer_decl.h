#pragma once
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

template <typename C>
struct component_output_ref {
    C& prod_ref;
};

template <typename T, typename in_p, typename ProdRef, typename... CompRefs>
class PipelineBuffer {
    private:
        std::shared_ptr<BlockingQueue<T>> queue;
        std::tuple<CompRefs...> refs;
        static constexpr std::size_t no_subscribers = sizeof... (CompRefs);
        std::atomic_int no_subscribers_finished = 0;
        std::atomic_int no_subscribers_ready = 0;
        std::mutex mut;
        std::condition_variable cond_var;

        
    public:
        // Need a way to make this constructor private,
        // But it must be called by std::make_shared
        PipelineBuffer(ProdRef producer,
                       CompRefs... consumers);
        static std::shared_ptr<PipelineBuffer> PipelineBuffer_factory(ProdRef producer,
                                                                      CompRefs... consumers);
        using value_type = T;

        void push(T value);
        std::optional<T> pop(std::atomic_bool& sig);
};