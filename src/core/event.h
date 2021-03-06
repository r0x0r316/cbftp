#pragma once

#include <memory>
#include <string>

class EventReceiver;

class Event {
private:
  EventReceiver * receiver;
  std::shared_ptr<EventReceiver> preceiver;
  int type;
  void * data;
  int datalen;
  int numdata;
  int numdata2;
  std::string strdata;
public:
  Event();
  Event(EventReceiver *, int, int, void *, int);
  Event(EventReceiver *, int, int, void *);
  Event(EventReceiver *, int);
  Event(EventReceiver *, int, int);
  Event(EventReceiver *, int, int, int);
  Event(EventReceiver *, int, int, const std::string &);
  Event(std::shared_ptr<EventReceiver> &, int);
  ~Event();
  EventReceiver * getReceiver() const;
  int getType() const;
  void * getData() const;
  int getDataLen() const;
  int getNumericalData() const;
  int getNumericalData2() const;
  std::string getStrData() const;
};
