#pragma once

#if defined(TRACY_ENABLE)

auto operator new(const std::size_t count) -> void* {
   const auto ptr = malloc(count);
   TracyAllocS(ptr, count, 32);
   return ptr;
}
void operator delete(void* ptr) noexcept {
   TracyFreeS(ptr, 32);
   free(ptr);
}
#endif
