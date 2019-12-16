/** @file CbmTofDigi.cxx
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 07.06.2013
 **
 ** Code for Data class for compressed digital TOF information
 **/
#include "CbmTofDigi.hpp"
#include <iostream> //debug

// It seems C++ standard force the initialization to be in cxx/cpp file (outside
// of class definition)
// When not trivial constant values => To check if it should apply also to
// simple values maybe?

const double CbmTofDigi::fgkdTimeBinSize =
    10.0; // [ps] => 41 943.40 ns ( > GET4 e
const double CbmTofDigi::fgkdTotBinSize = 50.0; // [ps] => 0-51.200 range

// -----   Bit masks -----------------------------------------------------------
const long long int CbmTofDigi::fgklAddrMask =
    ((long long int)1 << CbmTofDigi::fgkiAddressSize) - 1;
const long long int CbmTofDigi::fgklTimeMask =
    ((long long int)1 << CbmTofDigi::fgkiTimeSize) - 1;
const long long int CbmTofDigi::fgklTotMask =
    ((long long int)1 << CbmTofDigi::fgkiTotSize) - 1;
// -----------------------------------------------------------------------------

// -----   Bit shifts   --------------------------------------------------------
const int CbmTofDigi::fgkiAddressOffs = 0;
const int CbmTofDigi::fgkiTimeOffs =
    CbmTofDigi::fgkiAddressOffs + CbmTofDigi::fgkiAddressSize;
const int CbmTofDigi::fgkiTotOffs =
    CbmTofDigi::fgkiTimeOffs + CbmTofDigi::fgkiTotSize;
// -----------------------------------------------------------------------------

CbmTofDigi::CbmTofDigi() : flData(0) {}

CbmTofDigi::CbmTofDigi(unsigned int address, double time, double tot)
    : flData(0) {
  SetAddress(address);
  SetTime(time);
  SetTot(tot);
}

CbmTofDigi::CbmTofDigi(unsigned int Sm,
                       unsigned int Rpc,
                       unsigned int Channel,
                       double time,
                       double tot,
                       unsigned int Side,
                       unsigned int SmType)
    : flData(0) {
  SetAddress(Sm, Rpc, Channel, Side, SmType);
  SetTime(time);
  SetTot(tot);
}

void CbmTofDigi::SetAddress(unsigned int address) {
  // First set old address to zero
  flData = flData & ~(fgklAddrMask << fgkiAddressOffs);

  // Now set new address
  flData = flData | ((address & fgklAddrMask) << fgkiAddressOffs);
};
void CbmTofDigi::SetAddress(unsigned int Sm,
                            unsigned int Rpc,
                            unsigned int Channel,
                            unsigned int Side,
                            unsigned int SmType) {
  int iAddress =
      CbmTofAddress::GetUniqueAddress(Sm, Rpc, Channel, Side, SmType);

  // First set old address to zero
  flData = flData & ~(fgklAddrMask << fgkiAddressOffs);

  // Now set new address
  flData = flData | ((iAddress & fgklAddrMask) << fgkiAddressOffs);
}
void CbmTofDigi::SetTime(double time) {
  // std::cout << flData << " Time: " << time << " binSize " << fgkdTimeBinSize
  // << " erg: " << time / fgkdTimeBinSize;
  int iBinnedTime = static_cast<int>(time / fgkdTimeBinSize);
  // std::cout << " Binned:" << iBinnedTime;
  // First set old time to zero
  flData = flData & ~(fgklTimeMask << fgkiTimeOffs);

  // Now set new time
  flData = flData | ((iBinnedTime & fgklTimeMask) << fgkiTimeOffs);
  // std::cout << flData << std::endl;
};
void CbmTofDigi::SetTot(double tot) {
  int iBinnedTot = (int)(tot / fgkdTotBinSize);

  // First set old tot to zero
  flData = flData & ~(fgklTotMask << fgkiTotOffs);

  // Now set new tot
  flData = flData | ((iBinnedTot & fgklTotMask) << fgkiTotOffs);
};

std::string CbmTofDigi::ToString() const {

  // TString string = Form( "CbmTrdDigi: address = 0x%08X time = %7f tot = %7f",
  //                       GetAddress(), GetCharge(), GetTot() );
  // return string.Data();
  char buff[80];
  sprintf(buff, "CbmTrdDigi: address = 0x%08X time = %7f tot = %7f",
          this->GetAddress(), GetTime(), GetTot());
  return buff;
  // return "Function not available atm";
}

bool CbmTofDigi::operator<(const CbmTofDigi& rhs) const {
  return (this->GetTime() < rhs.GetTime()) ? true : false;
}

int CbmTofDigi::Compare(const CbmTofDigi* obj) const {
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
