#include <iostream>
#include <string>
#include <experimental/string_view>
#include <type_traits>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace stdexp = std::experimental;

template<typename T> constexpr std::tuple<T, T> get_fnv1a_config() noexcept;

template<> constexpr std::tuple<uint32_t, uint32_t> get_fnv1a_config() noexcept {
    return std::make_tuple(0x811c9dc5u, 16777619u);
}

template<> constexpr std::tuple<uint64_t, uint64_t> get_fnv1a_config() noexcept {
    return std::make_tuple(0xcbf29ce484222325ull, 1099511628211ull);
}

template<typename T>
constexpr T fnv1a_internal(const char * str) noexcept
{
    auto [basis, prime] = get_fnv1a_config<T>();
    uint32_t index = 0;
    T hash = basis;
    while(str[index] != 0) {
        hash = hash ^ str[index];
        hash = hash * prime;
        ++index;
    }
    return hash;
}

template<typename T>
class fnv1a_hash
{
public:
    constexpr fnv1a_hash(const char * str) noexcept : hash_(fnv1a_internal<T>(str))
    {}

    constexpr fnv1a_hash(stdexp::string_view str) noexcept : fnv1a_hash(str.data())
    {}

    constexpr T get() const noexcept {
        return hash_;
    }
private:
    T hash_;
};

using fnv1a32 = fnv1a_hash<uint32_t>;
using fnv1a64 = fnv1a_hash<uint64_t>;

void collider(const int n)
{
    std::unordered_map<uint32_t, std::string> entries;
    std::string next_str = "0";
    auto len = 1;

    int c = 0;
    while(c < n) {
        auto next_hash = fnv1a32(next_str).get();
        auto iter = entries.find(next_hash);

        if(iter == entries.end()) {
            entries.insert_or_assign(next_hash, next_str);
        }
        else {
            std::cout << "Collision " << iter->second << " (" << iter->first << "), " << next_str << " (" << next_hash << ")" << std::endl;
            ++c;
        }

        if(next_str[0] != '9' && next_str[0] < 'z')
        {
            ++next_str[0];
        }
        else if(next_str[0] == '9') {
            next_str[0] = 'a';
        }
        else
        {
            next_str[0] = 'a';
            int j = 1;
            for(; j<len; ++j) {
                if(next_str[j] != '9' && next_str[j] < 'z') {
                    ++next_str[j];
                    break;
                }
                else if(next_str[j] == '9') {
                    next_str[j] = 'a';
                    break;
                }
            }
            if(j == len) {
                for(j=0; j<len; ++j) {
                    next_str[j] = '0';
                }
                next_str += '0';
                ++len;
            }
        }
    }
}

int main() {
    constexpr auto str = "alamakota";
    std::cout << fnv1a_internal<uint32_t>(str) << std::endl;
    std::cout << fnv1a_internal<uint64_t>(str) << std::endl;
    std::cout << fnv1a32{str}.get() << std::endl;
    static_assert(std::is_literal_type<std::tuple<uint, uint>>::value);
    static_assert(fnv1a64{str}.get() != 0);
    std::get<0>(get_fnv1a_config<uint>());

    static_assert(fnv1a32{"szzzzzzzzzo"}.get() == fnv1a32{"ezzzzzzzzzzzu00000000"}.get());

    return 0;
}