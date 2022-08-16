#pragma once
#include <array>
#include <algorithm>
#include <cstddef>
#include <string>
#include <functional>
#include <string_view>
#include <type_traits>
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
  constexpr static auto value = str;
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

template <typename out, typename... in>
struct pipeline_module {
    using function_type = std::function<
            out(const in...)
        >;

    std::string_view component_name;
    function_type work_function;
    std::array<std::string_view, sizeof... (in)> input_names;

    using component_type = std::conditional_t<std::is_same_v<void, out>, 
                                              ComponentConsumeOnly<in...>, 
                                              Component<out, in...>>;
    
    using output_type = out;

    constexpr pipeline_module(std::string_view component_name,
                              function_type work_function);
                              
    constexpr pipeline_module(std::string_view component_name,
                              function_type work_function,
                              std::string_view input_name...);
};

template <typename out, typename... in>
pipeline_module(std::string_view component_name,
                              std::function<out(in...)> work_function) -> pipeline_module<out>;

template <typename out, typename... in>
pipeline_module(std::string_view component_name,
                              std::function<out(in...)> work_function,
                              std::string_view input_name...) -> pipeline_module<out, in...>;

template <typename... PM>
struct Pipeline {
    std::tuple<typename PM::component_type...> components;
    std::tuple<std::conditional_t<std::is_same_v<void, typename PM::out_type>, std::monostate, std::shared_ptr<PipelineBuffer<typename PM::out_type>>>...> pipeline_buffers;
    Pipeline(PM... pm);

    void start();
    
    void stop();

    static constexpr auto make_buffers(auto& components, PM... pm) {
        auto names_to_indices = [&]<std::size_t... Idx>(std::index_sequence<Idx...>){
            return static_map{kv{pm.component_name, Idx}...};
        }(std::index_sequence_for<PM...>{});
    }
};