#include <tuple>
#include "test_component_stdout_decl.h"

template <typename out, typename... in>
void TestComponentStdout<out, in...>::operator()(std::atomic_bool& sig){
    while(sig) {
        std::apply([&](auto&... queues) {
            work_function(queues.pop(sig)...);
        }, TestComponentStdout<out, in...>::Component::inputs);
    }
}