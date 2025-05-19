#pragma once

namespace tr {

class IUploadQueue {
public:
  IUploadQueue() = default;
  virtual ~IUploadQueue() = default;

  IUploadQueue(const IUploadQueue&) = default;
  IUploadQueue(IUploadQueue&&) = delete;
  auto operator=(const IUploadQueue&) -> IUploadQueue& = default;
  auto operator=(IUploadQueue&&) -> IUploadQueue& = delete;
};

}
