#include <catch2/catch_test_macros.hpp>
// without the range, clangd complains. So, comment the include out when clangd
// complains
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <thread>
#ifdef TSDS_MODULE
import tsds.pool_alloc;
import tsds.arena_alloc;
#else
#include "arena_alloc.hpp"
#include "pool_alloc.hpp"
#endif // TSDS_MODULE

// NOTE: Catch2 assertions are not thread-safe.
// So, for now, I retreat to asserts.
// Maybe I will switch to GTest for this very reason.

// NOLINTBEGIN(*function-cognitive-complexity*)
TEST_CASE("Hopefully it compiles", "[pool_alloc]") {
  constexpr std::size_t POOL_NUM = 1024;
  tsds::PoolAlloc<int, POOL_NUM> test{};
  auto* first_ptr = test.allocate();
  REQUIRE(first_ptr != nullptr);
  test.deallocate(first_ptr);

  // there's a very high chance you're borked by TSan if you do allocate
  // dynamically (say, with a vector)
  std::array<std::thread, 32> test_threads{}; // NOLINT(*magic-number*)
  for (uint16_t i = 0; i < 32; ++i) {         // NOLINT(*magic-number*)
    test_threads.at(i) = std::thread{[&]() mutable {
      std::array<int*, 32> ptr_vec{};    // NOLINT(*magic-number*)
      for (uint8_t j = 0; j < 32; ++j) { // NOLINT(*magic-number*)
        auto* ptr = test.allocate();
        // should be consistent all the way until deallocation.
        *ptr = j; // NOLINT
        // Link with TSan to check.
        assert(ptr != nullptr);
        // if shits go wrong, this goes wrong
        assert(*ptr == j);
        ptr_vec.at(j) = ptr;
      }
      // If nothing goes wrong, commenting this out should still pass all the
      // tests.
      // Try to test what happens if no deallocate called also. Technically,
      // nothing should go wrong.
      for (uint8_t j = 0; j < 32; ++j) { // NOLINT(*magic-number*)
        assert(*ptr_vec.at(j) == j);
        test.deallocate(ptr_vec.at(j));
      }
    }};
    for (auto& thr : test_threads) {
      if (thr.joinable()) {
        thr.join();
      }
    }
  }
}

TEST_CASE("Hopefully it compiles", "[arena_alloc]") {
  tsds::ArenaAlloc<4096> test{};             // NOLINT(*magic-number*)
  std::array<std::thread, 8> test_threads{}; // NOLINT(*magic-number*)
  for (uint8_t i = 0; i < 8; ++i) {          // NOLINT(*magic-number*)
    test_threads.at(i) = std::thread{[&]() mutable {
      std::array<std::tuple<long*, int*, char*>, 16> // NOLINT(*magic-number*)
          arr{};
      for (uint8_t j = 0; j < 16; ++j) { // NOLINT(*magic-number*)
        // the worst case is
        // char then int then long requested, due to alignment.
        // Which, if requested in that order, takes a total of 16 bytes,
        // while technically only 13 is needed.
        // With other contending threads, the worst case is something like:
        // a char
        // then two ints, so alignment is now divisible by 4 but potentially NOT
        // by 8
        // then a long, which needs 4 extra padding bytes.
        auto* chr = static_cast<char*>(
            test.allocate({.size = sizeof(char), .align = alignof(char)}));
        auto* num = static_cast<int*>(
            test.allocate({.size = sizeof(int), .align = alignof(int)}));
        auto* lnum = static_cast<long*>(
            test.allocate({.size = sizeof(long), .align = alignof(long)}));
        assert(lnum != nullptr);
        assert(num != nullptr);
        assert(chr != nullptr);
        *lnum = j + 97; // NOLINT(*magic-number*)
        *num = 4 + j;
        *chr = 'c';
        assert(*chr == 'c');
        assert(*num == 4 + j);
        assert(*lnum == 97 + j);
        arr.at(j) = std::make_tuple(lnum, num, chr);
      }

      for (uint8_t j = 0; j < 16; ++j) { // NOLINT(*magic-number*)
        auto& [lnum, num, chr] = arr.at(j);
        assert(*chr == 'c');
        assert(*num == 4 + j);
        assert(*lnum == 97 + j);
      }
    }};
  }
  for (auto& thr : test_threads) {
    if (thr.joinable()) {
      thr.join();
    }
  }
}
// NOLINTEND(*function-cognitive-complexity*)
