/** @file CbmTofDigiExp.cxx
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 07.06.2013
 **
 ** Code for Data class for expanded digital TOF information
 **/
#include "CbmTofDigiExp.hpp"

CbmTofDigiExp::CbmTofDigiExp() : fdTime(0.), fdTot(-1.), fuAddress(0) {}

CbmTofDigiExp::CbmTofDigiExp(unsigned int address, double time, double tot)
    : fdTime(time), fdTot(tot), fuAddress(address) {}

CbmTofDigiExp::CbmTofDigiExp(unsigned int Sm,
                             unsigned int Rpc,
                             unsigned int Channel,
                             double time,
                             double tot,
                             unsigned int Side,
                             unsigned int SmType)
    : fdTime(time), fdTot(tot), fuAddress(0) {
  fuAddress = CbmTofAddress::GetUniqueAddress(Sm, Rpc, Channel, Side, SmType);
}

// --- Copy constructor
CbmTofDigiExp::CbmTofDigiExp(const CbmTofDigiExp& digiIn)
    : fdTime(digiIn.fdTime), fdTot(digiIn.fdTot), fuAddress(digiIn.fuAddress) {}
/*
// -----   Move constructor  -----------------------------------------------
CbmTofDigiExp::CbmTofDigiExp(CbmTofDigiExp&& other) {
  fdTime = other.fdTime;
  fdTot = other.fdTot;
  fuAddress = other.fuAddress;
}
// -------------------------------------------------------------------------
*/
// -----   Assignment operator  ----------- --------------------------------
CbmTofDigiExp& CbmTofDigiExp::operator=(const CbmTofDigiExp& rhs) {
  if (this != &rhs) {
    // CbmDigi::operator=(rhs);
    fdTime = rhs.fdTime;
    fdTot = rhs.fdTot;
    fuAddress = rhs.fuAddress;
  }
  return *this;
}
// -------------------------------------------------------------------------

// -----   Move Assignment operator   --------------------------------------
CbmTofDigiExp& CbmTofDigiExp::operator=(CbmTofDigiExp&& other) {
  if (this != &other) {
    // CbmDigi::operator=(std::forward<CbmTofDigiExp>(other));
    fuAddress = other.fuAddress;
    fdTime = other.fdTime;
    fdTot = other.fdTot;
  }
  return *this;
}
// -------------------------------------------------------------------------

std::string CbmTofDigiExp::ToString() const {
  char buff[100];
  sprintf(buff, "CbmTofDigi: address = 0x%08X time = %7f tot = %7f",
          GetAddress(), GetTime(), GetTot());
  // sprintf(buff, "a=0x%08X t=%8f v=%8f", GetAddress(), GetTime(), GetTot());
  return buff;
}

bool CbmTofDigiExp::operator<(const CbmTofDigiExp& rhs) const {
  return (this->GetTime() < rhs.GetTime()) ? true : false;
}

int CbmTofDigiExp::Compare(const CbmTofDigiExp* obj) const {
  if (this->GetTime() < obj->GetTime())
    // hit ... obj
    return -1;
  else if (this->GetTime() > obj->GetTime())
    // obj ... hit
    return 1;
  // obj = hit
  else
    return 0;
}

void CbmTofDigiExp::SetAddress(unsigned int Sm,
                               unsigned int Rpc,
                               unsigned int Channel,
                               unsigned int Side,
                               unsigned int SmType) {
  fuAddress = CbmTofAddress::GetUniqueAddress(Sm, Rpc, Channel, Side, SmType);
}
