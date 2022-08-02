#include <tuple>
#include <functional>
#include <atomic>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"

template<typename out, typename... in>
class Component {
    private:
        BlockingQueue<out>& output;
        std::function<out(const in&...)> work_function;
        std::tuple<BlockingQueue<in>&...> inputs;

    public:
        Component(BlockingQueue<out>& output, std::function<out(const in&...)> work_function, BlockingQueue<in>&... inputs);

        void operator()(std::atomic_bool &sig);
};