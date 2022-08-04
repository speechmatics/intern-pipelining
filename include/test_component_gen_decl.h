#include "component.h"

template <typename out, typename... in>
class TestComponentGen : public Component<out, in...> {
    public:
        void operator()(std::atomic_bool& sig) override;
};