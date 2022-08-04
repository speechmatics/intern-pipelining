#pragma once
#include <memory>
#include <vector>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"
#include "component.h"

template <std::size_t N, typename C>
struct component_input_ref {
    C& comp_ref;
    static constexpr std::size_t input_num = N;
};

template <typename T, typename in_p, typename... CompRefs>
class PipelineBuffer {
    private:
        std::shared_ptr<BlockingQueue<T>> queue;
        std::tuple<CompRefs...> refs;
    public:
        using value_type = T;
        PipelineBuffer(Component<T, in_p>& producer,
                       CompRefs... consumers);
};