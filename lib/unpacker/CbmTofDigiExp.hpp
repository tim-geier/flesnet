/** CbmTofDigiExp.h
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 07.06.2013
 **/
 
/** @class CbmTofDigiExp
 ** @brief Data class for expanded digital TOF information
 ** @brief Data level: TDC CALIB
 ** @version 1.0
 **
 ** The information is encoded into 3*4 bytes (2 double + 1 unsigned int).
 ** Unique Address:                32 bits following CbmTofAddress
 ** Calibrated Time [ps]:          32 bits double
 ** Calibrated Tot  [ps]:          32 bits double
 **
 ** In triggered setup, the time is relative to the trigger time, which 
 ** is measured with a resolution of a few ns corresponding to the TDC 
 ** system clock cycle. 
 ** In free-streaming setups, the time is relative to the last epoch.
 **/ 
 
#ifndef CBMTOFDIGIEXP_H
#define CBMTOFDIGIEXP_H 1

#include "CbmTofAddress.hpp"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

#include <string>

class CbmTofDigiExp
{
   public:
      /**
       ** @brief Default constructor.
       **/
      CbmTofDigiExp();

      /**
       ** @brief Constructor with assignment.
       ** @param[in] address Unique channel address. (cf CbmTofAddress)
       ** @param[in] time    Absolute time [ps].
       ** @param[in] tot     Time Over Threshold [ps].
       **/
      CbmTofDigiExp( unsigned int address, double time, double tot );

      /**
       ** @brief Constructor with detailled assignment.
       ** @param[in] Sm      Super Module Id. (cf CbmTofAddress)
       ** @param[in] Rpc     Rpc Id. (cf CbmTofAddress)
       ** @param[in] Channel Channel Id. (cf CbmTofAddress)
       ** @param[in] time    Absolute time [ps].
       ** @param[in] tot     Time Over Threshold [ps].
       ** @param[in] Side    Channel Side (optional, used for strips). (cf CbmTofAddress)
       ** @param[in] Sm Type Super Module Type (optional). (cf CbmTofAddress)
       **/
      CbmTofDigiExp( unsigned int Sm, unsigned int Rpc, unsigned int Channel, 
                     double time, double tot, 
                     unsigned int Side = 0, unsigned int SmType = 0 );

      /**
       ** @brief Copy constructor.
       **/
      CbmTofDigiExp(const CbmTofDigiExp&);


      /** Move constructor  **/
      CbmTofDigiExp(CbmTofDigiExp&&);


      /** Assignment operator  **/
      CbmTofDigiExp& operator=(const CbmTofDigiExp&);


      /** Move Assignment operator  **/
      CbmTofDigiExp& operator=(CbmTofDigiExp&&);


      /**
       ** @brief Destructor.
       **/
      ~CbmTofDigiExp();

      /** Accessors **/
         /**
          ** @brief Inherited from CbmDigi.
          **/
      int GetAddress() const { return fuAddress; };

         /**
          ** @brief Inherited from CbmDigi.
          **/
      int GetSystemId() const { return CbmTofAddress::GetSystemId(fuAddress); };

         /**
          ** @brief Inherited from CbmDigi.
          **/
      double GetTime() const { return fdTime; };

         /**
          ** @brief Inherited from CbmDigi.
          **/
      double GetCharge() const { return fdTot; };
         /**
          ** @brief Alias for GetCharge.
          **/
      double GetTot()    const { return GetCharge(); };
         /**
          ** @brief Sm.
          **/
      double GetSm()    const { return CbmTofAddress::GetSmId( GetAddress() ); };
         /**
          ** @brief Sm Type .
          **/
      double GetType()    const { return CbmTofAddress::GetSmType( GetAddress() ); };
         /**
          ** @brief Detector aka Module aka RPC .
          **/
      double GetRpc()    const { return CbmTofAddress::GetRpcId( GetAddress() ); };
         /**
          ** @brief Channel .
          **/
      double GetChannel()    const { return CbmTofAddress::GetChannelId( GetAddress() ); };
         /**
          ** @brief Channel Side.
          **/
      double GetSide()    const { return CbmTofAddress::GetChannelSide( GetAddress() ); };

         /**
          ** @brief Sorting using the time, assumes Digis are in same reference frame (e.g. same epoch).
          **/

     virtual bool operator <( const CbmTofDigiExp& rhs) const; 
     // virtual int	 Compare(      const TObject* obj) const;
      virtual int	 Compare(      const CbmTofDigiExp*  obj) const;
      virtual bool IsSortable() const { return true; };

      /** Modifiers **/
      void SetAddress(int address) { fuAddress = address; };
      void SetAddress( unsigned int Sm, unsigned int Rpc, unsigned int Channel,
                        unsigned int Side = 0, unsigned int SmType = 0 );
      void SetTime(double time) { fdTime = time; };
      void SetTot(double tot) { fdTot = tot; };

      std::string ToString() const;

/*
      CbmMatch* GetMatch() const { return fMatch; }

      void SetMatch(CbmMatch* match) { fMatch = match; }
*/


   
   private:
      double fdTime;    ///< Absolute time [ps]
      double fdTot;     ///< Tot [ps]
      unsigned int   fuAddress; ///< Unique channel address
     // CbmMatch* fMatch;   ///< Match to MC

      friend class boost::serialization::access;

      template <class Archive>
      void serialize(Archive& ar, const unsigned int /*version*/)
      {
        ar& fuAddress;
        ar& fdTime;
        ar& fdTot;
      }
      
  // ClassDef(CbmTofDigiExp, 1);
};
#endif // CBMTOFDIGIEXP_H
