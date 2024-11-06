// service.h
#pragma once
#include "Database.hpp"
#include "Logger.hpp"
#include "IService.hpp"

class Service : public IService {
 public:
   Service(Database& db, Logger& logger) : db_(db), logger_(logger) {
   }
   void doWork() {
      db_.connect("database_url");
      logger_.log("Work done!");
   }

 private:
   Database& db_;
   Logger& logger_;
};