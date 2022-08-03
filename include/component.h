#include <tuple>
#include "component_decl.h"

template <typename out, typename... in>
Component<out, in...>::Component(function_type work_function) : work_function{std::move(work_function)} {};

template <typename out, typename... in>
void Component<out, in...>::operator()(std::atomic_bool& sig) {
    while(sig) {
        std::apply([&](auto&... queues) {
            output.push(work_function(queues.pop(sig)...));
        }, inputs);
    }
}

template <typename out, typename... in>
void Component<out, in...>::bindOutput(out& o) {
    output = o;
}

template <typename out, typename... in>
template <typename CompRef>
void Component<out, in...>::bindInput(
    std::tuple_element_t<CompRef::N, decltype(inputs)>& i) {
    std::get<CompRef::N>(inputs) = CompRef::comp_ref;
}