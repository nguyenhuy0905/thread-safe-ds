#ifndef TSDS_ARENA_ALLOC_HPP
#define TSDS_ARENA_ALLOC_HPP

#ifndef TSDS_MODULE
#include <atomic>
#include <cstddef>
#include <memory>
#endif // !TSDS_MODULE

#ifdef TSDS_MODULE
export namespace tsds {
#else
namespace tsds {
#endif // !TSDS_MODULE
/**
 * @class ArenaAlloc
 * @brief Bulk-deallocate memory allocator. Work in progress.
 * @tparam Size The size in bytes of the buffer managed by @a this allocator.
 * @tparam BuffInitAlloc The allocator used to allocate the buffer. Only used
 * once.
 */
template <std::size_t Size,
          template <typename> typename BuffInitAlloc = std::allocator>
class ArenaAlloc {
public:
private:
  /**
   * @class AllocBuf
   * @brief Holds the memory block used by some instances of @ref ArenaAlloc
   */
  class AllocBuf {
  public:
  private:
  };
  std::shared_ptr<AllocBuf> m_buf;
  std::atomic<uint8_t> m_head;
};
}
#endif // !TSDS_ARENA_ALLOC_HPP
