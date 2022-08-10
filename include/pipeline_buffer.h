#pragma once
#include "blocking_queue_decl.h"
#include "pipeline_buffer_decl.h"
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>

template <typename T>
template <typename ProdRef, typename... CompRef>
PipelineBuffer<T>::
    PipelineBuffer(ProdRef producer,
                   CompRef... consumers) :
                   queue{std::make_shared<BlockingQueue<T>>()},
                   no_subscribers{sizeof... (CompRef)},
                   cur_count(sizeof... (CompRef)),
                   generation{0},
                   mut{},
                   cond_var{} {}

template <typename T>
void PipelineBuffer<T>::
    push(T value) {
        queue->push(value);
    }

template <typename T>
std::optional<T> PipelineBuffer<T>::
    pop(std::atomic_bool& sig) {
        std::unique_lock<std::mutex> lock{mut};
        std::size_t _generation = generation;
        if (!--cur_count) {
            // cur_count is 0 meaning all threads are ready to fetch
            // this means we can fetch the item as a copy
            // and pop from the queue
            // all threads read this copy
            ++generation;
            cur_count = no_subscribers;
            data_copy = queue->pop(sig);
            cond_var.notify_all();
        } else {
            cond_var.wait(lock, [this, _generation, &sig] { 
                // We should stop waiting either when sig is unset,
                // or everyone is ready for the next generation
                if (!sig) return true;
                return _generation != generation; 
                });
        }
        if (!sig) return {};
        // Threads access data_copy
        return data_copy;

}

template <typename T>
template <typename ProdRef, typename... CompRef>
std::shared_ptr<PipelineBuffer<T>> PipelineBuffer<T>::
    PipelineBuffer_factory(ProdRef producer,
                            CompRef... consumers) {
                            std::shared_ptr<PipelineBuffer<T>> new_pipeline_buffer = std::make_shared<PipelineBuffer<T>>(producer, consumers...);
                            producer.prod_ref.bindOutput(new_pipeline_buffer);
                            ((consumers.comp_ref.template bindInput<CompRef>(new_pipeline_buffer)), ...);
                            return new_pipeline_buffer;
                            }
    