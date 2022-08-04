#include <tuple>
#include "test_component_gen_decl.h"

template <typename out, typename... in>
void TestComponentGen<out, in...>::operator()(std::atomic_bool& sig){
    while(sig) {
        std::apply([&](auto&... queues) {
            TestComponentGen<out, in...>::Component::output.push(std::make_shared<int>(1));
        }, TestComponentGen<out, in...>::Component::inputs);
    }
}