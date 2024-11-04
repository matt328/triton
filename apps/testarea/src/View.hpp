#pragma once

struct Renderer;

class IView {
 public:
   IView(const IView&) = default;
   IView(IView&&) = delete;
   auto operator=(const IView&) -> IView& = default;
   auto operator=(IView&&) -> IView& = delete;

   IView() = default;
   virtual ~IView() noexcept = default;

   virtual void update() = 0;
};

class TextView : public IView {
 public:
   explicit TextView(const Renderer& renderer) {
      std::cout << "Created TextView" << '\n';
   }

   void update() override {
      std::cout << "Updating Text View" << '\n';
   }
};

class GuiView : public IView {
 public:
   GuiView(const std::string& title, const Renderer& renderer) : title{title} {
      std::cout << "Created gui view with title: " << title << '\n';
   }

   void update() override {
      std::cout << "Updating Gui View" << '\n';
   }

 private:
   std::string title;
};