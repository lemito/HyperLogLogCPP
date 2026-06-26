#include <iostream>

#include "hyperll.hpp"

int main() {
  HyperLogLog<int64_t> hll;

  hll.add(0);
  hll.add(1);
  hll.add(2);
  hll.add(8);
  hll.add(INT64_MAX);

  std::cout << hll.count();

  HyperLogLog<int64_t> hl;

  hl.add(5);
  hl.add(6);
  hl.add(7);
  hl.add(9);
  hl.add(64);

  std::cout << hl.count();

  HyperLogLog<int64_t> cpy = hll;
  cpy.merge(hl);

  std::cout << cpy.count();

  return 0;
}
