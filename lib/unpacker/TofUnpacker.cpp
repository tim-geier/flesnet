#include "TofUnpacker.hpp"

#define BYTES_PER_MESSAGE 8

TofUnpacker::TofUnpacker(std::ostream& arg_out) : out_(arg_out) {}

TofUnpacker::~TofUnpacker() {}

bool TofUnpacker::load_mapping(std::string path) {
  out_ << "Reading mapping from file: " << path << std::endl;
  std::ifstream paramFile;
  paramFile.open(path);
  if (!paramFile.good()) // File not found or not accessible
    return false;

  mapping.clear();
  unsigned int key, value;
  while (paramFile >> key >> value) {
    mapping[key] = value;
  }

  paramFile.close();
  out_ << "Finished reading mapping." << std::endl;
  return true;
}

void TofUnpacker::process_microslice(const uint64_t* data,
                                     const uint32_t ms_size,
                                     std::vector<CbmTofDigiExp>* digiVect) {
  unsigned long long int current_epoch_cycle = 0;
  unsigned long long int lastEpoch = 0;
  unsigned int counter = 0;
  unsigned int errors = 0;

  for (size_t i = 0;
       i < (ms_size - (ms_size % BYTES_PER_MESSAGE)) / BYTES_PER_MESSAGE; i++) {
    // Loop over messages in microslice
    if (i == 0) // First MS = Epoch Cycle;
    {
      current_epoch_cycle = data[i] & gdpbv100::kulEpochCycleFieldSz;
    }

    gdpbv100::Message mess(data[i]);

    switch (mess.getMessageType()) {
    case gdpbv100::MSG_HIT: {
      if (lastEpoch == 0) {
        // Missing Epoch information at start of microslice
        // Ignore current message and continue...
        errors++;
        break;
      }

      unsigned int detectorAddress = (mess.getGdpbGenGdpbId() << 12) +
                                     (mess.getGdpbGenChipId() << 4) +
                                     mess.getGdpbHitChanId();
      auto it = mapping.find(detectorAddress);
      if (it == mapping.end()) {
        // Address not found -> Hit not mapped to digi
        continue;
      }

      digiVect->emplace_back(it->second, mess.getMsgFullTimeD(lastEpoch),
                             mess.getGdpbHit32Tot());
      counter++;
      break;
    }
    case gdpbv100::MSG_EPOCH: {
      lastEpoch = mess.getGdpbEpEpochNb() +
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
  return;
}