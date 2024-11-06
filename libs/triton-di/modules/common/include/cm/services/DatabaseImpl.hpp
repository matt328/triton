// database_impl.h
#pragma once
#include "Database.hpp"
#include <iostream>

class DatabaseImpl : public Database {
 public:
   void connect(const std::string& connectionString) override {
      std::cout << "Connecting to " << connectionString << std::endl;
   }
};