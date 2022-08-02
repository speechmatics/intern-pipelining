#include <tuple>
#include "component_decl.h"

template <typename out, typename... in>
Component<out, in...>::Component(BlockingQueue<out>& output, std::function<out(const in&...)> work_function, BlockingQueue<in>&... inputs) : output{output}, work_function{std::move(work_function)}, inputs{inputs...} {};

template <typename out, typename... in>
void Component<out, in...>::operator()(std::atomic_bool &sig) {
    while(sig) {
        std::apply([&](auto&... queues) {
            output.push(work_function(queues.pop(sig)...));
        }, inputs);
    }
}