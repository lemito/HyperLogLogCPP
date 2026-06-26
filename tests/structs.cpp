#include <iostream>

#include "hyperll.hpp"

struct MyStruct {
  std::string name;
  int64_t age;
  double value;
  bool alpha;
};

template <> struct std::hash<MyStruct> {
  auto operator()(const MyStruct &s) const noexcept {
    const auto h1 = std::hash<std::string>{}(s.name);
    const auto h2 = std::hash<std::int64_t>{}(s.age);
    const auto h3 = std::hash<double>{}(s.value);
    const auto h4 = std::hash<bool>{}(s.alpha);
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
  }
};

int main() {
  HyperLogLog<MyStruct> hll;

  hll.add({"meow", 10, 25.5, true});
  hll.add({"UwU", 42, 22.2, false});
  hll.add({"meow", 10, 25.5, true});
  hll.add({"meow", 10, -25.50000000001, false});

  std::cout << hll.count() << std::endl;
}