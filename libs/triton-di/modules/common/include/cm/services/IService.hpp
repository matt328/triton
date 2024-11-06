// iservice.h
#pragma once

class IService {
 public:
   virtual ~IService() = default;
   virtual void doWork() = 0;
};