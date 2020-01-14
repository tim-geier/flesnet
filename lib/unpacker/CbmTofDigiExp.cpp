/** @file CbmTofDigiExp.cxx
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 07.06.2013
 **
 ** Code for Data class for expanded digital TOF information
 **/
#include "CbmTofDigiExp.hpp"

CbmTofDigiExp::CbmTofDigiExp() : fdTime(0.), fdTot(0), fuAddress(0) {}

CbmTofDigiExp::CbmTofDigiExp(unsigned int address,
                             double time,
                             unsigned int tot)
    : fdTime(time), fdTot(tot), fuAddress(address) {}

CbmTofDigiExp::CbmTofDigiExp(unsigned int Sm,
                             unsigned int Rpc,
                             unsigned int Channel,
                             double time,
                             unsigned int tot,
                             unsigned int Side,
                             unsigned int SmType)
    : fdTime(time), fdTot(tot), fuAddress(0) {
  fuAddress = CbmTofAddress::GetUniqueAddress(Sm, Rpc, Channel, Side, SmType);
}

std::string CbmTofDigiExp::ToString() const {
  char buff[100];
  sprintf(buff, "CbmTofDigi: address = 0x%08X time = %7f tot = %7f",
          GetAddress(), GetTime(), (double)GetTot());
  // sprintf(buff, "a=0x%08X t=%8f v=%8f", GetAddress(), GetTime(), GetTot());
  return buff;
}

bool CbmTofDigiExp::operator<(const CbmTofDigiExp& rhs) const {
  return this->GetTime() < rhs.GetTime();
}

bool CbmTofDigiExp::operator>=(const CbmTofDigiExp& rhs) const {
  return this->GetTime() >= rhs.GetTime();
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
