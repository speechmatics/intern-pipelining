#pragma once
#include "pipeline_orchestrator_decl.h"

template <std::size_t N>
constexpr static_string<N>::static_string(array_type& arr) : buf{} {
    for (std::size_t idx=0; idx < N; ++idx) {
        buf[idx] = arr[idx];
    }
}

template <std::size_t N>
constexpr bool static_string<N>::operator==(const static_string& rhs) const {
    for (std::size_t idx{0}; idx < N; ++idx) {
        if (buf[idx] != rhs.buf[idx]) {
            return false;
        }
    }
    return true;
}

template <typename key, typename value>
constexpr kv<key, value>::kv(key k, value v) : k{k}, v{v} {};

template <std::size_t N, typename kv>
template <std::size_t ...Idx>
constexpr std::array<kv, N> static_map<N, kv>::make_array_impl(array_type& arr, std::index_sequence<Idx...>) {
    return std::array<kv, N>{arr[Idx]...};
}

template <std::size_t N, typename kv>
constexpr static_map<N, kv>::static_map(array_type& arr) : buf{make_array_impl(arr, std::make_index_sequence<N>{})} {}

template <std::size_t N, typename kv>
constexpr decltype(std::declval<kv>().v) static_map<N, kv>::get(decltype(std::declval<kv>().k) key) const {
    for (auto it = buf.begin(); it != buf.end(); ++it) {
        if (it->k == key) {
            return it->v;
        }
    }
    throw "Item not in static_map!";
}

template <typename ComponentName, typename InputNameTuple, typename out, typename... in>
template <typename ...InputName>
pipeline_module<ComponentName, InputNameTuple, out, in...>::pipeline_module(ComponentName component_name, function_type work_function, InputName... input_names) : 
    work_function{work_function} {};

template <typename T, std::size_t N>
constexpr void static_vector<T, N>::push_back(T t) {
    buf[size] = t;
    ++size;
  }

template <typename... PM>
Pipeline<PM...>::Pipeline(PM... pm) : components{pm.work_function...}, pipeline_buffers(make_buffers(components, pm...)), threads{
    std::apply([&](auto&... component) {
        return std::array<std::thread, sizeof...(PM)> {
            std::thread([&component, this](){ component(sig); })...
        };
    }, components)} {};

template <typename... PM>
void Pipeline<PM...>::start() {};

template <typename... PM> 
void Pipeline<PM...>::stop() {};

template <typename... PM> 
constexpr typename Pipeline<PM...>::pipeline_buffers_t Pipeline<PM...>::make_buffers(auto& components, PM... pm) {
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