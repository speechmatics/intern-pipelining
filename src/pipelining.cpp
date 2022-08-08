#include "blocking_queue.h"
#include "component_decl.h"
#include "pipeline_stages.h"
#include "component.h"
#include "component_gen_only.h"
#include "component_consume_only.h"
#include "pipeline_buffer_decl.h"
#include "pipeline_buffer.h"
#include <chrono>
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

constexpr int num_loops = 100000;

int gen_1() { 
  static int state = 0;
  return state++;
};

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

  Component<BlockingQueue<int>> start{Gen_1};

  ComponentConsumeOnly<BlockingQueue<int>> end_component{Print_Input};

  component_input_ref<0L, ComponentConsumeOnly<BlockingQueue<int>>> end_ref{end_component};
  component_output_ref<Component<BlockingQueue<int>>> start_ref{start};

  auto pb = PipelineBuffer<int, int, component_output_ref<Component<BlockingQueue<int>>>, component_input_ref<0L, ComponentConsumeOnly<BlockingQueue<int>>>>::PipelineBuffer_factory(start_ref, end_ref);

  std::thread Start{start, std::ref(sig)};
  // std::thread WorkThread{work_component, std::ref(sig)};
  std::thread End{end_component, std::ref(sig)};

  std::this_thread::sleep_for(std::chrono::seconds(2));
  sig = false;

  Start.join();
  // WorkThread.join();
  End.join();

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