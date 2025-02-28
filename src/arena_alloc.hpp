#ifndef TSDS_ARENA_ALLOC_HPP
#define TSDS_ARENA_ALLOC_HPP

#include <cstdint>
#ifndef TSDS_MODULE
#include <atomic>
#include <cstddef>
#include <memory>
#include <type_traits>
#endif // !TSDS_MODULE

#ifdef TSDS_MODULE
export namespace tsds {
#else
namespace tsds {
#endif // !TSDS_MODULE

template <std::size_t Size>
concept ValidSize = requires { Size > 0; };

/**
 * @class ArenaAlloc
 * @brief Your everyday arena allocator, with some thread-safety.
 * @tparam Size The byte size of the buffer.
 * @tparam BuffInitAlloc The allocator. It will be ``rebind``ed anyways, so
 * the template parameter type doesn't really matter.
 */
template <std::size_t Size,
          template <typename> typename BuffInitAlloc = std::allocator>
  requires ValidSize<Size>
class ArenaAlloc {
public:
  ArenaAlloc() noexcept(noexcept(BuffAllocType{}.allocate(1))) = default;
  /**
   * @class AllocInfo
   * @brief It's nicer than having 2 similar-looking arguments in the function
   * call I suppose?
   * @param size Must be larger than 0.
   * @param align Must be a power of 2, and smaller or equal to @p size.
   */
  struct AllocInfo {
    std::size_t size;
    std::uintptr_t align;
  };
  // if you wonder why I would pass everything by value with this.
  static_assert(std::is_trivial_v<AllocInfo>);
  /**
   * @brief Allocates at least @p t_size bytes at alignment @p t_align.
   *
   * @return Pointer to the newly allocated memory block.
   */
  auto allocate(AllocInfo t_alloc_info) noexcept -> void*;
  /**
   * @brief Does nothing at all.
   */
  auto deallocate(void* /*unused*/) {}

private:
  class AllocBuff;
  using BuffAllocType = BuffInitAlloc<AllocBuff>;
  class AllocWrapper {
  public:
    explicit AllocWrapper(BuffAllocType&& t_alloc) noexcept(
        noexcept(m_alloc.allocate(1)))
        : m_alloc(std::move(t_alloc)) {}
    auto allocate() -> AllocBuff* { return m_alloc.allocate(1); }
    auto operator()(AllocBuff* t_p_buff) { m_alloc.deallocate(t_p_buff, 1); }

  private:
    BuffAllocType m_alloc;
  };
  std::shared_ptr<AllocBuff> m_alloc_buff{[]() {
    auto alloc = AllocWrapper{BuffAllocType{}};
    auto* buff = alloc.allocate();
    return std::shared_ptr<AllocBuff>(new (buff) AllocBuff, alloc);
  }()};
};

template <std::size_t Size, template <typename> typename BuffInitAlloc>
  requires ValidSize<Size>
class ArenaAlloc<Size, BuffInitAlloc>::AllocBuff {
public:
  AllocBuff() = default;
  auto allocate(AllocInfo t_alloc_info) -> void* {
    auto align_index = [&](std::size_t t_idx) -> std::size_t {
      // similar to curr_head_num % curr_head_idx since we assume align is a
      // power of 2
      auto* head_ptr = &m_buff.front() + t_idx;
      // NOLINTNEXTLINE(*reinterpret-cast*)
      auto curr_head_num = reinterpret_cast<std::uintptr_t>(head_ptr);
      auto mod = curr_head_num & (t_alloc_info.align - 1);
      return (mod == 0)
                 ? t_idx
                 : static_cast<std::size_t>(t_idx + t_alloc_info.align - mod);
    };

    auto curr_head_idx = m_head_idx.load(std::memory_order::relaxed);
    auto aligned_idx = align_index(curr_head_idx);
    auto next_head_idx =
        static_cast<std::size_t>(aligned_idx + t_alloc_info.size);
    if (next_head_idx >= Size) {
      return nullptr;
    }

    while (!m_head_idx.compare_exchange_weak(curr_head_idx, next_head_idx,
                                             std::memory_order::release,
                                             std::memory_order::acquire)) {
      aligned_idx = align_index(curr_head_idx);
      next_head_idx = static_cast<std::size_t>(aligned_idx + t_alloc_info.size);
      if (next_head_idx >= Size) {
        return nullptr;
      }
    }
    return static_cast<void*>(&m_buff.front() + aligned_idx);
  }

private:
  std::array<uint8_t, Size> m_buff{};
  std::atomic<decltype(Size)> m_head_idx{};
};

template <std::size_t Size, template <typename> typename BuffInitAlloc>
  requires ValidSize<Size>
auto ArenaAlloc<Size, BuffInitAlloc>::allocate(AllocInfo t_alloc_info) noexcept
    -> void* {
  return m_alloc_buff->allocate(t_alloc_info);
}
}
#endif // !TSDS_ARENA_ALLOC_HPP
