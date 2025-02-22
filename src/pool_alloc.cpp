/**
 * @file pool_alloc.cpp
 * @brief Contains definitions of @ref tsds::PoolAlloc.
 */

/**
 * @module tsds.pool_alloc
 * @brief Defines a fixed-size, thread-safe chunk allocator.
 * @see tsds::PoolAlloc
 */

module;
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <ranges>
#include <type_traits>
export module tsds_pool_alloc;

#include "pool_alloc.hpp"
