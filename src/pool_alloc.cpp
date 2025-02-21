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
/// @cond
#include "tsds_export.h"
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <ranges>
#include <type_traits>
/// @endcond
export module tsds.pool_alloc;

namespace tsds {

/**
 * @class PoolAlloc
 * @brief A fix-sized pool allocator.
 * @tparam T The type to alloc.
 * @tparam NBlock The maximum number of T that can be held.
 * @warning This allocator's allocations are @b not constexpr. For that purpose,
 * just use the "normal" global-state @c ::operator new()
 *
 * This class does NOT satisfies the named requirement
 * [Allocator](https://en.cppreference.com/w/cpp/named_req/Allocator).
 *
 * As for why that's the case: this allocator only allows you to get
 * one chunk of memory of size @c sizeof(T) each time. The named
 * requirement for @e Allocator requires @c allocate to be able to
 * take in an extra parameter @c n, and return a pointer to @c T[n].
 * This allocator simply doesn't do that.
 */
export TSDS_EXPORT template <class T, std::size_t NBlock>
  requires std::is_same_v<T, std::remove_reference_t<T>>
class PoolAlloc {
  // https://en.cppreference.com/w/cpp/named_req/Allocator
public:
  // NOLINTBEGIN(*identifier-naming*)
  /// @{
  using value_type = T;
  using pointer = std::add_pointer_t<T>;
  using const_pointer = std::add_const_t<pointer>;
  using void_pointer = void*;
  using const_void_pointer = const void_pointer;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  static constexpr bool is_always_equal = false;
  /// @}
  /**
   * @brief Part of Allocator named requirement (that is actually useful for
   * this allocator, although, not that much).
   * @tparam Other The type to rebind to.
   *
   * Could be useful if a data structure requires 2 different ways to
   * chunk-allocate. If that's the case, prefer to just store the extra
   * allocator as a variable.
   */
  template <typename Other> struct rebind {
    /// @cond
    using other = PoolAlloc<Other, NBlock>;
    /// @endcond
  };
  // NOLINTEND(*identifier-naming*)

  /**
   * @brief Allocates a block of size @c sizeof(T).
   * @return A valid pointer to an uninitialized memory block of size @c
   * sizeof(T) if successful, or a nullptr otherwise.
   *
   * @warning This is @b not an allocator that conforms to Allocator named
   * requirement, simply because it doesn't need to. Refer to the document of
   * @ref tsds::PoolAlloc for a brief explanation.
   */
  auto allocate() noexcept -> pointer;

  /**
   * @brief Releases the memory held by @a t_p_obj back.
   * @param t_p_obj The memory. Must be allocated by the @c this allocator, or
   * any allocator that is equal to @c this allocator instance. Can be @c
   * nullptr, at which point, the function does nothing. After the deallocation,
   * the pointer becomes invalidated and is set to @c nullptr.
   * @invariant @a t_p_obj must have been an allocation by @c this allocator or
   * an allocator @c alloc such that:
   * @code{.cpp}
   * *this == alloc;
   * @endcode
   * @warning Please only let exactly @b one thread deallocate. Fortunately,
   * this is the behavior that smart pointers support.
   */
  void deallocate(pointer t_p_obj) noexcept;
  /**
   * @brief Just returns @a NBlock
   * @return NBlock.
   * This is just here as part of the @e Allocator named requirement. Despite
   * that, @ref tsds::PoolAlloc does @b not satisfy the named requirement.
   */
  [[nodiscard]] constexpr auto max_size() const noexcept -> size_type;
  /**
   * @brief Compares this pool allocator to another.
   * @param t_other The other allocator
   * @return true if both allocator instances operate on the same allocation
   * pool, false otherwise.
   */
  [[nodiscard]] constexpr auto
  operator==(const PoolAlloc& t_other) const noexcept -> bool;
  /**
   * @brief Basically the opposite of @ref operator==
   * @return The opposite of @ref operator==
   */
  [[nodiscard]] constexpr auto
  operator!=(const PoolAlloc& t_other) const noexcept -> bool;

private:
  /// @cond DEV
  /**
   * @class AllocSlot
   * @brief Linked list node, each holding a pointer to a block of size
   * sizeof(T) inside the buffer. There are NBlocks of AllocSlots in each @ref
   * AllocBuf.
   * @see tsds::PoolAlloc::AllocBuf
   */
  struct AllocSlot {
    AllocSlot() = default;
    /**
     * @brief Convenient initialization function for @ref AllocSlot
     *
     * @param t_p_next Initializes @ref next
     * @param t_p_blk Initializes @ref blk
     */
    AllocSlot(AllocSlot* t_p_next, pointer t_p_blk)
        : next(t_p_next), blk(t_p_blk) {}
    /// Pointer to the next @ref AllocSlot in the linked list.
    std::atomic<AllocSlot*> next;
    /// The block of memory this @ref AllocSlot controls.
    pointer blk;
  };

  /**
   * @class AllocBuf
   * @brief Manages the allocation buffer. Contains @a NBlocks of @ref
   * AllocSlot.
   * @see tsds::PoolAlloc::AllocSlot
   */
  class AllocBuf;

  /**
   * @brief Points to a block of allocation buffer.
   * Being a shared pointer, this allows easy (but NOT trivial) copying. And,
   * it's kind of the only way we can satisfy the named requirement of
   * Allocator.
   */
  std::shared_ptr<AllocBuf> m_alloc_buf{std::make_shared<AllocBuf>()};

  static_assert(!std::is_trivially_copy_constructible_v<decltype(m_alloc_buf)>);
  static_assert(!std::is_trivially_copy_assignable_v<decltype(m_alloc_buf)>);
  static_assert(!std::is_trivially_move_constructible_v<decltype(m_alloc_buf)>);
  static_assert(!std::is_trivially_move_assignable_v<decltype(m_alloc_buf)>);
  /// @endcond DEV
};

/// @cond
template <class T, std::size_t NBlock>
  requires std::is_same_v<T, std::remove_reference_t<T>>
class PoolAlloc<T, NBlock>::AllocBuf {
public:
  AllocBuf() noexcept
      : m_init_flag{true}, m_buff{reinterpret_cast<T*>(           // NOLINT
                               std::malloc(sizeof(T) * NBlock))}, // NOLINT
        m_head{m_slot_list.begin()} {
    AllocSlot* last_slot = nullptr;
    std::size_t idx = 0;
    for (auto& slot : m_slot_list | std::views::reverse) {
      slot.blk = m_buff.get() + idx;
      slot.next = last_slot;
      last_slot = &slot;
      ++idx;
    }

    m_init_flag.clear(std::memory_order::release);
    m_init_flag.notify_all();
  }
  AllocBuf(const AllocBuf&) = delete;
  AllocBuf(AllocBuf&&) = delete;
  auto operator=(const AllocBuf&) = delete;
  auto operator=(AllocBuf&&) = delete;
  ~AllocBuf() = default;

  /**
   * @copydoc tsds::PoolAlloc::allocate
   */
  [[nodiscard]] auto allocate() noexcept -> pointer;
  /**
   * @copydoc tsds::PoolAlloc::deallocate
   */
  void deallocate(pointer t_p_obj) noexcept;

private:
  struct BufferDeallocator {
    void operator()(pointer t_p_buff) noexcept {
      std::free(t_p_buff); // NOLINT(*no-malloc*)
    }
  };
  /**
   * @brief Blocks all allocations until initialization is finished.
   *
   * We need to finish initializing the allocator before we use it.
   * So, we acquire the lock as soon as initialization starts.
   * That's why it needs to be placed as the first member.
   *
   * @ref allocate will need to check (but doesn't need to acquire) if the
   * flag is false.
   */
  std::atomic_flag m_init_flag;
  /**
   * @brief Holds instances @ref AllocSlot.
   *
   * @ref AllocSlot themselves form a linked list. This array simply holds the
   * data.
   */
  std::array<AllocSlot, NBlock> m_slot_list{};
  /**
   * @brief The buffer.
   */
  const std::unique_ptr<T, BufferDeallocator> m_buff; // NOLINT(*avoid-c-array*)
  /**
   * @brief The head of the slot list. Doesn't necessarily have to be the
   * slot with the lowest/highest memory position.
   */
  std::atomic<AllocSlot*> m_head;
};

template <class T, std::size_t NBlock>
  requires std::is_same_v<T, std::remove_reference_t<T>>
[[nodiscard]] constexpr auto
PoolAlloc<T, NBlock>::operator==(const PoolAlloc& t_other) const noexcept
    -> bool {
  return m_alloc_buf == t_other.m_alloc_buf;
}

template <class T, std::size_t NBlock>
  requires std::is_same_v<T, std::remove_reference_t<T>>
[[nodiscard]] constexpr auto
PoolAlloc<T, NBlock>::operator!=(const PoolAlloc& t_other) const noexcept
    -> bool {
  return m_alloc_buf != t_other.m_alloc_buf;
}

template <class T, std::size_t NBlock>
  requires std::is_same_v<T, std::remove_reference_t<T>>
[[nodiscard]] auto PoolAlloc<T, NBlock>::allocate() noexcept -> pointer {
  return m_alloc_buf->allocate();
}

template <class T, std::size_t NBlock>
  requires std::is_same_v<T, std::remove_reference_t<T>>
void PoolAlloc<T, NBlock>::deallocate(pointer t_p_obj) noexcept {
  m_alloc_buf->deallocate(t_p_obj);
}

template <class T, std::size_t NBlock>
  requires std::is_same_v<T, std::remove_reference_t<T>>
[[nodiscard]] auto PoolAlloc<T, NBlock>::AllocBuf::allocate() noexcept
    -> pointer {
  // wait until initialization finishes.
  // We don't need to get the "spinlock" here.
  if (m_init_flag.test()) {
    m_init_flag.wait(true, std::memory_order::relaxed);
  }

  auto curr_head = m_head.load(std::memory_order::relaxed);
  if (curr_head == nullptr) {
    return nullptr;
  }
  // gain an exclusive block.
  while (!m_head.compare_exchange_weak(curr_head, curr_head->next,
                                       std::memory_order::acquire,
                                       std::memory_order::relaxed)) {
    // no more memory in this allocator.
    if (curr_head == nullptr) {
      return nullptr;
    }
  }
  // now curr_head is an exclusive block.
  return curr_head->blk;
}

template <class T, std::size_t NBlock>
  requires std::is_same_v<T, std::remove_reference_t<T>>
void PoolAlloc<T, NBlock>::AllocBuf::deallocate(pointer t_p_obj) noexcept {
  auto t_idx = static_cast<size_type>(t_p_obj - m_buff.get());
  auto* slot = &m_slot_list.at(t_idx);
  auto curr_head = m_head.load(std::memory_order::relaxed);
  while (!m_head.compare_exchange_weak(curr_head, slot,
                                       std::memory_order::release,
                                       std::memory_order::relaxed)) {
  }
  slot->next.store(curr_head, std::memory_order::release);
}

/// @endcond DEV
} // namespace tsds
