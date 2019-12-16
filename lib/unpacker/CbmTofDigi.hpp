/** CbmTofDigi.h
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 07.06.2013
 **/

/** @class CbmTofDigi
 ** @brief Data class for expanded digital TOF information
 ** @brief Data level: TDC CALIB
 ** @version 1.0
 **
 ** The information is encoded into 1*8 bytes (1 long long int).
 ** Unique Address:                 32 bits following CbmTofAddress
 ** Calibrated Time [T Digig Bin]:  22 bits with Bin Size defined as constant in
 *this file
 ** Calibrated Tot  [Tot Digi Bin]: 10 bits with Bin Size defined as constant in
 *this file
 **
 ** In triggered setup, the time is relative to the trigger time, which
 ** is measured with a resolution of a few ns corresponding to the TDC
 ** system clock cycle.
 ** In free-streaming setups, the time is relative to the last epoch.
 **/

#ifndef CBMTOFDIGI_H
#define CBMTOFDIGI_H 1

#include "CbmTofAddress.hpp"

class TString;
#include <string>

class CbmTofDigi {
public:
  /**
   ** @brief Default constructor.
   **/
  CbmTofDigi();

  /**
   ** @brief Constructor with assignment.
   ** @param[in] address Unique channel address. (cf CbmTofAddress)
   ** @param[in] time    Absolute time [ps].
   ** @param[in] tot     Time Over Threshold [ps].
   **/
  CbmTofDigi(unsigned int address, double time, double tot);

  /**
   ** @brief Constructor with detailled assignment.
   ** @param[in] Sm      Super Module Id. (cf CbmTofAddress)
   ** @param[in] Rpc     Rpc Id. (cf CbmTofAddress)
   ** @param[in] Channel Channel Id. (cf CbmTofAddress)
   ** @param[in] time    Absolute time [ps].
   ** @param[in] tot     Time Over Threshold [ps].
   ** @param[in] Side    Channel Side (optional, used for strips). (cf
   *CbmTofAddress)
   ** @param[in] Sm Type Super Module Type (optional). (cf CbmTofAddress)
   **/
  CbmTofDigi(unsigned int Sm,
             unsigned int Rpc,
             unsigned int Channel,
             double time,
             double tot,
             unsigned int Side = 0,
             unsigned int SmType = 0);

  /** Accessors **/
  /**
   ** @brief Inherited from CbmDigi.
   **/
  int GetAddress() const {
    return ((flData >> fgkiAddressOffs) & fgklAddrMask);
  };

  /**
   ** @brief Inherited from CbmDigi.
   **/
  int GetSystemId() const { return CbmTofAddress::GetSystemId(GetAddress()); };

  /**
   ** @brief Inherited from CbmDigi.
   **/
  double GetTime() const {
    return fgkdTimeBinSize * ((flData >> fgkiTimeOffs) & fgklTimeMask);
  };

  /**
   ** @brief Inherited from CbmDigi.
   **/
  double GetCharge() const {
    return fgkdTotBinSize * ((flData >> fgkiTotOffs) & fgkiTotOffs);
  };
  /**
   ** @brief Alias for GetCharge.
   **/
  double GetTot() const { return GetCharge(); };
  /**
   ** @brief Sm.
   **/
  double GetSm() const { return CbmTofAddress::GetSmId(GetAddress()); };
  /**
   ** @brief Sm Type .
   **/
  double GetType() const { return CbmTofAddress::GetSmType(GetAddress()); };
  /**
   ** @brief Detector aka Module aka RPC .
   **/
  double GetRpc() const { return CbmTofAddress::GetRpcId(GetAddress()); };
  /**
   ** @brief Channel .
   **/
  double GetChannel() const {
    return CbmTofAddress::GetChannelId(GetAddress());
  };
  /**
   ** @brief Channel Side.
   **/
  double GetSide() const {
    return CbmTofAddress::GetChannelSide(GetAddress());
  };
  /**
   ** @brief Full data acess.
   **/
  long long int GetData() const { return flData; };

  /**
   ** @brief Sorting using the time, assumes Digis are in same reference frame
   *(e.g. same epoch).
   **/
  bool operator<(const CbmTofDigi& rhs) const;

  int Compare(const CbmTofDigi* obj) const;
  bool IsSortable() const { return true; };

  /** Modifiers **/
  void SetAddress(unsigned int address);
  void SetAddress(unsigned int Sm,
                  unsigned int Rpc,
                  unsigned int Channel,
                  unsigned int Side = 0,
                  unsigned int SmType = 0);
  void SetTime(double time);
  void SetTot(double tot);

  std::string ToString() const;

private:
  long long int flData; ///< Fully compressed data

  // -----   Bit fields sizes --------------------------------------------------
  static const int fgkiAddressSize = 32;
  static const int fgkiTimeSize = 22;
  static const int fgkiTotSize = 10;
  // -----------------------------------------------------------------------------

  // -----   Bit masks
  // -----------------------------------------------------------
  static const long long int fgklAddrMask;
  static const long long int fgklTimeMask;
  static const long long int fgklTotMask;
  // -----------------------------------------------------------------------------

  // -----   Bit shifts --------------------------------------------------------
  static const int fgkiAddressOffs;
  static const int fgkiTimeOffs;
  static const int fgkiTotOffs;
  // -----------------------------------------------------------------------------

  static const double
      fgkdTimeBinSize; // [ps] => 41 943.40 ns ( > GET4 epoch, > trigger window)
  static const double fgkdTotBinSize; // [ps] => 0-51.200 range
};
#endif // CBMTOFDIGI_H
