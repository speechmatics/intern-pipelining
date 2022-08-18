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

class Gen_1_Class {
  int state = 0;
  public:
    int operator()() {
      return state++;
    }
};

int work(int x) { return x * 2; };

void work2(int x) { std::cout << "work2: " << x << std::endl; }

void print_input(int x) { std::cout << "print_input: " << x << std::endl; }

int main() {
  EASY_PROFILER_ENABLE;
  EASY_FUNCTION();
  printf("Hello World!\n");

  std::function Gen_1 = Gen_1_Class{};
  std::function Work = work;
  std::function Work2 = work2;
  std::function Print_Input = print_input;

  pipeline_module one{name<"start">, Gen_1};
  pipeline_module two{name<"two">, Work, name<"start">};
  pipeline_module three{name<"three">, Work2, name<"start">};
  pipeline_module four{name<"four">, Print_Input, name<"two">};

  Pipeline p{one, two, three, four};
  p.start();

  int no_blocks_written = profiler::dumpBlocksToFile("pipelining.prof");
  printf("Wrote %i easy_profiler blocks\n", no_blocks_written);
  return 0;
}