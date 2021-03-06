#include "pendingtransfer.h"

#include "localfilelist.h"

PendingTransfer::PendingTransfer(const std::shared_ptr<SiteLogic> & slsrc, FileList * flsrc, std::string srcfilename, const std::shared_ptr<SiteLogic> & sldst, FileList * fldst, std::string dstfilename) :
  slsrc(slsrc),
  sldst(sldst),
  flsrc(flsrc),
  fldst(fldst),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_FXP) {
}

PendingTransfer::PendingTransfer(const std::shared_ptr<SiteLogic> & sl, FileList * fl, std::string srcfilename, std::shared_ptr<LocalFileList> fllocal, std::string dstfilename) :
  slsrc(sl),
  sldst(nullptr),
  flsrc(fl),
  fldst(nullptr),
  fllocal(fllocal),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_DOWNLOAD) {
}

PendingTransfer::PendingTransfer(std::shared_ptr<LocalFileList> fllocal, std::string srcfilename, const std::shared_ptr<SiteLogic> & sl, FileList * fl, std::string dstfilename) :
  slsrc(nullptr),
  sldst(sl),
  flsrc(nullptr),
  fldst(fl),
  fllocal(fllocal),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_UPLOAD) {
}

PendingTransfer::~PendingTransfer() {

}

int PendingTransfer::type() const {
  return transfertype;
}

const std::shared_ptr<SiteLogic> & PendingTransfer::getSrc() const {
  return slsrc;
}

const std::shared_ptr<SiteLogic> & PendingTransfer::getDst() const {
  return sldst;
}

FileList * PendingTransfer::getSrcFileList() const {
  return flsrc;
}

FileList * PendingTransfer::getDstFileList() const {
  return fldst;
}

std::shared_ptr<LocalFileList> & PendingTransfer::getLocalFileList() {
  return fllocal;
}

std::string PendingTransfer::getSrcFileName() const {
  return srcfilename;
}

std::string PendingTransfer::getDstFileName() const {
  return dstfilename;
}
