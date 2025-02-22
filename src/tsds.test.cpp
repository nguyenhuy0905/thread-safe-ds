#include <catch2/catch_test_macros.hpp>
// without the range, clangd complains. So, comment the include out when clangd
// complains
#include <array>
#include <cstddef>
#include <cstdint>
#include <thread>
import tsds;

TEST_CASE("Placebo") { REQUIRE(1 == 1); }

// NOLINTBEGIN(*function-cognitive-complexity*)
TEST_CASE("Hopefully it compiles") {
  constexpr std::size_t POOL_NUM = 1024;
  tsds::PoolAlloc<int, POOL_NUM> test{};
  auto* ptr = test.allocate();
  REQUIRE(ptr != nullptr);
  test.deallocate(ptr);

  // there's a very high chance you're borked by TSan if you do allocate
  // dynamically (say, with a vector)
  std::array<std::thread, 32> test_threads{}; // NOLINT(*magic-number*)
  for (uint16_t i = 0; i < 32; ++i) {         // NOLINT(*magic-number*)
    test_threads.at(i) = std::thread{[&test]() mutable {
      std::array<int*, 32> ptr_vec{};    // NOLINT(*magic-number*)
      for (uint8_t i = 0; i < 32; ++i) { // NOLINT(*magic-number*)
        auto* ptr = test.allocate();
        // should be consistent all the way until deallocation.
        *ptr = i; // NOLINT
        // Link with TSan to check.
        REQUIRE(ptr != nullptr);
        // if shits go wrong, this goes wrong
        REQUIRE(*ptr == i);
        ptr_vec.at(i) = ptr;
      }
      // If nothing goes wrong, commenting this out should still pass all the
      // tests
      for (uint8_t i = 0; i < 32; ++i) { // NOLINT(*magic-number*)
        REQUIRE(*ptr_vec.at(i) == i);
        test.deallocate(ptr_vec.at(i));
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
