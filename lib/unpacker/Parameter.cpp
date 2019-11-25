// Needed for cmake
// DO NOT DELETE
#include "Parameter.hpp"
#include "gDpbMessv100.hpp"

unsigned int Parameter::ElinkIdxToGet4Idx( unsigned int uElink )
{
   if( gdpbv100::kuChipIdMergedEpoch == uElink  )
      return uElink;
   else if( uElink < kuNbGet4PerGdpb )
      return kuElinkToGet4[ uElink % kuNbGet4PerGbtx ] + kuNbGet4PerGbtx * ( uElink / kuNbGet4PerGbtx );
      else
      {
         return -1;
      } // else of if( uElink < kuNbGet4PerGbtx )
}

unsigned int Parameter::Get4ChanToPadiChan( unsigned int uChannelInFee )
{
   if( uChannelInFee < kuNbChannelsPerFee )
      return kuGet4topadi[ uChannelInFee ];
      else
      {
         return -1;
      } // else of if( uChannelInFee < kuNbChannelsPerFee )
}