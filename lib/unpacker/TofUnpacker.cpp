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
  unsigned int msg_count =
      (ms_size - (ms_size % BYTES_PER_MESSAGE)) / BYTES_PER_MESSAGE;

  for (size_t i = 0; i < msg_count; i++) {
    // Loop over messages in microslice
    if (i == 0) // First MS = Epoch Cycle;
    {
      current_epoch_cycle = data[i] & gdpbv100::kulEpochCycleFieldSz;
    }

    gdpbv100::Message mess(data[i]);
    gdpbv100::MessageTypes mtMsgTyp =
        static_cast<gdpbv100::MessageTypes>(mess.getMessageType());

    switch (mtMsgTyp) {
    case gdpbv100::MSG_HIT: {
      if (lastEpoch == 0) {
        // Missing Epoch information at start of microslice
        // Ignore current message and continue...
        errors++;
        break;
      }

#ifdef DEBUG_ALL
      out_ << "HIT: " << std::endl;
      out_ << "\t MAC: \t" << std::hex << mess.getGdpbGenGdpbId() << std::dec
           << std::endl;
      out_ << "\t ASIC: \t" << mess.getGdpbGenChipId() << std::endl;
      if (mess.getGdpbHitIs24b() == 0) {
        out_ << "\t TOT: \t" << mess.getGdpbHit32Tot() << std::endl;
      } else {
        out_ << "\t TOT: \t Hit 24b - not implemented" << std::endl;
        // timeslice_error_count_ ++;
      }
      out_ << "\t Time (ns): \t" << mess.getMsgFullTimeD(lastEpoch)
           << std::endl;
#endif // DEBUG_ALL

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
#ifdef DEBUG
      out_ << "New digi: " << digi->ToString() << std::endl;
#endif // DEBUG

      break;
    }
    case gdpbv100::MSG_EPOCH: {
      lastEpoch = mess.getGdpbEpEpochNb() +
                  (gdpbv100::kuEpochCounterSz + 1) * current_epoch_cycle;
#ifdef DEBUG
      out_ << "EPOCH: " << lastEpoch << " Typ: "
           << (mess.getGdpbGenChipId() == 255 ? "Merged" : "Unsupported type!")
           << std::endl;
#endif // DEBUG
      break;
    }
    case gdpbv100::MSG_SYST: {
#ifdef DEBUG
      out_ << "SYST Message" << std::endl;
#endif // DEBUG
      break;
    }
    default: {
#ifdef DEBUG
      out_ << "Unbekannter Typ: " << mtMsgTyp << std::endl;
#endif // DEBUG
      errors++;
    }
    }
  }
  return;
}