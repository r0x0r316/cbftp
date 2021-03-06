#include "ftpconnect.h"

#include <cassert>

#include "core/iomanager.h"
#include "site.h"
#include "ftpconnectowner.h"
#include "ftpconn.h"
#include "globalcontext.h"
#include "proxysession.h"
#include "proxy.h"

#define WELCOME_TIMEOUT_MSEC 7000

FTPConnect::FTPConnect(int id, FTPConnectOwner * owner, const std::string & addr, const std::string & port, Proxy * proxy, bool primary, bool implicittls) :
  id(id),
  sockid(-1),
  proxynegotiation(false),
  proxysession(new ProxySession()),
  owner(owner),
  databuflen(DATABUF),
  databuf((char *) malloc(databuflen)),
  databufpos(0),
  addr(addr),
  port(port),
  proxy(proxy),
  primary(primary),
  engaged(true),
  connected(false),
  welcomereceived(false),
  millisecs(0),
  implicittls(implicittls)
{
  bool resolving;
  if (proxy == NULL) {
    sockid = global->getIOManager()->registerTCPClientSocket(this, addr, std::stoi(port), resolving, true);
    if (resolving) {
      owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Resolving]");
    }
    else {
      printConnecting(addr);
    }
  }
  else {
    proxynegotiation = true;
    proxysession->prepare(proxy, addr, port);
    sockid = global->getIOManager()->registerTCPClientSocket(this, proxy->getAddr(), std::stoi(proxy->getPort()), resolving, true);
    if (resolving) {
      owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Resolving proxy " + proxy->getAddr() + "]");
    }
    else {
      printConnecting(proxy->getAddr());
    }
  }
}

FTPConnect::~FTPConnect() {
  assert(!engaged); // must disengage before deleting; events may still be in the work queue
  delete proxysession;
  free(databuf);
}

void FTPConnect::FDConnecting(int sockid, const std::string & addr) {
  printConnecting(addr, true);

}

void FTPConnect::printConnecting(const std::string & addr, bool resolved) {
  if (!engaged) {
    return;
  }
  if (proxynegotiation) {
    owner->ftpConnectInfo(id, "[" + this->addr + ":" + port + "][Connecting to proxy " + addr + ":" + proxy->getPort() + "]");
  }
  else if (resolved) {
    owner->ftpConnectInfo(id, "[" + this->addr + ":" + port + "][Connecting to " + addr + ":" + port + "]");
  }
  else {
    owner->ftpConnectInfo(id, "[" + this->addr + ":" + port + "][Connecting]");
  }
}

void FTPConnect::FDConnected(int sockid) {
  if (!engaged) {
    return;
  }
  connected = true;
  millisecs = 0;
  owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Established]");
  if (proxynegotiation) {
    proxySessionInit();
  }
  else if (implicittls) {
    global->getIOManager()->negotiateSSLConnect(sockid);
  }
}

void FTPConnect::FDDisconnected(int sockid) {
  FDFail(sockid, "Disconnected");
}

void FTPConnect::FDData(int sockid, char * data, unsigned int datalen) {
  if (!engaged) {
    return;
  }
  if (proxynegotiation) {
    proxysession->received(data, datalen);
    proxySessionInit();
  }
  else {
    owner->ftpConnectInfo(id, std::string(data, datalen));
    if (FTPConn::parseData(data, datalen, &databuf, databuflen, databufpos, databufcode)) {
      if (databufcode == 220) {
        welcomereceived = true;
        owner->ftpConnectSuccess(id);
      }
      else {
        owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Unknown response]");
        disengage();
        owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Disconnected]");
        owner->ftpConnectFail(id);
      }
    }
  }
}

void FTPConnect::FDFail(int sockid, const std::string & error) {
  if (engaged) {
    engaged = false;
    owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][" + error + "]");
    owner->ftpConnectFail(id);
  }
}

void FTPConnect::FDSSLSuccess(int sockid, const std::string & cipher) {
  owner->ftpConnectInfo(id, "[Cipher: " + cipher + "]");
}

void FTPConnect::FDSSLFail(int sockid) {
  if (engaged) {
    engaged = false;
    owner->ftpConnectInfo(id, "[TLS negotiation failed]");
    owner->ftpConnectFail(id);
  }
}

int FTPConnect::getId() const {
  return id;
}

int FTPConnect::handedOver() {
  engaged = false;
  return sockid;
}

void FTPConnect::proxySessionInit() {
  switch (proxysession->instruction()) {
    case PROXYSESSION_SEND_CONNECT:
      owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Connecting through proxy]");
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SEND:
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SUCCESS:
      owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Established]");
      proxynegotiation = false;
      if (implicittls) {
        global->getIOManager()->negotiateSSLConnect(sockid);
      }
      break;
    case PROXYSESSION_ERROR:
      owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Proxy error: " + proxysession->getErrorMessage() + "]");
      disengage();
      owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Disconnected]");
      owner->ftpConnectFail(id);
      break;
  }
}

std::string FTPConnect::getAddress() const {
  return addr;
}

std::string FTPConnect::getPort() const {
  return port;
}

bool FTPConnect::isPrimary() const {
  return primary;
}

void FTPConnect::disengage() {
  if (engaged) {
    global->getIOManager()->closeSocket(sockid);
    engaged = false;
  }
}

void FTPConnect::tickIntern() {
  millisecs += 1000;
  if (millisecs >= WELCOME_TIMEOUT_MSEC) {
    if (engaged && connected && !welcomereceived) {
      owner->ftpConnectInfo(id, "[" + addr + ":" + port + "][Timeout while waiting for welcome message]");
      disengage();
      owner->ftpConnectFail(id);
    }
  }
}
