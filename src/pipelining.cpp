#include "blocking_queue.h"
#include "component_decl.h"
#include "pipeline_stages.h"
#include "component.h"
#include "component_gen_only.h"
#include "component_consume_only.h"
#include "pipeline_buffer_decl.h"
#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <atomic>
#include <functional>

#include <easy/profiler.h>

constexpr int num_loops = 1000000000;

int gen_1() { return 1; };

int work(int x) { return x + 1; };

void print_input(int x) { printf("%i\n", x); }

int main() {
  EASY_PROFILER_ENABLE;
  EASY_FUNCTION();
  printf("Hello World!\n");

  std::atomic_bool sig{true};

  std::function<int()> Gen_1 = gen_1;
  std::function<int(int)> Work = work;
  std::function<void(int)> Print_Input = print_input;

  ComponentGenOnly<BlockingQueue<int>> start{Gen_1};

  std::shared_ptr<BlockingQueue<int>> buffer_1 = std::make_shared<BlockingQueue<int>>();
  start.bindOutput(buffer_1);

  Component<BlockingQueue<int>, BlockingQueue<int>> work_component{Work};

  std::shared_ptr<BlockingQueue<int>> buffer_2 = std::make_shared<BlockingQueue<int>>();
  work_component.bindInput<component_input_ref<0, Component<BlockingQueue<int>, BlockingQueue<int>>>>(buffer_1);
  work_component.bindOutput(buffer_2);

  ComponentConsumeOnly<BlockingQueue<int>> end_component{Print_Input};

  component_input_ref<0L, ComponentConsumeOnly<BlockingQueue<int>>> end_ref{end_component};
  end_component.bindInput<component_input_ref<0, ComponentConsumeOnly<BlockingQueue<int>>>>(buffer_2);

  std::thread Start{start, std::ref(sig)};
  std::thread WorkThread{work_component, std::ref(sig)};
  std::thread End{end_component, std::ref(sig)};
  Start.join();

  // sig = false;
  // input.cv_notify_all();
  // out_1.cv_notify_all();
  // out_2.cv_notify_all();
  // stage_1.join();
  // stage_2.join();
  // stage_3.join();
  int no_blocks_written = profiler::dumpBlocksToFile("pipelining.prof");
  printf("Wrote %i easy_profiler blocks\n", no_blocks_written);
  return 0;
}