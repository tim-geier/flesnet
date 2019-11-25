/** @file CbmTofAddress.cxx
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 07.06.2013
 **
 ** Empty file, just there for the sake of the build system.
 **/

#include "CbmTofAddress.hpp" 
//#include "Riostream.h"

// It seems C++ standard force the initialization to be in cxx/cpp file (outside of class definition)
// When not trivial constant values => To check if it should apply also to simple values maybe?
/** Offset in bits for Super Module Id in the address field  **/
const int CbmTofAddress::fgkSmIdOffset        = fgkSystemBits;
/** Offset in bits for Super Module Type in the address field  **/
const int CbmTofAddress::fgkSmTypeOffset      = CbmTofAddress::fgkSmIdBits        + CbmTofAddress::fgkSmIdOffset;
/** Offset in bits for Rpc Id in the address field  **/
const int CbmTofAddress::fgkRpcIdOffset       = CbmTofAddress::fgkSmTypeBits      + CbmTofAddress::fgkSmTypeOffset; 
/** Offset in bits for Channel Side Id in the address field  **/
const int CbmTofAddress::fgkChannelSideOffset = CbmTofAddress::fgkRpcIdBits       + CbmTofAddress::fgkRpcIdOffset;
/** Offset in bits for Channel Id in the address field  **/
const int CbmTofAddress::fgkChannelIdOffset   = CbmTofAddress::fgkChannelSideBits + CbmTofAddress::fgkChannelSideOffset;

const int CbmTofAddress::fgkiModFullIdMask    = ( ( ( 1 << fgkSystemBits ) - 1 )                    ) +
                                                   ( ( ( 1 << fgkSmIdBits   ) - 1 ) << fgkSmIdOffset   ) +
                                                   ( ( ( 1 << fgkSmTypeBits ) - 1 ) << fgkSmTypeOffset ) +
                                                   ( ( ( 1 << fgkRpcIdBits  ) - 1 ) << fgkRpcIdOffset  );
