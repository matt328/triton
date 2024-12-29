#pragma once

struct Book {
  explicit Book(const std::string& name) : name(name) {
  }
  std::string name;
};

struct Shelf {
  explicit Shelf(const Book& newBook) : book(newBook) {
  }
  Book book;
};

struct Library {
  explicit Library(const Shelf& newShelf) : shelf(newShelf) {
  }
  Shelf shelf;
};
