// Copyright 2019 Tim Geier <mail@tim-geier.de>
#include "TofUnpacker.hpp"

#define BYTES_PER_MESSAGE 8

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

TofUnpacker::TofUnpacker(std::ostream& arg_out) : out_(arg_out) {}

TofUnpacker::~TofUnpacker() {}

bool TofUnpacker::load_mapping(std::string path) {

  // DPB + ASIC + Channel wird im folgenden Format erwartet (32 Bit):
  // DPB      00001111111111111111000000000000
  // ASIC     00000000000000000000111111110000
  // Channel  00000000000000000000000000001111  -- 4 Bit wegen besserer
  // Lesbarkeit als HEX

  // TODO:
  // Channel auf 2 Bit ändern

  out_ << "Reading mapping from file: " << path << std::endl;
  std::ifstream paramFile;
  paramFile.open(path);
  if (!paramFile.good()) // File not found or not accessible
    return false;

  mapping.clear();
  unsigned int key, value;
  unsigned short int dpb = 0;
  while (paramFile >> key >> value) {
    dpb = (key >> 12);
    auto it = mapping.find(dpb);
    if (it == mapping.end()) {
      // DPB not found -> insert new vector
      std::vector<unsigned int> vect;
      vect.resize(1024); // 2^12
      vect[((key & 0x00000FF0) >> 2) | (key & 0x00000003)] = value;
      mapping[dpb] = vect;
    } else {
      mapping[dpb][((key & 0x00000FF0) >> 2) | (key & 0x00000003)] = value;
    }
  }

  paramFile.close();
  out_ << "Finished reading mapping." << std::endl;
  return true;
}

void TofUnpacker::process_microslice(const uint64_t* data,
                                     const uint32_t ms_size,
                                     const unsigned short dbp_id,
                                     std::vector<CbmTofDigiExp>* digiVect) {
  unsigned long long int current_epoch_cycle;
  unsigned long long int lastEpoch = 0;
  unsigned int errors = 0;

  auto it = mapping.find(dbp_id);
  if (unlikely(it == mapping.end())) {
    // DPB not found, something wrong with configuration...
    return;
  } else {

    current_epoch_cycle =
        data[0] & gdpbv100::kulEpochCycleFieldSz; // First MS = Epoch Cycle;

    // Loop over messages in microslice
    for (unsigned int i = 1;
         i < (ms_size - (ms_size % BYTES_PER_MESSAGE)) / BYTES_PER_MESSAGE;
         ++i) {

      // cast current data to message, doesn't need constructor call
      const gdpbv100::Message* mess =
          reinterpret_cast<const gdpbv100::Message*>(&data[i]);

      switch (mess->getMessageType()) {
      case gdpbv100::MSG_HIT: {
        if (lastEpoch == 0) {
          // Missing Epoch information at start of microslice
          // Ignore current message and continue...
          errors++;
          break;
        }

        unsigned int detectorAddress =
            it->second[(mess->getGdpbGenChipId() << 2) +
                       mess->getGdpbHitChanId()];

        if (detectorAddress == 0)
          continue; // Hit not mapped to digi

        digiVect->emplace_back(detectorAddress,
                               mess->getMsgFullTimeD(lastEpoch),
                               mess->getGdpbHit32Tot());
        break;
      }
      case gdpbv100::MSG_EPOCH: {
        lastEpoch = mess->getGdpbEpEpochNb() +
                    (gdpbv100::kuEpochCounterSz + 1) * current_epoch_cycle;

        break;
      }
      case gdpbv100::MSG_SYST: {
        // out_ << "SYST Message" << std::endl;

        break;
      }
      default: {
        // out_ << "Unbekannter Typ: " << mtMsgTyp << std::endl;

        errors++;
        break;
      }
      }
    }
  }
  return;
}