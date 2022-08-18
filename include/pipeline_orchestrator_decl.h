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

template <std::size_t N>
struct static_string {
    std::array<char, N> buf;

    using array_type = const char [N];

    constexpr static_string(array_type& arr);

    constexpr bool operator==(const static_string& rhs) const;
};

template <static_string str>
struct name_t {
  constexpr static auto value = std::string_view{str.buf.data(), str.buf.size()};
};

template <static_string str>
constexpr auto name = name_t<str>{};

template <typename key, typename value>
struct kv {
    key k;
    value v;
    
    constexpr kv(key k, value v);
};

template <std::size_t N, typename kv>
struct static_map {
    std::array<kv, N> buf;

    using array_type = const kv [N];

    template <std::size_t ...Idx>
    static constexpr std::array<kv, N> make_array_impl(array_type& arr, std::index_sequence<Idx...>);

    constexpr static_map(array_type& arr);

    constexpr decltype(std::declval<kv>().v) get(decltype(std::declval<kv>().k) key) const;
};

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

template <typename ComponentName, typename out, typename ...in, typename ...InputName>
pipeline_module(ComponentName, std::function<out(in...)>, InputName...)
    -> pipeline_module<ComponentName, std::tuple<InputName...>, out, in...>;

template <typename T, std::size_t N>
struct static_vector {
  std::array<T, N> buf{};
  std::size_t size = 0;

  constexpr void push_back(T t);
};

struct input_backref {
  std::size_t pm_idx{};
  std::size_t input_idx{};
};

template <typename... PM>
struct Pipeline {
  std::tuple<typename PM::component_type...> components;
  // changed out_type to output_type
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
