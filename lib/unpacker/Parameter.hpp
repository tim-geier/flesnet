//[CbmMcbm2018TofPar]
//----------------------------------------------------------------------------
class Parameter
{
public:
   /// Constants
      /// Data format
   static const unsigned int kuBytesPerMessage = 8;
      /// Readout chain
   static const unsigned int kuNbChannelsPerGet4 = 4;
   static const unsigned int kuNbGet4PerFee      = 8;
   static const unsigned int kuNbFeePerGbtx      = 5;
   static const unsigned int kuNbGbtxPerGdpb     = 6;
   static const unsigned int kuNbChannelsPerFee  = kuNbChannelsPerGet4 * kuNbGet4PerFee;
   static const unsigned int kuNbChannelsPerGbtx = kuNbChannelsPerFee  * kuNbFeePerGbtx;
   static const unsigned int kuNbChannelsPerGdpb = kuNbChannelsPerGbtx * kuNbGbtxPerGdpb;
   static const unsigned int kuNbGet4PerGbtx     = kuNbGet4PerFee      * kuNbFeePerGbtx;
   static const unsigned int kuNbGet4PerGdpb     = kuNbGet4PerGbtx     * kuNbGbtxPerGdpb;
   static const unsigned int kuNbFeePerGdpb      = kuNbFeePerGbtx      * kuNbGbtxPerGdpb;
      /// Mapping in Readout chain PCBs
   inline static constexpr unsigned int kuGet4topadi[ kuNbChannelsPerFee ] = {
          3,  2,  1,  0,
          7,  6,  5,  4,
         11, 10,  9,  8,
         15, 14, 13, 12,
         19, 18, 17, 16,
         23, 22, 21, 20,
         27, 26, 25, 24,
         31, 30, 29, 28
      }; //! Map from GET4 channel to PADI channel

   inline static constexpr unsigned int kuPaditoget4[ kuNbChannelsPerFee ] = {
          3,  2,  1,  0,
          7,  6,  5,  4,
         11, 10,  9,  8,
         15, 14, 13, 12,
         19, 18, 17, 16,
         23, 22, 21, 20,
         27, 26, 25, 24,
         31, 30, 29, 28
      }; //! Map from PADI channel to GET4 channel
   inline static constexpr unsigned int kuElinkToGet4[ kuNbGet4PerGbtx ] = {
         27,  2,  7,  3, 31, 26, 30,  1,
         33, 37, 32, 13,  9, 14, 10, 15,
         17, 21, 16, 35, 34, 38, 25, 24,
          0,  6, 20, 23, 18, 22, 28,  4,
         29,  5, 19, 36, 39,  8, 12, 11
      };
   inline static constexpr unsigned int kuGet4ToElink[ kuNbGet4PerGbtx ] = {
         24,  7,  1,  3, 31, 33, 25,  2,
         37, 12, 14, 39, 38, 11, 13, 15,
         18, 16, 28, 34, 26, 17, 29, 27,
         23, 22,  5,  0, 30, 32,  6,  4,
         10,  8, 20, 19, 35,  9, 21, 36
      };

    inline static constexpr int NrOfGdpbs = 5;
    inline static constexpr int GdpbIdArray[] = {0x1980, 0x1922, 0x1949, 0x18c5, 0x1925};
    inline static constexpr int NrOfFeesPerGdpb = 30;
    inline static constexpr int NrOfGet4PerFee = 8;
    inline static constexpr int NrOfChannelsPerGet4 = 4;
    inline static constexpr int NrOfGbtx = 30;
    inline static constexpr int NrOfModule = 9;
    inline static constexpr int NrOfRpc[] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
    inline static constexpr int intRpcType[] ={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 78, 6, -1, -1 ,-1, 5, -1, -1, -1, -1, -1, 5, -1, -1, -1, -1, -1};
    inline static constexpr int RpcSide[] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1};
    inline static constexpr int ModuleId[] = {3, 3, 4, 4, -1, -1, 0, 0, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    inline static constexpr int NbMsTot = 100;
    inline static constexpr int NbMsOverlap = 1;
    inline static constexpr double SizeMsInNs = 102400.0;
    //SizeMsInNs: Double_t 1638400
    inline static constexpr double StarTriggerDeadtime[] = {1000.0, 1000.0, 1000.0, 1000.0, 1000.0};
    inline static constexpr double StarTriggerDelay[] = {2000.0,  2000.0,  2000.0,  2000.0,  2000.0};
    //  2000.0  2000.0  2000.0  2000.0  2000.0
    //-23000.0  -23000.0  -23000.0  -23000.0  -23000.0
    inline static constexpr double StarTriggerWinSize[]  {2000.0,  2000.0,  2000.0,  2000.0,  2000.0};
    inline static constexpr double TsDeadtimePeriod = 62.5;

    static unsigned int ElinkIdxToGet4Idx (unsigned int);
    static unsigned int Get4ChanToPadiChan( unsigned int uChannelInFee );

};
