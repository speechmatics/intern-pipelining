#include "pipeline_stages.h"
#include <memory>
#include <iostream>
#include <ostream>

#include <easy/profiler.h>

void stage_one(BlockingQueue<int>& input, BlockingQueue<int>& output, std::atomic_bool& sig) {
    EASY_FUNCTION();
    // Adds one
    while(sig) {
        EASY_BLOCK("stage_one_while_loop");
        std::shared_ptr<int> i = input.pop(sig);
        if (i == nullptr) return;
        *i = (*i)+1;
        output.push(i);
    }
}

void stage_two(BlockingQueue<int>& input, BlockingQueue<int>& output, std::atomic_bool& sig) {
    EASY_FUNCTION();
    // Multiply by 2
    while(sig) {
        std::shared_ptr<int> i = input.pop(sig);
        if (i == nullptr) return;
        *i = (*i)*2;
        output.push(i);
    }
}

void stage_three(BlockingQueue<int>& input, std::atomic_bool& sig) {
    EASY_FUNCTION();
    while (sig) {
        std::shared_ptr<int> data_ptr = input.pop(sig);
        if (data_ptr == nullptr) return;
        std::cout << *data_ptr << std::endl;
    }
}