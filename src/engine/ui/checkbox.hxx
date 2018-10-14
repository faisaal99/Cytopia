#ifndef CHECKBOX_HXX_
#define CHECKBOX_HXX_

#include "uiElement.hxx"

class Checkbox : public UiElement
{
public:
  Checkbox(const SDL_Rect &uiElementRect);
  ~Checkbox() override = default;

  void draw() override;

  void onMouseButtonUp(const SDL_Event &event) override;
  void onMouseButtonDown(const SDL_Event &event) override;
  void onMouseEnter(const SDL_Event &event) override;
  void onMouseLeave(const SDL_Event &event) override;

private:
  SDL_Rect _rect;

  bool _isMouseButtonDown = false;
  Signal::Signal<void()> clickSignal;
  Signal::Signal<void(const std::string &)> toggleGroupSignal;
};

#endif