/**
 * @file pool_alloc.hpp
 * @brief Contains definitions of @ref tsds::PoolAlloc.
 */

#ifndef TSDS_POOL_ALLOC_HPP
#define TSDS_POOL_ALLOC_HPP

#ifndef TSDS_MODULE
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>
#include <ranges>
#include <type_traits>
#endif // !TSDS_MODULE

#ifdef TSDS_MODULE
export namespace tsds {
#else
namespace tsds {
#endif // !TSDS_MODULE

/**
 * @class PoolAlloc
 * @brief A fix-sized pool allocator.
 * @tparam T The type to alloc.
 * @tparam NBlock The maximum number of T that can be held.
 * @tparam BuffInitAlloc The allocator used to allocate the buffer. Only used
 * once.
 * @important This class does NOT satisfies the named requirement
 * [Allocator](https://en.cppreference.com/w/cpp/named_req/Allocator).
 * @note In @c constexpr context, all allocations fall back to @c ::operator new
 * @important At the moment of writing, @c shared_ptr is @b not @c constexpr.
 * So, as much as we hate it, this allocator is not @c constexpr.
 *
 * Manages a buffer allocated by @e BuffInitAlloc.
 *
 * As for why that's the case: this allocator only allows you to get
 * one chunk of memory of size @c sizeof(T) each time. The named
 * requirement for @e Allocator requires @c allocate to be able to
 * take in an extra parameter @c n, and return a pointer to @c T[n].
 * This allocator simply doesn't do that.
 */
template <class T, std::size_t NBlock,
          template <typename> class BuffInitAlloc = std::allocator>
  requires std::is_same_v<T, std::remove_reference_t<T>>
class PoolAlloc {
public:
  constexpr PoolAlloc() noexcept(noexcept(BuffAllocType{}.allocate(NBlock))) =
      default;
  // NOLINTBEGIN(*identifier-naming*)
  /**
   * @name Type definitions
   * @nosubgrouping
   * Typedefs (that tries) following the @e Allocator named requirement.
   */
  /// @{
  using value_type = T;
  using pointer = std::add_pointer_t<T>;
  using const_pointer = std::add_const_t<pointer>;
  using void_pointer = void*;
  using const_void_pointer = std::add_const_t<void_pointer>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  static constexpr bool is_always_equal = false;
  template <typename Other> struct rebind {
    using other = PoolAlloc<Other, NBlock>;
  };
  /// @}
  // NOLINTEND(*identifier-naming*)

  /**
   * @brief Allocates a block of size @c sizeof(T).
   * @return A valid pointer to an uninitialized memory block of size @c
   * sizeof(T) if successful, or a nullptr otherwise.
   * @important This is @b not an allocator that conforms to Allocator named
   * requirement, simply because it doesn't need to. Refer to the document of
   * @ref PoolAlloc for a brief explanation.
   * @important At the moment of writing, @c shared_ptr is @b not @c constexpr.
   * So, as much as we hate it, this allocator is not @c constexpr.
   *
   * In @c constexpr context, simply fall back to @c ::operator new
   */
  constexpr auto allocate() noexcept -> pointer;

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
   * @important Please only let exactly @b one thread deallocate. Fortunately,
   * this is the behavior that smart pointers support.
   * @important At the moment of writing, @c shared_ptr is @b not @c constexpr.
   * So, as much as we hate it, this allocator is not @c constexpr.
   */
  constexpr void deallocate(pointer t_p_obj) noexcept;
  /**
   * @brief Just returns @a NBlock
   * @return NBlock.
   * This is just here as part of the @e Allocator named requirement. Despite
   * that, @ref PoolAlloc does @b not satisfy the named requirement.
   */
  [[nodiscard]] constexpr auto max_size() const noexcept -> size_type;
  /**
   * @brief Compares @a this pool allocator to @a t_other.
   * @param t_other The other allocator
   * @return @c true if both allocator instances operate on the same allocation
   * pool, false otherwise.
   *
   * If two @ref PoolAlloc are equal, calls to @ref allocate or @ref deallocate
   * of both instances operate on the same memory pool.
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
  using BuffAllocType = BuffInitAlloc<std::array<T, NBlock>>;
  /**
   * @class AllocSlot
   * @brief Linked list node, each holding a pointer to a block of size
   * @c sizeof(T) inside the buffer. There are @a NBlocks of AllocSlots in each
   * @ref AllocBuf.
   * @see tsds::PoolAlloc::AllocBuf
   */
  struct AllocSlot {
    constexpr AllocSlot() = default;
    /**
     * @brief Convenient initialization function for @ref AllocSlot
     *
     * @param t_p_next Initializes @ref next
     * @param t_p_blk Initializes @ref blk
     */
    constexpr AllocSlot(AllocSlot* t_p_next, pointer t_p_blk)
        : next(t_p_next), blk(t_p_blk) {}
    /// Pointer to the next @ref AllocSlot in the linked list.
    std::atomic<AllocSlot*> next;
    /// The block of memory this @ref AllocSlot controls.
    pointer blk;
  };

  /**
   * @class AllocBuf
   * @brief Manages the allocation buffer. Contains @a NBlock of @ref
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
};

/**
 * @private
 */
template <class T, std::size_t NBlock, template <typename> class BuffInitAlloc>
  requires std::is_same_v<T, std::remove_reference_t<T>>
class PoolAlloc<T, NBlock, BuffInitAlloc>::AllocBuf {
public:
  constexpr AllocBuf() noexcept(noexcept(BuffAllocType{}.allocate(NBlock)))
      : m_buff(m_buff_alloc.allocate(NBlock), m_del) {
    AllocSlot* last_slot = nullptr;
    // std::views::zip hasn't landed in libstdc++
    // for (std::tuple<AllocSlot&, std::add_lvalue_reference_t<T>> slot_block :
    //      std::views::zip(m_slot_list, *m_buff) | std::views::reverse) {
    //   auto& slot = std::get<0>(slot_block);
    //   slot.blk = &std::get<1>(slot_block);
    //   // slot.blk = m_buff->data() + idx;
    //   slot.next = last_slot;
    //   last_slot = &slot;
    // }

    // in case someone defines iterator to not be a pointer
    auto buff_iter = &(*m_buff->end());
    for (auto& slot : m_slot_list | std::views::reverse) {
      slot.blk = --buff_iter;
      slot.next = last_slot;
      last_slot = &slot;
    }
    m_head.store(m_slot_list.data(), std::memory_order::relaxed);

    m_init_flag.test_and_set(std::memory_order::release);
    m_init_flag.notify_all();
  }
  AllocBuf(const AllocBuf&) = delete;
  AllocBuf(AllocBuf&&) = delete;
  auto operator=(const AllocBuf&) = delete;
  auto operator=(AllocBuf&&) = delete;
  constexpr ~AllocBuf() = default;

  /**
   * @copydoc tsds::PoolAlloc::allocate
   */
  [[nodiscard]] auto allocate() noexcept -> pointer;
  /**
   * @copydoc tsds::PoolAlloc::deallocate
   */
  void deallocate(pointer t_p_obj) noexcept;

private:
  /**
   * @class BuffDeleter
   * @brief Custom deleter for @ref m_buff
   *
   */
  class BuffDeleter {
  public:
    explicit constexpr BuffDeleter(BuffAllocType& t_alloc) noexcept
        : m_alloc{t_alloc} {}
    BuffDeleter(const BuffDeleter&) = delete;
    BuffDeleter(BuffDeleter&&) = delete;
    auto operator=(const BuffDeleter&) = delete;
    auto operator=(BuffDeleter&&) = delete;
    ~BuffDeleter() = default;
    constexpr void operator()(std::array<T, NBlock>* t_p_ptr) noexcept(
        noexcept(m_alloc.deallocate(t_p_ptr, NBlock))) {
      m_alloc.deallocate(t_p_ptr, NBlock);
    }

  private:
    /**
     * @brief Reference to @tsds::PoolAlloc::AllocBuf::BuffInitAlloc
     *
     * I know the naming is confusing.
     */
    BuffAllocType& m_alloc;
  };
  /**
   * @brief Blocks all allocations until initialization is finished.
   *
   * We need to finish initializing the allocator before we use it.
   * So, we acquire the lock as soon as initialization starts.
   * That's why it needs to be placed as the first member.
   *
   * @ref allocate will need to check (but doesn't need to acquire) if the
   * flag is true.
   */
  std::atomic_flag m_init_flag = ATOMIC_FLAG_INIT;
  /**
   * @brief One-time allocator used for allocating the buffer @ref m_buff.
   */
  BuffAllocType m_buff_alloc{};
  /**
   * @brief The deleter for the buffer @ref m_buff.
   */
  [[no_unique_address]] BuffDeleter m_del{m_buff_alloc};
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
  std::unique_ptr<std::array<T, NBlock>, BuffDeleter&>
      m_buff; // NOLINT(*avoid-c-array*)
  /**
   * @brief The head of the slot list. Doesn't necessarily have to be the
   * slot with the lowest/highest memory position.
   */
  std::atomic<AllocSlot*> m_head{};
};

template <class T, std::size_t NBlock, template <typename> class BuffInitAlloc>
  requires std::is_same_v<T, std::remove_reference_t<T>>
[[nodiscard]] constexpr auto PoolAlloc<T, NBlock, BuffInitAlloc>::operator==(
    const PoolAlloc& t_other) const noexcept -> bool {
  return m_alloc_buf == t_other.m_alloc_buf;
}

template <class T, std::size_t NBlock, template <typename> class BuffInitAlloc>
  requires std::is_same_v<T, std::remove_reference_t<T>>
[[nodiscard]] constexpr auto PoolAlloc<T, NBlock, BuffInitAlloc>::operator!=(
    const PoolAlloc& t_other) const noexcept -> bool {
  return m_alloc_buf != t_other.m_alloc_buf;
}

template <class T, std::size_t NBlock, template <typename> class BuffInitAlloc>
  requires std::is_same_v<T, std::remove_reference_t<T>>
[[nodiscard]] constexpr auto
PoolAlloc<T, NBlock, BuffInitAlloc>::allocate() noexcept -> pointer {
  if (std::is_constant_evaluated()) {
    return static_cast<pointer>(::operator new(sizeof(T)));
  }
  return m_alloc_buf->allocate();
}

template <class T, std::size_t NBlock, template <typename> class BuffInitAlloc>
  requires std::is_same_v<T, std::remove_reference_t<T>>
constexpr void
PoolAlloc<T, NBlock, BuffInitAlloc>::deallocate(pointer t_p_obj) noexcept {
  if (std::is_constant_evaluated()) {
    return ::operator delete(t_p_obj);
  }
  m_alloc_buf->deallocate(t_p_obj);
}

template <class T, std::size_t NBlock, template <typename> class BuffInitAlloc>
  requires std::is_same_v<T, std::remove_reference_t<T>>
[[nodiscard]] auto
PoolAlloc<T, NBlock, BuffInitAlloc>::AllocBuf::allocate() noexcept -> pointer {
  // wait until initialization finishes.
  // We don't need to get the "spinlock" here.
  if (!m_init_flag.test()) {
    m_init_flag.wait(false, std::memory_order::relaxed);
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

template <class T, std::size_t NBlock, template <typename> class BuffInitAlloc>
  requires std::is_same_v<T, std::remove_reference_t<T>>
void PoolAlloc<T, NBlock, BuffInitAlloc>::AllocBuf::deallocate(
    pointer t_p_obj) noexcept {
  auto t_idx = static_cast<size_type>(t_p_obj - m_buff->data());
  auto* slot = &m_slot_list.at(t_idx);
  auto curr_head = m_head.load(std::memory_order::relaxed);
  while (!m_head.compare_exchange_weak(curr_head, slot,
                                       std::memory_order::release,
                                       std::memory_order::relaxed)) {
  }
  slot->next.store(curr_head, std::memory_order::release);
}
}

#endif // !TSDS_POOL_ALLOC_HPP
