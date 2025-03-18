#pragma once
namespace tr {
using TracyContextPtr = std::unique_ptr<tracy::VkCtx, void (*)(tracy::VkCtx*)>;
}
