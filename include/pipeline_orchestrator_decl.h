#pragma once
#include <array>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
#include <functional>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include "component.h"
#include "component_consume_only_decl.h"
#include "component_decl.h"
#include "pipeline_buffer_decl.h"

// static_string is a special type of string,
// which can be used as a non-type template parameter
template <std::size_t N>
struct static_string {
    std::array<char, N> buf;

    using array_type = const char [N];

    constexpr static_string(array_type& arr);

    constexpr bool operator==(const static_string& rhs) const;
};

// name_t is a special string type where the string data is stored
// within the type name
template <static_string str>
struct name_t {
  constexpr static auto value = std::string_view{str.buf.data(), str.buf.size()};
};

// name is just a wrapper around name_t for easier syntax for construction of name_t
template <static_string str>
constexpr auto name = name_t<str>{};

// kv is a struct representing a key-value pair
template <typename key, typename value>
struct kv {
    key k;
    value v;
    
    constexpr kv(key k, value v);
};

// A static_map is a compile-time map, holding a list of kvs
// get simply performs a linear search through the member buf
// this linear time complexity is not too much of a deal,
// since this search is all performed at compile time
// no lookups are performed at run-time
// This static_map is a crucial part of the puzzle to construct
// the entire pipeline at compile-time
template <std::size_t N, typename kv>
struct static_map {
    std::array<kv, N> buf;

    using array_type = const kv [N];

    template <std::size_t ...Idx>
    static constexpr std::array<kv, N> make_array_impl(array_type& arr, std::index_sequence<Idx...>);

    constexpr static_map(array_type& arr);

    constexpr decltype(std::declval<kv>().v) get(decltype(std::declval<kv>().k) key) const;
};

// A pipeline_module is the interface exposed to the user
// A user specifies the name to give to their component,
// the work_function and the names of other pipeline_modules
// which act as inputs to this work_function
// Order of these input names matters - it should match the order
// of arguments to the work_function
// InputNameTuple type is std::tuple<input name types>
template <typename ComponentName, typename InputNameTuple, typename out, typename... in>
struct pipeline_module {
    using function_type = std::function<
            out(const in...)
        >;

    function_type work_function;

    using component_type = std::conditional_t<std::is_same_v<void, out>, 
                                              ComponentConsumeOnly<PipelineBuffer<in>...>, 
                                              Component<PipelineBuffer<out>, PipelineBuffer<in>...>>;
    
    using output_type = out;
    using component_name_type = ComponentName;
    using input_name_tuple_type = InputNameTuple;

    template <typename ...InputName>
    pipeline_module(ComponentName, function_type work_function, InputName...);
};

// A deduction guide for the pipeline_module constructor
template <typename ComponentName, typename out, typename ...in, typename ...InputName>
pipeline_module(ComponentName, std::function<out(in...)>, InputName...)
    -> pipeline_module<ComponentName, std::tuple<InputName...>, out, in...>;

// A compile-time vector, allowing for a push_back operation
template <typename T, std::size_t N>
struct static_vector {
  std::array<T, N> buf{};
  std::size_t size = 0;

  constexpr void push_back(T t);
};

// For a given pm (pipeline_module), an input_backref
// holds the dependent pm's index (this index is in a static_map constructed in the beginning of the make_buffers implementation)
// and on which input port to connect to on the dependent pm
struct input_backref {
  std::size_t pm_idx{};
  std::size_t input_idx{};
};

// Pipeline is responsible for statically generating the pipeline form a bunch of pipeline_modules
template <typename... PM>
struct Pipeline {
  std::tuple<typename PM::component_type...> components;
  // changed out_type to output_type
  // Some components might by ComponentConsumeOnly, where they don't need a PipelineBuffer
  // So, we store a std::monostate in place of a PipelineBuffer at that place in the tuple
  // We do this, as the length of the PipelineBuffers tuple is the same as the Components tuple
  using pipeline_buffers_t = std::tuple<std::conditional_t<std::is_same_v<void, typename PM::output_type>, std::monostate, std::shared_ptr<PipelineBuffer<typename PM::output_type>>>...>;
  pipeline_buffers_t pipeline_buffers;
  std::atomic_bool sig{true};
  std::array<std::thread, sizeof... (PM)> threads;

  Pipeline(PM... pm);
  
  ~Pipeline() {
    for (auto& thread: threads) {
      thread.join();
    }
  }

  void start();
  
  void stop();

  private:
    static constexpr pipeline_buffers_t make_buffers(auto& components, PM... pm);
};
