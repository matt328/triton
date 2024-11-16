#pragma once
namespace tr::cm {
   using TracyContextPtr = std::unique_ptr<tracy::VkCtx, void (*)(tracy::VkCtx*)>;
}