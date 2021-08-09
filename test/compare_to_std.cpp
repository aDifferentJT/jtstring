
#include <rapidcheck.h>

#include "jtstring.hpp"

#include <compare>
#include <sstream>
#include <string>
#include <string_view>

using namespace std::literals;

namespace rc {
  template<typename T>
  struct Arbitrary<std::basic_string_view<T>> {
    static Gen<std::basic_string_view<T>> arbitrary() {
      return gen::construct<std::basic_string_view<T>, std::basic_string<T>>();
    }
  };
}

auto tests(rc::Gen<std::string> strs) -> bool {
  auto results =
  { rc::check
    ( "size()"
    , [&] () {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_ASSERT(jtstr.size() == s.size());
      }
    )

  , rc::check
    ( "view()"
    , [&] () {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_ASSERT(jtstr.view() == s);
      }
    )

  , rc::check
    ( "swap(s, s)"
    , [&] {
        auto const s1 = *strs;
        auto const s2 = *strs;
        auto jtstr1 = jtstring{s1};
        auto jtstr2 = jtstring{s2};
        swap(jtstr1, jtstr2);
        RC_ASSERT(jtstr1 == s2);
        RC_ASSERT(jtstr2 == s1);
      }
    )

  , rc::check
    ( "jtstring{}"
    , [&] () {
        auto const s = std::string{};
        auto const jtstr = jtstring{};
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "jtstring{capacity}"
    , [&] () {
        auto const capacity = *rc::gen::withSize([](int size) { return rc::gen::inRange<std::size_t>(0, size); }).as("capacity");
        auto const s = std::string{};
        auto const jtstr = jtstring{capacity};
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "jtstring{jtstring&&}"
    , [&] {
        auto const s = *strs;
        auto jtstr1 = jtstring{s};
        auto const jtstr2 = jtstring{std::move(jtstr1)};
        RC_ASSERT(jtstr2 == s);
      }
    )

  , rc::check
    ( "operator=(jtstring&&)"
    , [&] {
        auto const s = *strs;
        auto jtstr1 = jtstring{s};
        auto jtstr2 = jtstring{};
        jtstr2 = std::move(jtstr1);
        RC_ASSERT(jtstr2 == s);
      }
    )

  , rc::check
    ( "operator=(sv)"
    , [&] {
        auto const s = *strs;
        auto jtstr = jtstring{};
        jtstr = s;
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "jtstring{jtstring const &}"
    , [&] {
        auto const s = *strs;
        auto const jtstr1 = jtstring{s};
        auto const jtstr2 = jtstring{jtstr1};
        RC_ASSERT(jtstr2 == jtstr1);
        RC_ASSERT(jtstr2 == s);
      }
    )

  , rc::check
    ( "operator=(jtstring const &)"
    , [&] {
        auto const s = *strs;
        auto const jtstr1 = jtstring{s};
        auto jtstr2 = jtstring{};
        jtstr2 = jtstr1;
        RC_ASSERT(jtstr2 == jtstr1);
        RC_ASSERT(jtstr2 == s);
      }
    )

  , rc::check
    ( "jtstring{char const *}"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s.c_str()};
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "operator=(char const *)"
    , [&] {
        auto const s = *strs;
        auto jtstr = jtstring{};
        jtstr = s.c_str();
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "at(i)"
    , [&] {
        auto s = *strs;
        auto const i = *rc::gen::inRange<std::size_t>(0, s.size()).as("i");
        auto jtstr = jtstring{s};
        RC_ASSERT(jtstr.at(i) == s.at(i));
      }
    )

  , rc::check
    ( "at(i) const"
    , [&] {
        auto const s = *strs;
        auto const i = *rc::gen::inRange<std::size_t>(0, s.size()).as("i");
        auto const jtstr = jtstring{s};
        RC_ASSERT(jtstr.at(i) == s.at(i));
      }
    )

  , rc::check
    ( "operator[](i)"
    , [&] {
        auto s = *strs;
        auto const i = *rc::gen::inRange<std::size_t>(0, s.size()).as("i");
        auto jtstr = jtstring{s};
        RC_ASSERT(jtstr[i] == s[i]);
      }
    )

  , rc::check
    ( "operator[](i) const"
    , [&] {
        auto const s = *strs;
        auto const i = *rc::gen::inRange<std::size_t>(0, s.size()).as("i");
        auto const jtstr = jtstring{s};
        RC_ASSERT(jtstr[i] == s[i]);
      }
    )

  , rc::check
    ( "front()"
    , [&] {
        auto s = *strs;
        auto jtstr = jtstring{s};
        RC_PRE(!s.empty());
        RC_ASSERT(jtstr.front() == s.front());
      }
    )

  , rc::check
    ( "front() const"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_PRE(!s.empty());
        RC_ASSERT(jtstr.front() == s.front());
      }
    )

  , rc::check
    ( "back()"
    , [&] {
        auto s = *strs;
        auto jtstr = jtstring{s};
        RC_PRE(!s.empty());
        RC_ASSERT(jtstr.back() == s.back());
      }
    )

  , rc::check
    ( "back() const"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_PRE(!s.empty());
        RC_ASSERT(jtstr.back() == s.back());
      }
    )

  , rc::check
    ( "c_str()"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_ASSERT(strcmp(jtstr.c_str(), s.c_str()) == 0);
      }
    )

  , rc::check
    ( "begin(), end()"
    , [&] {
        auto s = *strs;
        auto jtstr = jtstring{s};
        RC_ASSERT(std::equal(jtstr.begin(), jtstr.end(), s.begin(), s.end()));
      }
    )

  , rc::check
    ( "begin(), end() const"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_ASSERT(std::equal(jtstr.begin(), jtstr.end(), s.begin(), s.end()));
      }
    )

  , rc::check
    ( "cbegin(), cend()"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_ASSERT(std::equal(jtstr.cbegin(), jtstr.cend(), s.cbegin(), s.cend()));
      }
    )

  , rc::check
    ( "rbegin(), rend()"
    , [&] {
        auto s = *strs;
        auto jtstr = jtstring{s};
        RC_ASSERT(std::equal(jtstr.rbegin(), jtstr.rend(), s.rbegin(), s.rend()));
      }
    )

  , rc::check
    ( "rbegin(), rend() const"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_ASSERT(std::equal(jtstr.rbegin(), jtstr.rend(), s.rbegin(), s.rend()));
      }
    )

  , rc::check
    ( "crbegin(), crend()"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_ASSERT(std::equal(jtstr.crbegin(), jtstr.crend(), s.crbegin(), s.crend()));
      }
    )

  , rc::check
    ( "empty()"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_ASSERT(jtstr.empty() == s.empty());
      }
    )

  , rc::check
    ( "length()"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        RC_ASSERT(jtstr.length() == s.length());
      }
    )

  , rc::check
    ( "max_size()"
    , [&] {
        auto const s = *strs;
        auto const jtstr = jtstring{s};
        auto const max_size [[maybe_unused]] = jtstr.max_size();
      }
    )

  , rc::check
    ( "reserve()"
    , [&] {
        auto const s = *strs;
        auto const new_cap = *rc::gen::withSize([](int size) { return rc::gen::inRange<std::size_t>(0, size); }).as("new_cap");
        auto jtstr = jtstring{s};
        jtstr.reserve(new_cap);
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "shrink_to_fit()"
    , [&] {
        auto const s = *strs;
        auto jtstr = jtstring{s};
        jtstr.shrink_to_fit();
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "clear()"
    , [&] {
        auto s = *strs;
        auto jtstr = jtstring{s};
        s.clear();
        jtstr.clear();
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "insert(cpos, count, ch)"
    , [&] {
        auto s = *strs;
        auto const i = *rc::gen::inRange<std::size_t>(0, s.size()).as("i");
        auto const count = *rc::gen::withSize([](int size) { return rc::gen::inRange<std::size_t>(0, size); }).as("count");
        auto const ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        s.insert(i, count, ch);
        jtstr.insert(jtstr.begin() + i, count, ch);
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "insert(cpos, sv)"
    , [&] {
        auto s1 = *strs;
        auto const i = *rc::gen::inRange<std::size_t>(0, s1.size());
        auto const s2 = *strs;
        auto jtstr = jtstring{s1};
        s1.insert(i, s2);
        jtstr.insert(jtstr.begin() + i, s2);
        RC_ASSERT(jtstr == s1);
      }
    )

  , rc::check
    ( "erase(index, count)"
    , [&] {
        auto s = *strs;
        auto const index = *rc::gen::inRange<std::size_t>(0, s.size()).as("index");
        auto const count = *rc::gen::arbitrary<std::size_t>().as("count");
        auto jtstr = jtstring{s};
        s.erase(index, count);
        jtstr.erase(index, count);
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "push_back(ch)"
    , [&] {
        auto s = *strs;
        auto const ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        s.push_back(ch);
        jtstr.push_back(ch);
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "pop_back()"
    , [&] {
        auto s = *strs;
        RC_PRE(!s.empty());
        auto jtstr = jtstring{s};
        s.pop_back();
        jtstr.pop_back();
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "append(count, ch)"
    , [&] {
        auto s = *strs;
        auto count = *rc::gen::withSize([](int size) { return rc::gen::inRange<std::size_t>(0, size); }).as("count");
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        s.append(count, ch);
        jtstr.append(count, ch);
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "append(std::string_view)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr = jtstring{s1};
        s1.append(s2);
        jtstr.append(s2);
        RC_ASSERT(jtstr == s1);
      }
    )

  , rc::check
    ( "operator+=(std::string_view)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr = jtstring{s1};
        s1 += s2;
        jtstr += s2;
        RC_ASSERT(jtstr == s1);
      }
    )

  , rc::check
    ( "operator+=(char)"
    , [&] {
        auto s = *strs;
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        s += ch;
        jtstr += ch;
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "starts_with(s)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr = jtstring{s1};
        RC_ASSERT(jtstr.starts_with(s2) == s1.starts_with(s2));
      }
    )

  , rc::check
    ( "starts_with(ch)"
    , [&] {
        auto s = *strs;
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        RC_ASSERT(jtstr.starts_with(ch) == s.starts_with(ch));
      }
    )

  , rc::check
    ( "starts_with(char const *)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr = jtstring{s1};
        RC_ASSERT(jtstr.starts_with(s2.c_str()) == s1.starts_with(s2.c_str()));
      }
    )

  , rc::check
    ( "(s1 + s2).starts_with(s1)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((jtstr1 + jtstr2).starts_with(s1));
      }
    )

  , rc::check
    ( "(ch + s).starts_with(ch)"
    , [&] {
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto s = *strs;
        auto jtstr = jtstring{s};
        RC_ASSERT((ch + jtstr).starts_with(ch));
      }
    )

  , rc::check
    ( "(s1 + s2).starts_with(s1.c_str())"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((jtstr1 + jtstr2).starts_with(s1.c_str()));
      }
    )

  , rc::check
    ( "ends_with(s)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr = jtstring{s1};
        RC_ASSERT(jtstr.ends_with(s2) == s1.ends_with(s2));
      }
    )

  , rc::check
    ( "ends_with(ch)"
    , [&] {
        auto s = *strs;
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        RC_ASSERT(jtstr.ends_with(ch) == s.ends_with(ch));
      }
    )

  , rc::check
    ( "ends_with(char const *)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr = jtstring{s1};
        RC_ASSERT(jtstr.ends_with(s2.c_str()) == s1.ends_with(s2.c_str()));
      }
    )

  , rc::check
    ( "(s1 + s2).ends_with(s2)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((jtstr1 + jtstr2).ends_with(s2));
      }
    )

  , rc::check
    ( "(s + ch).ends_with(ch)"
    , [&] {
        auto s = *strs;
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        RC_ASSERT((jtstr + ch).ends_with(ch));
      }
    )

  , rc::check
    ( "(s1 + s2).ends_with(s2.c_str())"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((jtstr1 + jtstr2).ends_with(s2.c_str()));
      }
    )

  , rc::check
    ( "substr(pos, count)"
    , [&] {
        auto s = *strs;
        auto pos = *rc::gen::inRange<std::size_t>(0, s.size()).as("pos");
        auto count = *rc::gen::arbitrary<std::size_t>().as("count");
        auto jtstr = jtstring{s};
        RC_ASSERT(jtstr.substr(pos, count) == s.substr(pos, count));
      }
    )

  , rc::check
    ( "copy(dest, count, pos)"
    , [&] {
        auto s = *strs;
        auto count = *rc::gen::withSize([](int size) { return rc::gen::inRange<std::size_t>(0, size); }).as("count");
        auto pos = *rc::gen::inRange<std::size_t>(0, s.size()).as("pos");
        auto jtstr = jtstring{s};
        auto dest1 = std::vector<char>(count, '\0');
        auto dest2 = std::vector<char>(count, '\0');
        s.copy(dest1.data(), count, pos);
        jtstr.copy(dest2.data(), count, pos);
        RC_ASSERT(dest1 == dest2);
      }
    )

  , rc::check
    ( "resize(count, ch)"
    , [&] {
        auto s = *strs;
        auto count = *rc::gen::withSize([](int size) { return rc::gen::inRange<std::size_t>(0, size); }).as("count");
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        s.resize(count, ch);
        jtstr.resize(count, ch);
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "resize(count)"
    , [&] {
        auto s = *strs;
        auto count = *rc::gen::withSize([](int size) { return rc::gen::inRange<std::size_t>(0, size); }).as("count");
        auto jtstr = jtstring{s};
        s.resize(count);
        jtstr.resize(count);
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "operator+(s, s)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((jtstr1 + jtstr2) == s1 + s2);
      }
    )

  , rc::check
    ( "operator+(s, sv)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        RC_ASSERT(jtstr1 + s2 == s1 + s2);
      }
    )

  , rc::check
    ( "operator+(sv, s)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr2 = jtstring{s2};
        RC_ASSERT(s1 + jtstr2 == s1 + s2);
      }
    )

  , rc::check
    ( "operator+(s, ch)"
    , [&] {
        auto s = *strs;
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        RC_ASSERT(jtstr + ch == s + ch);
      }
    )

  , rc::check
    ( "operator+(ch, s)"
    , [&] {
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto s = *strs;
        auto jtstr = jtstring{s};
        RC_ASSERT(ch + jtstr == ch + s);
      }
    )

  , rc::check
    ( "operator+(s&&, s)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((std::move(jtstr1) + jtstr2) == s1 + s2);
      }
    )

  , rc::check
    ( "operator+(s&&, sv)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        RC_ASSERT((std::move(jtstr1) + s2) == s1 + s2);
      }
    )

  , rc::check
    ( "operator+(sv, s&&)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((s1 + std::move(jtstr2)) == s1 + s2);
      }
    )

  , rc::check
    ( "operator+(s&&, ch)"
    , [&] {
        auto s = *strs;
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto jtstr = jtstring{s};
        RC_ASSERT((std::move(jtstr) + ch) == s + ch);
      }
    )

  , rc::check
    ( "operator+(ch, s&&)"
    , [&] {
        auto ch = *rc::gen::arbitrary<char>().as("ch");
        auto s = *strs;
        auto jtstr = jtstring{s};
        RC_ASSERT((ch + std::move(jtstr)) == ch + s);
      }
    )

  , rc::check
    ( "operator==(s, s)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((jtstr1 == jtstr2) == (s1 == s2));
      }
    )

  , rc::check
    ( "operator==(s, sv)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        RC_ASSERT((jtstr1 == s2) == (s1 == s2));
      }
    )

  , rc::check
    ( "operator==(sv, s)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((s1 == jtstr2) == (s1 == s2));
      }
    )

  , rc::check
    ( "operator==(s, s) reflexive"
    , [&] {
        auto s = *strs;
        auto jtstr1 = jtstring{s};
        auto jtstr2 = jtstring{s};
        RC_ASSERT(jtstr1 == jtstr1);
        RC_ASSERT(jtstr1 == jtstr2);
      }
    )

  , rc::check
    ( "operator==(s, sv) reflexive"
    , [&] {
        auto s = *strs;
        auto jtstr = jtstring{s};
        RC_ASSERT(jtstr == s);
      }
    )

  , rc::check
    ( "operator==(sv, s) reflexive"
    , [&] {
        auto s = *strs;
        auto jtstr = jtstring{s};
        RC_ASSERT(s == jtstr);
      }
    )

  , rc::check
    ( "operator<=>(s, s)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((jtstr1 <=> jtstr2) == (s1 <=> s2));
      }
    )

  , rc::check
    ( "operator<=>(s, sv)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr1 = jtstring{s1};
        RC_ASSERT((jtstr1 <=> s2) == (s1 <=> s2));
      }
    )

  , rc::check
    ( "operator<=>(sv, s)"
    , [&] {
        auto s1 = *strs;
        auto s2 = *strs;
        auto jtstr2 = jtstring{s2};
        RC_ASSERT((s1 <=> jtstr2) == (s1 <=> s2));
      }
    )

  , rc::check
    ( "operator<<(os, s)"
    , [&] {
        auto s = *strs;
        auto ss1 = std::stringstream{};
        auto ss2 = std::stringstream{};
        auto jtstr = jtstring{s};
        ss1 << s;
        ss2 << jtstr;
        RC_ASSERT(ss1.str() == ss2.str());
      }
    )
  };

  for (auto result : results) {
    if (!result) { return false; }
  }
  return true;
}

int main(int, char**) {
  return
    (  tests(rc::gen::string<std::string>())
//    && tests(rc::gen::arbitrary<std::string>())
    ) ? EXIT_SUCCESS : EXIT_FAILURE;
}

