#include <tuple>
#include <functional>
#include <atomic>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"

template <typename out, typename... in>
class Component {
    private:
        using function_type = std::function<
            typename out::value_type(
                const typename in::value_type&...
            )
        >;

        function_type work_function;

        out& output;
        std::tuple<in&...> inputs;

    public:
        Component(function_type work_function);

        void operator()(std::atomic_bool& sig);

        void bindOutput(out& o);

        template <typename CompRef>
        void bindInput(std::tuple_element_t<CompRef::N, decltype(inputs)>& i);
};