#pragma once
#include <array>
#include <algorithm>
#include <cstddef>

template <std::size_t N>
struct static_string {
    std::array<char, N> buf;

    using array_type = const char [N];

    constexpr static_string(array_type& arr) : buf{} {
        for (std::size_t idx=0; idx < N; ++idx){
            buf[idx] = arr[idx];
        }
    }

    constexpr bool operator==(const static_string& rhs) const {
        for (std::size_t idx{0}; idx < N; ++idx) {
            if (buf[idx] != rhs.buf[idx]) {
                return false;
            }
        }
        return true;
    }
};

template <typename key, typename value>
struct kv {
    key k;
    value v;
    
    constexpr kv(key k, value v) : k{k}, v{v} {};
};

template <std::size_t ...Idx>
struct index_sequence {};

template <std::size_t N, std::size_t... Indx>
constexpr auto append(index_sequence<Indx...>) {
    return index_sequence<Indx..., N> {};
}

template <int x>
constexpr auto make_index() {
    if constexpr (x == 0) {
        return index_sequence<> {};
    } else {
        return append<x>(make_index<x-1>());
    }
}



// template <typename Seq, std::size_t N>
// struct append;

// template <std::size_t ...Idx, std::size_t N>
// struct append<index_sequence<Idx...>, N> {
//     using type = index_sequence<Idx..., N>;
// };

// template <std::size_t N>
// struct make_index {
//     using type = typename append<
//        typename make_index<N-1>::type, N
//     >::type;
// };

// template <>
// struct make_index<0> {
//     using type = index_sequence<>;
// };

// template <std::size_t N>
// using make_index_sequence = typename make_index<N>::type;

template <std::size_t N, typename kv>
struct static_map {
    std::array<kv, N> buf;

    using array_type = const kv [N];

    template <std::size_t ...Idx>
    static constexpr std::array<kv, N> make_array_impl(array_type& arr, std::index_sequence<Idx...>) {
        return std::array<kv, N>{arr[Idx]...};
    }

    constexpr static_map(array_type& arr) : buf{make_array_impl(arr, std::make_index_sequence<N>{})} {
    }

    constexpr decltype(auto) get(decltype(std::declval<kv>().k) key) const {
        for (auto it = buf.begin(); it != buf.end(); ++it) {
            if (it->k == key) {
                return it->v;
            }
        }
        throw "Item not in static_map!";
    }
};