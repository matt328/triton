// service.h
#pragma once
#include "Database.hpp"
#include "Logger.hpp"
#include "IService.hpp"
#include "IThing.hpp"

class Service : public IService {
 public:
   Service(Database& db, Logger& logger, IThing& thing) : db_(db), logger_(logger), thing_(thing) {
   }
   void doWork() {
      thing_.sayHello();
      db_.connect("database_url");
      logger_.log("Work done!");
   }

 private:
   Database& db_;
   Logger& logger_;
   IThing& thing_;
};