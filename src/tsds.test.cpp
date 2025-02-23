#include <catch2/catch_test_macros.hpp>
// without the range, clangd complains. So, comment the include out when clangd
// complains
#include <array>
#include <cstddef>
#include <cstdint>
#include <thread>
#ifdef TSDS_MODULE
import tsds.pool_alloc;
#else
#include "pool_alloc.hpp"
#endif // TSDS_MODULE

// NOLINTBEGIN(*function-cognitive-complexity*)
TEST_CASE("Hopefully it compiles") {
  constexpr std::size_t POOL_NUM = 1024;
  tsds::PoolAlloc<int, POOL_NUM> test{};
  auto* first_ptr = test.allocate();
  REQUIRE(first_ptr != nullptr);
  test.deallocate(first_ptr);

  // there's a very high chance you're borked by TSan if you do allocate
  // dynamically (say, with a vector)
  std::array<std::thread, 32> test_threads{}; // NOLINT(*magic-number*)
  for (uint16_t i = 0; i < 32; ++i) {         // NOLINT(*magic-number*)
    test_threads.at(i) = std::thread{[&test]() mutable {
      std::array<int*, 32> ptr_vec{};    // NOLINT(*magic-number*)
      for (uint8_t j = 0; j < 32; ++j) { // NOLINT(*magic-number*)
        auto* ptr = test.allocate();
        // should be consistent all the way until deallocation.
        *ptr = j; // NOLINT
        // Link with TSan to check.
        REQUIRE(ptr != nullptr);
        // if shits go wrong, this goes wrong
        REQUIRE(*ptr == j);
        ptr_vec.at(j) = ptr;
      }
      // If nothing goes wrong, commenting this out should still pass all the
      // tests
      for (uint8_t j = 0; j < 32; ++j) { // NOLINT(*magic-number*)
        REQUIRE(*ptr_vec.at(j) == j);
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
// NOLINTEND(*function-cognitive-complexity*)
