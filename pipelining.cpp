#include "blocking_queue.h"
#include "pipeline_stages.h"
#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include <easy/profiler.h>

constexpr int num_loops = 1000000000;

int main() {
  EASY_PROFILER_ENABLE;
  EASY_FUNCTION();
  printf("Hello World!\n");
  BlockingQueue<int> input{};
  std::shared_ptr<int> one = std::make_shared<int>(1);
  std::shared_ptr<int> two = std::make_shared<int>(2);
  std::shared_ptr<int> three = std::make_shared<int>(3);
  input.push(one);
  input.push(two);
  input.push(three);
  BlockingQueue<int> out_1{};
  BlockingQueue<int> out_2{};

  std::atomic_bool sig{true};

  std::thread stage_1{stage_one, std::ref(input), std::ref(out_1),
                      std::ref(sig)};
  std::thread stage_2{stage_two, std::ref(out_1), std::ref(out_2),
                      std::ref(sig)};
  std::thread stage_3{stage_three, std::ref(out_2), std::ref(sig)};

  for (int i = 0; i < num_loops; ++i) {
    input.push(std::make_shared<int>(i));
  }

  sig = false;
  input.cv_notify_all();
  out_1.cv_notify_all();
  out_2.cv_notify_all();
  stage_1.join();
  stage_2.join();
  stage_3.join();
  int no_blocks_written = profiler::dumpBlocksToFile("pipelining.prof");
  printf("Wrote %i easy_profiler blocks\n", no_blocks_written);
  return 0;
}