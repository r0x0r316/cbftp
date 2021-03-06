#include "changekeyscreen.h"

#include "../../globalcontext.h"
#include "../../settingsloadersaver.h"

#include "../ui.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"

ChangeKeyScreen::ChangeKeyScreen(Ui * ui) {
  this->ui = ui;
}

ChangeKeyScreen::~ChangeKeyScreen() {

}

void ChangeKeyScreen::initialize(unsigned int row, unsigned int col) {
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  mismatch = false;
  oldmismatch = false;
  tooshort = false;
  unsigned int y = 4;
  unsigned int x = 1;
  mso.clear();
  mso.addStringField(y++, x, "oldkey", "Old passphrase:", "", true);
  mso.addStringField(y++, x, "newkey", "New passphrase:", "", true);
  mso.addStringField(y++, x, "newkey2", "Verify new:", "", true);
  init(row, col);
}

void ChangeKeyScreen::redraw() {
  ui->erase();
  unsigned int y = 1;
  ui->printStr(y, 1, "Please verify with your old encryption key.");
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void ChangeKeyScreen::update() {
  if (mismatch || oldmismatch || tooshort) {
    redraw();
  }
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  std::string error = "                                                          ";
  if (tooshort) {
    error = "Failed: The passphrase must be at least " + std::to_string(SHORTESTKEY) + " characters long.";
  }
  else if (mismatch) {
    error = "Failed: The new keys did not match.";
  }
  else if (oldmismatch) {
    error = "Failed: the old key was not correct.";
  }
  ui->printStr(8, 1, error);

  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    ui->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool ChangeKeyScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->setLegend();
      ui->update();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      mso.goUp();
      ui->update();
      return true;
    case KEY_DOWN:
      mso.goDown();
      ui->update();
      return true;
    case 10:

      activation = mso.getElement(mso.getSelectionPointer())->activate();
      tooshort = false;
      mismatch = false;
      oldmismatch = false;
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      ui->setLegend();
      ui->update();
      return true;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
    case 'd':
      std::shared_ptr<MenuSelectOptionTextField> field1 = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement(0));
      std::shared_ptr<MenuSelectOptionTextField> field2 = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement(1));
      std::shared_ptr<MenuSelectOptionTextField> field3 = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement(2));
      std::string oldkey = field1->getData();
      std::string newkey = field2->getData();
      std::string newkey2 = field3->getData();
      field1->clear();
      field2->clear();
      field3->clear();
      if (newkey == newkey2) {
        if (newkey.length() >= SHORTESTKEY) {
          if (global->getSettingsLoaderSaver()->changeKey(oldkey, newkey)) {
            ui->returnToLast();
            return true;
          }
          else {
            oldmismatch = true;
          }
        }
        else {
          tooshort = true;
        }
      }
      else {
        mismatch = true;
      }
      ui->update();
      return true;
  }
  return false;
}

std::string ChangeKeyScreen::getLegendText() const {
  return currentlegendtext;
}

std::string ChangeKeyScreen::getInfoLabel() const {
  return "CHANGE ENCRYPTION KEY";
}
