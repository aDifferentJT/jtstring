// Type your code here, or load an example.
#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <utility>

enum struct jtstring_mask : int8_t { small = 0, large = -1 };

struct jtstring_small {
  static constexpr auto capacity = std::size_t{30};
  uint8_t size;
  std::array<char, capacity> data;
  jtstring_mask mask;

  jtstring_small(std::size_t size = 0) noexcept
    : size{static_cast<uint8_t>(size)}
    , mask{jtstring_mask::small}
    {}
};

struct jtstring_large {
  std::size_t size;
  std::unique_ptr<char[]> data;
  std::size_t capacity_less_sso;
  std::array<char, 7> padding;
  jtstring_mask mask;

  jtstring_large(std::size_t size, std::size_t capacity)
    : size{size}
    // TODO: Better for c++20
    //, data{std::make_unique_for_overwrite<char[]>(capacity + 1)}
    , data{std::make_unique<char[]>(capacity + 1)}
    , capacity_less_sso{capacity - jtstring_small::capacity}
    , mask{jtstring_mask::large}
    {}
};

static_assert(sizeof(jtstring_large) == sizeof(jtstring_small), "Short string and long string should be the same size");
static_assert(offsetof(jtstring_large, size) == offsetof(jtstring_small, size), "Short string and long string should have size at the same offset");
static_assert(offsetof(jtstring_large, mask) == offsetof(jtstring_small, mask), "Short string and long string should have mask at the same offset");

class jtstring {
  public:
    static constexpr auto npos = static_cast<std::size_t>(-1);

  private:
    union {
      jtstring_large large;
      jtstring_small small;
    };

    [[nodiscard]] auto mask_sx() const noexcept -> uintptr_t {
      return static_cast<intptr_t>(large.mask);
    }

    void set_size(std::size_t size) noexcept {
      large.size = size | (large.size & (~this->mask_sx() << 8));
    }

  public:
    [[nodiscard]] auto data() noexcept -> char * {
      auto const mask = mask_sx();
      return reinterpret_cast<char *>((mask & reinterpret_cast<uintptr_t>(large.data.get())) | (~mask & reinterpret_cast<uintptr_t>(&small.data)));
    }

    [[nodiscard]] auto data() const noexcept -> char const * {
      auto const mask = mask_sx();
      return reinterpret_cast<char const *>((mask & reinterpret_cast<uintptr_t>(large.data.get())) | (~mask & reinterpret_cast<uintptr_t>(&small.data)));
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t {
      auto const mask = mask_sx() | 0xFF;
      return large.size & mask;
    }

    [[nodiscard]] auto capacity() const noexcept -> std::size_t {
      auto const mask = mask_sx();
      return small.data.size() + (large.capacity_less_sso & mask);
    }

    [[nodiscard]] auto view() const { return std::string_view{data(), size()}; }

    [[nodiscard]] auto begin()       noexcept { return data(); }
    [[nodiscard]] auto begin() const noexcept { return data(); }
    [[nodiscard]] auto end()       noexcept { return data() + size(); }
    [[nodiscard]] auto end() const noexcept { return data() + size(); }

    friend void swap(jtstring & lhs, jtstring & rhs) {
      std::swap(lhs.small, rhs.small);
    }

    jtstring() {
      new(&small) jtstring_small{};
      small.data[0] = '\0';
    }

    jtstring(std::size_t capacity) {
      if (capacity <= jtstring_small::capacity) {
        new(&small) jtstring_small{0};
        small.data[0] = '\0';
      } else {
        new(&large) jtstring_large{0, capacity};
        large.data[0] = '\0';
      }
    }

  private:
    jtstring(std::size_t size, std::size_t capacity, char** it) {
      if (capacity <= jtstring_small::capacity) {
        new(&small) jtstring_small{size};
        *it = small.data.data();
      } else {
        new(&large) jtstring_large{size, capacity};
        *it = large.data.get();
      }
    }

  public:
    ~jtstring() {
      if (static_cast<bool>(large.mask)) {
        large.~jtstring_large();
      }
    }

    jtstring(jtstring&& that) : small{std::exchange(that.small, {})} {}

    jtstring& operator=(jtstring&& that) {
      auto tmp = std::move(that);
      swap(*this, tmp);
      return *this;
    }

  private:
    jtstring(std::string_view view, char * it)
      : jtstring{view.size(), view.size(), &it}
    {
      it = std::copy(view.begin(), view.end(), it);
      *it = '\0';
    }
  public:
    jtstring(std::string_view view) : jtstring{view, nullptr} {}

    jtstring& operator=(std::string_view view) {
      if (view.size() <= this->capacity()) {
        set_size(view.size());
        auto it = std::copy(view.begin(), view.end(), this->data());
        *it = '\0';
      } else {
        auto tmp = jtstring{view};
        swap(*this, tmp);
      }
      return *this;
    }

    jtstring(jtstring const & that) : jtstring{that.view()} {}

    jtstring& operator=(jtstring const & that) { return *this = that.view(); }

    jtstring(char const * str) : jtstring{std::string_view{str}} {}
    jtstring& operator=(char const * str) { return *this = std::string_view{str}; }

    auto at(std::size_t i) -> char & {
      if (i >= size()) {
        throw std::out_of_range{"jtstring: Index out of range"};
      } else {
        return data()[i];
      }
    }
    auto at(std::size_t i) const -> char const & {
      if (i >= size()) {
        throw std::out_of_range{"jtstring: Index out of range"};
      } else {
        return data()[i];
      }
    }
    [[nodiscard]] auto operator[](std::size_t i)       -> char       & { return data()[i]; }
    [[nodiscard]] auto operator[](std::size_t i) const -> char const & { return data()[i]; }

    [[nodiscard]] auto front()       -> char       & { return (*this)[0]; }
    [[nodiscard]] auto front() const -> char const & { return (*this)[0]; }
    [[nodiscard]] auto back()       -> char       & { return (*this)[size() - 1]; }
    [[nodiscard]] auto back() const -> char const & { return (*this)[size() - 1]; }

    [[nodiscard]] auto c_str() const { return data(); }

    [[nodiscard]] operator std::string_view() const { return view(); }

    [[nodiscard]] auto rbegin()       { return std::reverse_iterator{end()}; }
    [[nodiscard]] auto rbegin() const { return std::reverse_iterator{end()}; }
    [[nodiscard]] auto rend()       { return std::reverse_iterator{begin()}; }
    [[nodiscard]] auto rend() const { return std::reverse_iterator{begin()}; }

    [[nodiscard]] auto cbegin() const { return begin(); }
    [[nodiscard]] auto cend() const { return end(); }
    [[nodiscard]] auto crbegin() const { return rbegin(); }
    [[nodiscard]] auto crend() const { return rend(); }

    [[nodiscard]] auto empty() const -> bool { return size() == 0; }

    [[nodiscard]] auto length() const { return size(); }

    [[nodiscard]] auto max_size() const noexcept {
      return std::numeric_limits<std::size_t>::max();
    }

    void reserve(std::size_t new_cap) {
      if (new_cap > capacity()) {
        char * it;
        auto tmp = jtstring{size(), new_cap, &it};
        it = std::copy(begin(), end(), it);
        *it = '\0';
        swap(*this, tmp);
      }
    }

    void shrink_to_fit() {
      if (size() < capacity()) {
        char * it;
        auto tmp = jtstring{size(), size(), &it};
        it = std::copy(begin(), end(), it);
        *it = '\0';
        swap(*this, tmp);
      }
    }

    void clear() {
      set_size(0);
      data()[0] = '\0';
    }

    auto insert(char const * cpos, std::size_t count, char ch) -> jtstring & {
      if (size() + count <= capacity()) {
        auto pos = const_cast<char *>(cpos);
        auto it = std::copy(cpos, cend(), pos + count);
        *it = '\0';
        std::fill_n(pos, count, ch);
        set_size(size() + count);
      } else {
        char * it;
        auto tmp = jtstring{size() + count, (size() + count) * 2, &it};
        it = std::copy(cbegin(), cpos, it);
        it = std::fill_n(it, count, ch);
        it = std::copy(cpos, cend(), it);
        *it = '\0';
        swap(*this, tmp);
      }
      return *this;
    }

    auto insert(char const * cpos, std::string_view view) -> jtstring & {
      if (size() + view.size() <= capacity()) {
        auto pos = const_cast<char *>(cpos);
        auto it = std::copy(cpos, cend(), pos + view.size());
        *it = '\0';
        std::copy(view.begin(), view.end(), pos);
        set_size(size() + view.size());
      } else {
        char * it;
        auto tmp = jtstring{size() + view.size(), (size() + view.size()) * 2, &it};
        it = std::copy(cbegin(), cpos, it);
        it = std::copy(view.begin(), view.end(), it);
        it = std::copy(cpos, cend(), it);
        *it = '\0';
        swap(*this, tmp);
      }
      return *this;
    }

    auto erase(char const * first, char const * last) noexcept -> char * {
      auto first2 = const_cast<char *>(first);
      for (auto end_it = end(); last != end_it; first2 += 1, last += 1) {
        *first2 = *last;
      }
      set_size(first2 - begin());
      *first2 = '\0';
      return const_cast<char *>(first);
    }

    auto erase(std::size_t index = 0, std::size_t count = npos) -> jtstring & {
      if (index > size()) {
        throw std::out_of_range{"jtstring: erase index out of range"};
      }
      erase(begin() + index, begin() + index + std::min(count, size() - index));
      return *this;
    }

    auto erase(char const * position) {
      return erase(position, position + 1);
    }

    void push_back(char ch) {
      if (size() < capacity()) {
        data()[size()] = ch;
        set_size(size() + 1);
        data()[size()] = '\0';
      } else {
        char * it;
        auto tmp = jtstring{size() + 1, 2 * capacity(), &it};
        it = std::copy(begin(), end(), it);
        *(it++) = ch;
        *(it++) = '\0';
        swap(*this, tmp);
      }
    }

    void pop_back() { erase(end() - 1); }

    auto append(std::size_t count, char ch) -> jtstring & {
      if (size() + count <= capacity()) {
        auto it = std::fill_n(end(), count, ch);
        *it = '\0';
        set_size(size() + count);
      } else {
        char * it;
        auto tmp = jtstring{size() + count, (size() + count) * 2, &it};
        it = std::copy(begin(), end(), it);
        it = std::fill_n(it, count, ch);
        swap(*this, tmp);
      }
      return *this;
    }

    auto append(std::string_view view) -> jtstring & {
      if (size() + view.size() <= capacity()) {
        auto it = std::copy(view.begin(), view.end(), end());
        *it = '\0';
        set_size(size() + view.size());
      } else {
        char * it;
        auto tmp = jtstring{size() + view.size(), (size() + view.size()) * 2, &it};
        it = std::copy(begin(), end(), it);
        it = std::copy(view.begin(), view.end(), it);
        swap(*this, tmp);
      }
      return *this;
    }

    auto operator+=(std::string_view rhs) -> jtstring & {
      return append(rhs);
    }

    auto operator+=(char ch) -> jtstring & {
      return *this += {&ch, 1};
    }

    // TODO compare

    auto starts_with(std::string_view sv) const noexcept -> bool {
      if (size() < sv.size()) {
        return false;
      }

      auto it = begin();
      for (auto c : sv) {
        if (c != *it) {
          return false;
        }
        it += 1;
      }
      return true;
    }

    auto starts_with(char c) const noexcept -> bool {
      return size() > 0 && front() == c;
    }

    auto starts_with(char const * str) const -> bool {  
      for (auto it = begin(); *str != '\0'; it += 1, str += 1) {
        if (it == end() || *it != *str) {
          return false;
        }
      }
      return true;
    }

    auto ends_with(std::string_view sv) const noexcept -> bool {
      if (size() < sv.size()) {
        return false;
      }

      auto it = end() - sv.size();
      for (auto c : sv) {
        if (c != *it) {
          return false;
        }
        it += 1;
      }
      return true;
    }

    auto ends_with(char c) const noexcept -> bool {
      return size() > 0 && back() == c;
    }

    auto ends_with(char const * str) const -> bool {
      return ends_with(std::string_view{str});
    }

    // TODO: contains

    // TODO: replace

    auto substr(std::size_t pos = 0, std::size_t count = npos) const -> jtstring {
      if (pos > size()) {
        throw std::out_of_range{"jtstring: substr pos out of range"};
      }
      auto const count2 = std::min(count, size() - pos);
      char * it;
      auto ret = jtstring{count2, count2, &it};
      it = std::copy(begin() + pos, begin() + pos + count2, it);
      *it = '\0';
      return ret;
    }

    auto copy(char * dest, std::size_t count, std::size_t pos = 0) const -> std::size_t {
      if (pos > size()) {
        throw std::out_of_range{"jtstring: copy pos out of range"};
      }
      return std::copy(begin() + pos, begin() + pos + std::min(count, size() - pos), dest) - (begin() + pos);
    }

    void resize(std::size_t count, char ch) {
      if (count <= capacity()) {
        auto it = end();
        for (; it < begin() + count; it += 1) {
          *it = ch;
        }
        *it = '\0';
        set_size(count);
      } else {
        char * it;
        auto tmp = jtstring{count, count, &it};
        it = std::copy(begin(), begin() + std::min(count, size()), it);
        for (; it < tmp.begin() + count; it += 1) {
          *it = ch;
        }
        *it = '\0';
        swap(*this, tmp);
      }
    }

    void resize(std::size_t count) {
      resize(count, char{});
    }

  private:
    [[nodiscard]] static auto concat(std::initializer_list<std::string_view> views) -> jtstring { 
      char * it;

      auto size = std::accumulate
        ( views.begin()
        , views.end()
        , std::size_t{0}
        , [](std::size_t acc, std::string_view view) {
            return acc + view.size();
          }
        );

      auto ret = jtstring{size, size, &it};

      for (auto view : views) {
        it = std::copy(view.begin(), view.end(), it);
      }
      *it = '\0';

      return ret;
    }

  public:
    [[nodiscard]] friend auto operator+(jtstring const & lhs, jtstring const & rhs) -> jtstring {
      return concat({lhs, rhs});
    }

    [[nodiscard]] friend auto operator+(jtstring const & lhs, std::string_view rhs) -> jtstring {
      return concat({lhs, rhs});
    }

    [[nodiscard]] friend auto operator+(std::string_view lhs, jtstring const & rhs) -> jtstring {
      return concat({lhs, rhs});
    }

    [[nodiscard]] friend auto operator+(jtstring const & lhs, char rhs) -> jtstring {
      return lhs + std::string_view{&rhs, 1};
    }

    [[nodiscard]] friend auto operator+(char lhs, jtstring const & rhs) -> jtstring {
      return std::string_view{&lhs, 1} + rhs;
    }

    [[nodiscard]] friend auto operator+(jtstring&& lhs, jtstring const & rhs) -> jtstring {
      lhs.append(rhs);
      return std::move(lhs);
    }

    [[nodiscard]] friend auto operator+(jtstring&& lhs, std::string_view rhs) -> jtstring {
      lhs.append(rhs);
      return std::move(lhs);
    }

    [[nodiscard]] friend auto operator+(std::string_view lhs, jtstring&& rhs) -> jtstring {
      rhs.insert(rhs.begin(), lhs);
      return std::move(rhs);
    }

    [[nodiscard]] friend auto operator+(jtstring&& lhs, char rhs) -> jtstring {
      lhs.append(1, rhs);
      return std::move(lhs);
    }

    [[nodiscard]] friend auto operator+(char lhs, jtstring&& rhs) -> jtstring {
      rhs.insert(rhs.begin(), 1, lhs);
      return std::move(rhs);
    }

    [[nodiscard]] friend auto operator==(jtstring const & lhs, jtstring const & rhs) {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    [[nodiscard]] friend auto operator==(jtstring const & lhs, std::string_view rhs) {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    [[nodiscard]] friend auto operator==(std::string_view lhs, jtstring const & rhs) {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    [[nodiscard]] friend auto operator<=>(jtstring const & lhs, jtstring const & rhs) -> std::weak_ordering {
      return lhs.view() <=> rhs.view();
    }

    [[nodiscard]] friend auto operator<=>(jtstring const & lhs, std::string_view rhs) -> std::weak_ordering {
      return lhs.view() <=> rhs;
    }

    [[nodiscard]] friend auto operator<=>(std::string_view lhs, jtstring const & rhs) -> std::weak_ordering {
      return lhs <=> rhs.view();
    }

    friend auto operator<<(std::basic_ostream<char>& os, jtstring const & str) -> std::basic_ostream<char> & {
      return os << str.view();
    }
};

