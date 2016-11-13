// Copyright 2016 Qi Wang
#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <list>
#include <string>
#include "global.h"

class Controler {
 public:
  static Controler& instance();
  void run();

 private:
  Controler();
  Controler(const Controler&);
  Controler& operator =(const Controler&);

  std::list<std::string> m_cmd;
  QUERY_TYPE m_queryType;
  friend class Shell;
};

inline Controler& Controler::instance() {
  static Controler obj;
  return obj;
}

#endif  // CONTROLLER_H_
