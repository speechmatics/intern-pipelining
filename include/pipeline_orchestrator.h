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
