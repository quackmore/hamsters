/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <quackmore-ff@yahoo.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return. Quackmore
 * ----------------------------------------------------------------------------
 */

#ifndef __MDNS_HPP__
#define __MDNS_HPP__

extern "C"
{
#include "espconn.h"
}

class Mdns
{
private:
  struct mdns_info m_info;

public:
  Mdns(){};
  ~Mdns(){};

  void start(char *app_alias);
  void stop(void);
};

#endif