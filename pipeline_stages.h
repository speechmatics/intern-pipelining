#pragma once
#include <atomic>
#include "blocking_queue.h"
#include "blocking_queue_decl.h"

void stage_one(BlockingQueue<int>& input, BlockingQueue<int>& output, std::atomic_bool& sig);

void stage_two(BlockingQueue<int>& input, BlockingQueue<int>& output, std::atomic_bool& sig);

void stage_three(BlockingQueue<int>& input, std::atomic_bool& sig); // Writes straight to stdout