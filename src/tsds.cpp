/**
 * @file tsds.cpp
 * @brief Public-facing module of the project.
 * Doesn't really do anything other than:
 * - Tie other modules into a large one.
 * - Define document groups.
 * - And some other document-related stuff.
 */

module;
export module tsds;
export import tsds.pool_alloc;

/**
 * @page ref Reference page
 *
 * Below are all user-facing functionalities.
 *
 * @tableofcontents
 * @section alloc Allocators
 * - The thread-safe, (almost) lock-free allocators.
 * @subsection pool Pool allocator
 * - See @ref tsds::PoolAlloc for more info.
 *
 * @section ds Data structures
 */
