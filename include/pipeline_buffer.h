#pragma once
#include "blocking_queue_decl.h"
#include "pipeline_buffer_decl.h"
#include <memory>
#include <mutex>

template <typename T>
template <typename ProdRef, typename... CompRefs>
PipelineBuffer<T>::
    PipelineBuffer(ProdRef producer,
                   CompRefs... consumers) :
                   queue{std::make_shared<BlockingQueue<T>>()},
                   no_subscribers{sizeof... (CompRefs)} {}

template <typename T>
void PipelineBuffer<T>::
    push(T value) {
        queue->push(value);
    }

template <typename T>
std::optional<T> PipelineBuffer<T>::
    pop(std::atomic_bool& sig) {
        no_subscribers_finished++;
        cond_var.notify_all();
        std::unique_lock<std::mutex> lock{mut};
        while (no_subscribers_finished < no_subscribers) {
            if (!sig) {
                return {};
            }
            cond_var.wait(lock);
        }
        std::optional<T> val = queue->peek(sig);
        if (!val.has_value()) return val;
        if (++no_subscribers_ready == no_subscribers) {
            queue->pop(sig);
            no_subscribers_finished = 0;
            no_subscribers_ready = 0;
        }
        return val;
}

template <typename T>
template <typename ProdRef, typename... CompRefs>
std::shared_ptr<PipelineBuffer<T>> PipelineBuffer<T>::
    PipelineBuffer_factory(ProdRef producer,
                            CompRefs... consumers) {
                            std::shared_ptr<PipelineBuffer<T>> new_pipeline_buffer = std::make_shared<PipelineBuffer<T>>(producer, consumers...);
                            producer.prod_ref.bindOutput(new_pipeline_buffer);
                            ((consumers.comp_ref.template bindInput<consumers>(new_pipeline_buffer)), ...);
                            return new_pipeline_buffer;
                            }
    