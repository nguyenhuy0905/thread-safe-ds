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
#include "pool_alloc.hpp"
#include "arena_alloc.hpp"
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
    test_threads.at(i) = std::thread{[=]() mutable {
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
  tsds::ArenaAlloc<4096> test{}; // NOLINT(*magic-number*)
}
// NOLINTEND(*function-cognitive-complexity*)
