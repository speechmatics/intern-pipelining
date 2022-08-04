#pragma once
#include "pipeline_buffer_decl.h"
#include <memory>

template <typename T, typename in_p, typename... CompRefs>
PipelineBuffer<T, in_p, CompRefs...>::
    PipelineBuffer(Component<T, in_p>& producer,
                   CompRefs... consumers) :
                   queue{std::make_shared<BlockingQueue<T>>()},
                   refs{consumers...} {
                    producer.bindOutput(queue);
                    ((consumers.comp_ref.bindInput<consumers>(queue)), ...);
                   }