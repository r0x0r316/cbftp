#pragma once

#include "../../proxy.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;

class EditProxyScreen : public UIWindow {
public:
  EditProxyScreen(Ui *);
  ~EditProxyScreen();
  void initialize(unsigned int, unsigned int, std::string, std::string);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  Proxy * proxy;
  Proxy modproxy;
  std::shared_ptr<MenuSelectOptionTextArrow> authmethod;
  int latestauthmethod;
  std::string operation;
};
