#pragma once

struct Database {
   virtual void connect(const std::string& connectionString) = 0;
   virtual ~Database() = default;
};