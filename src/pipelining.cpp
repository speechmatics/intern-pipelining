#include "blocking_queue.h"
#include "component_decl.h"
#include "pipeline_stages.h"
#include "component.h"
#include "component_consume_only.h"
#include "pipeline_buffer_decl.h"
#include "pipeline_buffer.h"
#include "pipeline_orchestrator.h"
#include <chrono>
#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
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

int work(int x) { return x * 2; };

void work2(int x) { std::cout << "work2: " << x << std::endl; }

void print_input(int x) { std::cout << "print_input: " << x << std::endl; }

int main() {
  EASY_PROFILER_ENABLE;
  EASY_FUNCTION();
  printf("Hello World!\n");

  constexpr static_map smap{{
    kv{std::string_view{"Hello"}, 0},
    {{"World!"}, 1}
  }};
  constexpr auto x = smap.get(std::string_view("World!"));

  std::cout << "static_map: " << x << std::endl;  

  std::atomic_bool sig{true};

  std::function Gen_1 = gen_1;
  std::function Work = work;
  std::function Work2 = work2;
  std::function Print_Input = print_input;

  pipeline_module one{std::string_view{"start"}, Gen_1};
  pipeline_module two{std::string_view{"two"}, Work, std::string_view{"start"}};
  pipeline_module three{std::string_view{"three"}, Work2, std::string_view{"start"}};
  pipeline_module four{std::string_view{"four"}, Print_Input, std::string_view{"two"}};

  Pipeline p{one, two, three, four};
  p.start();

  return 0;

  Component<PipelineBuffer<int>> start_component{Gen_1};

  Component<PipelineBuffer<int>, PipelineBuffer<int>> middle_component{Work};

  ComponentConsumeOnly<PipelineBuffer<int>> middle_component_2{Work2};

  ComponentConsumeOnly<PipelineBuffer<int>> end_component{Print_Input};

  component_output_ref<Component<PipelineBuffer<int>>> start_ref{start_component};

  component_input_ref<0L, Component<PipelineBuffer<int>, PipelineBuffer<int>>> middle_input_ref{middle_component};
  component_output_ref<Component<PipelineBuffer<int>, PipelineBuffer<int>>> middle_output_ref{middle_component};

  component_input_ref<0L, ComponentConsumeOnly<PipelineBuffer<int>>> middle_input_2_ref{middle_component_2};

  component_input_ref<0L, ComponentConsumeOnly<PipelineBuffer<int>>> end_ref{end_component};
  

  auto pb = PipelineBuffer<int>::PipelineBuffer_factory(start_ref, middle_input_ref, middle_input_2_ref);

  auto pb2 = PipelineBuffer<int>::PipelineBuffer_factory(middle_output_ref, end_ref);

  std::thread Start{start_component, std::ref(sig)};
  std::thread WorkThread{middle_component, std::ref(sig)};
  std::thread WorkThread2{middle_component_2, std::ref(sig)};
  std::thread End{end_component, std::ref(sig)};

  std::this_thread::sleep_for(std::chrono::seconds(2));
  sig = false;

  Start.join();
  WorkThread.join();
  WorkThread2.join();
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