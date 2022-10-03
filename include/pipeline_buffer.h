// Copyright 2022 Cantab Research Ltd.
// Licensed under the MIT license. See LICENSE.txt in the project root for details.
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

// Pushing data is simple
// It simply calls the push method of the member queue
template <typename T>
void PipelineBuffer<T>::
    push(T value) {
        queue->push(value);
    }

// Popping data is much more complex
// This is because we are essentially implementing a barrier
// The idea is that all threads must have popped the top element
// in the buffer, before any thread can move onto the next element
// This is to keep the various branches of the pipeline synchronised
// So, for example, if two component rely on this PipelineBuffer and
// Component A is slightly faster at execution than Component B,
// A must wait for B to pop, before being able to access new data
// This keeps A and B synchronised
template <typename T>
std::optional<T> PipelineBuffer<T>::
    pop(std::atomic_bool& sig) {
        std::unique_lock<std::mutex> lock{mut};
        std::size_t _generation = generation;
        // We first wait for all threads to be ready to pop
        // Those threads which arrive first will wait on the condition variable
        // The last thread to arrive will move the data from the queue into a temporary data_copy
        // and notify all the other waiting threads
        // Then all threads, including the last thread, which move the data into the data_copy
        // will read and return data_copy
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
                            std::shared_ptr<PipelineBuffer<T>> new_pipeline_buffer{new PipelineBuffer<T>(producer, consumers...)};
                            producer.prod_ref.bindOutput(new_pipeline_buffer);
                            ((consumers.comp_ref.template bindInput<CompRef>(new_pipeline_buffer)), ...);
                            return new_pipeline_buffer;
                            }
    