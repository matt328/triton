#pragma once

namespace tr {

class IUploadSystem {
public:
  IUploadSystem() = default;
  virtual ~IUploadSystem() = default;

  IUploadSystem(const IUploadSystem&) = default;
  IUploadSystem(IUploadSystem&&) = delete;
  auto operator=(const IUploadSystem&) -> IUploadSystem& = default;
  auto operator=(IUploadSystem&&) -> IUploadSystem& = delete;
};

}
