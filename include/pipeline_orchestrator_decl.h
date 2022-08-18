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

  constexpr void push_back(T t) {
    buf[size] = t;
    ++size;
  }
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

    static constexpr pipeline_buffers_t make_buffers(auto& components, PM... pm) {
        constexpr auto names_to_indices = [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
            // changed pm.component_name
            return static_map{{kv{PM::component_name_type::value, Idx}...}};
        }(std::index_sequence_for<PM...>{});

        // Stores which pms use this pm as input and on which port

        // pm_dependents_and_ports[idx]: array of pairs of pm and input port idx,
        // idx is upstream to the pms in the array,
        // idx's output is used as the input of each pm in the array at the corresponding port idx
        constexpr auto pm_dependents_and_ports = [&]<std::size_t... outer_pm_index>(
            std::index_sequence<outer_pm_index...>) {
            std::array<static_vector<input_backref, sizeof...(PM)>,
                       sizeof...(PM)>
                ret;
            (std::apply(
                 // Iterate over all of the input_names inside a pm
                 [&](auto... input_names) {
                   return [&]<std::size_t... input_port_idx>(
                       std::index_sequence<input_port_idx...>, std::size_t _outer_pm_index) {
                     (
                         [&] {
                           constexpr auto input_idx =
                               names_to_indices.get(input_names.value);
                           ret[input_idx].push_back(input_backref{_outer_pm_index, input_port_idx});
                         }(),
                         ...);
                   }
                   (std::make_index_sequence<sizeof...(input_names)>{}, outer_pm_index);
                 },
                 typename PM::input_name_tuple_type{}),
             ...);
            return ret;
        }
        (std::index_sequence_for<PM...>{});

        return [&]<std::size_t... PM_Idx>(std::index_sequence<PM_Idx...>) {
            return pipeline_buffers_t{[&] {
              if constexpr (std::is_same_v<void, typename PM::output_type>) {
                return std::monostate{};
              } else {
                return [&]<std::size_t... Dependent_PM_Idx, std::size_t pm_idx>(
                    std::index_sequence<Dependent_PM_Idx...>, std::integral_constant<std::size_t, pm_idx>) {
                  return PipelineBuffer<typename PM::output_type>::
                      PipelineBuffer_factory(
                          component_output_ref{std::get<pm_idx>(components)},
                          [&] {
                            constexpr std::size_t input_idx =
                                pm_dependents_and_ports[pm_idx].buf[Dependent_PM_Idx].input_idx;
                            constexpr std::size_t component_idx =
                                pm_dependents_and_ports[pm_idx].buf[Dependent_PM_Idx].pm_idx;
                            return component_input_ref_factory<input_idx>(
                                std::get<component_idx>(components));
                          }()...);
                }
                (std::make_index_sequence<pm_dependents_and_ports[PM_Idx].size>{},
                 std::integral_constant<std::size_t, PM_Idx>{});
              }
            }()...};
        }
        (std::index_sequence_for<PM...>{});
    }
};
