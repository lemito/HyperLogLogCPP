#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <utility>

#define IS_DEBUG 1

#ifdef IS_DEBUG
#include <iostream>
#define DEBUG(...)                                                             \
  do {                                                                         \
    std::cerr << __VA_ARGS__ << std::endl;                                     \
  } while (0)
#else
#define DEBUG(...)                                                             \
  do {                                                                         \
  } while (0)
#endif

/**
 *
 * @tparam T class for store
 * @tparam Hash Hash functor (default std::hash<T>)
 */
template <typename T, typename Hash = std::hash<T>> class HyperLogLog {
  std::byte *_registers{nullptr};
  size_t _size{0};
  Hash _hash{};
  uint8_t _precision{0};

  [[nodiscard]] constexpr double alpha() const {
    if (_size == 16) {
      return 0.673;
    }
    if (_size == 32) {
      return 0.697;
    }
    if (_size == 64) {
      return 0.709;
    }
    return 0.7213 / (1.0 + 1.079 / static_cast<double>(_size));
  }

  static constexpr std::byte rank(uint64_t w) {
    uint8_t r = 1;
    while ((w & 0x8000000000000000ULL) == 0 && r < 64) {
      w <<= 1;
      ++r;
    }
    return static_cast<std::byte>(r);
  }

  void _add(const uint64_t hash) const noexcept {
    const size_t ix = hash & (_size - 1);
    const uint64_t w = hash << _precision;
    const std::byte r =
        w == 0 ? static_cast<std::byte>(64 - _precision + 1) : rank(w);
    if (r > _registers[ix]) {
      _registers[ix] = r;
    }
    DEBUG(hash << ' ' << ix << ' ' << static_cast<int32_t>(r) << '\n');
  }

  [[nodiscard]] uint64_t _count() const {
    double sum = 0.0;

    for (size_t i = 0; i < _size; i++) {
      sum += std::exp2(-static_cast<double>(_registers[i]));
    }

    constexpr double kHashSpace = (static_cast<double>(1ULL << 63) * 2.0);
    double E =
        alpha() * static_cast<double>(_size) * static_cast<double>(_size) / sum;

    if (E <= 2.5 * _size) {
      size_t zero_count = 0;
      for (size_t i = 0; i < _size; ++i)
        if (_registers[i] == static_cast<std::byte>(0))
          ++zero_count;
      if (zero_count > 0)
        E = static_cast<double>(_size) *
            std::log(static_cast<double>(_size) / zero_count);
    } else if (E > kHashSpace / 30.0) {
      E = -kHashSpace * std::log(1.0 - E / kHashSpace);
    }

    return static_cast<uint64_t>(E + 0.5);
  }

public:
  explicit HyperLogLog(const uint8_t precision = 8)
      : _registers(new std::byte[1ULL << precision]), _size(1ULL << precision),
        _precision(precision) {
    if (precision < 4 || precision > 16) {
      throw std::invalid_argument("precision must be 4..16");
    }
    std::memset(_registers, 0, _size);
  }

  HyperLogLog(const HyperLogLog &other)
      : _registers(new std::byte[other._size]), _size(other._size),
        _hash(other._hash), _precision(other._precision) {
    std::memcpy(_registers, other._registers, other._size);
  }

  HyperLogLog &operator=(const HyperLogLog &other) {
    if (this != &other) {
      delete[] _registers;
      _size = other._size;
      _precision = other._precision;
      _hash = other._hash;
      _registers = new std::byte[_size];
      std::memcpy(_registers, other._registers, _size);
    }
    return *this;
  }

  HyperLogLog(HyperLogLog &&other) noexcept
      : _registers(other._registers), _size(other._size),
        _hash(other._hash), _precision(other._precision) {
    other._registers = nullptr;
    other._size = 0;
    other._precision = 0;
  }

  HyperLogLog &operator=(HyperLogLog &&other) noexcept {
    if (this != &other) {
      delete[] _registers;

      _precision = std::exchange(other._precision, 8);
      _size = std::exchange(other._size, 0);
      _registers = std::exchange(other._registers, nullptr);
      _hash = std::exchange(other._hash, {});
    }
    return *this;
  }

  void add(const T &elem) noexcept {
    const auto hash = _hash(elem);
    _add(hash);
  }

  void merge(const HyperLogLog &other) noexcept {
    if (_precision != other._precision) {
      throw std::invalid_argument("precision mismatch");
    }
    if (_size != other._size) {
      throw std::invalid_argument("size mismatch");
    }

    for (size_t i = 0; i < other._size; ++i) {
      _registers[i] = std::max(_registers[i], other._registers[i]);
    }
  }

  [[nodiscard]] uint64_t count() const noexcept { return _count(); }

  ~HyperLogLog() { delete[] _registers; }

  void clear() const { std::memset(_registers, 0, _size); }

  [[nodiscard]] size_t size() const { return _size; }
};
