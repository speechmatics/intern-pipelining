#pragma once
#include "blocking_queue_decl.h"
#include "pipeline_buffer_decl.h"
#include <memory>
#include <mutex>

template <typename T, typename in_p, typename ProdRef, typename... CompRefs>
PipelineBuffer<T, in_p, ProdRef, CompRefs...>::
    PipelineBuffer(ProdRef producer,
                   CompRefs... consumers) :
                   queue{std::make_shared<BlockingQueue<T>>()},
                   refs{consumers...} {
                    producer.prod_ref.bindOutput(queue);
                    ((consumers.comp_ref.template bindInput<consumers>(queue)), ...);
                   }

template <typename T, typename in_p, typename ProdRef, typename... CompRefs>
void PipelineBuffer<T, in_p, ProdRef, CompRefs...>::
    push(T value) {
        queue->push(value);
    }

template <typename T, typename in_p, typename ProdRef, typename... CompRefs>
std::optional<T> PipelineBuffer<T, in_p, ProdRef, CompRefs...>::
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
        T val = queue.peek(sig);
        if (++no_subscribers_ready == no_subscribers) {
            queue.pop(sig);
            no_subscribers_finished = 0;
            no_subscribers_ready = 0;
        }
}

template <typename T, typename in_p, typename ProdRef, typename... CompRefs>
std::shared_ptr<PipelineBuffer<T, in_p, ProdRef, CompRefs...>> PipelineBuffer<T, in_p, ProdRef, CompRefs...>::
    PipelineBuffer_factory(ProdRef producer,
                            CompRefs... consumers) {
                            return std::make_shared<PipelineBuffer<T, in_p, ProdRef, CompRefs...>>(producer, consumers...);
                            } 
    