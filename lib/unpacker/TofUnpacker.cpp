// Copyright 2019-2020 Tim Geier <mail@tim-geier.de>
#include "TofUnpacker.hpp"

#define BYTES_PER_MESSAGE 8

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// workaround for missing EPOCH message at start of microslice
// can be deleted as soon as hardware generates specification conform data...
#define TOF_UNPACKER_USE_MISSING_EPOCH_QUIRKS_MODE

// enable second output vector for all messages except EPOCH and HIT
//#define TOF_UNPACKER_PROCESS_INFO_MESSAGES

// enable warning message if channel is mapped to 0
#define TOF_UNPACKER_WARN_WRONG_MAPPING

TofUnpacker::TofUnpacker(std::ostream& arg_out) : out_(arg_out) {}

TofUnpacker::~TofUnpacker() {}

bool TofUnpacker::reset_error_counters() {
  errors = 0;
  unprocessed_messages = 0;
  unmapped_messages = 0;
  return true;
}

bool TofUnpacker::load_mapping(std::string path) {

  // DPB + ASIC + Channel schould be as following (32 Bit):
  // DPB      00001111111111111111000000000000
  // ASIC     00000000000000000000111111110000
  // Channel  00000000000000000000000000001111

  // TODO:
  // Change Channel to 2 Bit in mapping file

  out_ << "Reading mapping from file: " << path << std::endl;
  std::ifstream mappingFile;
  mappingFile.open(path);
  if (!mappingFile.good()) // File not found or not accessible
  {
    mapping_loaded = false;
    return false;
  }

  mapping.clear();
  unsigned int key, value;
  unsigned short int dpb = 0;
  while (mappingFile >> key >> value) {
    dpb = (key >> 12);
    if (!value) {
#ifdef TOF_UNPACKER_WARN_WRONG_MAPPING
      out_ << "WARNING: DPB 0x" << std::hex << dpb << std::dec << " ASIC "
           << ((key & 0x00000FF0) >> 4) << " CH " << (key & 0x00000003)
           << " mapped to " << value << std::endl;
#endif
      continue;
    }
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

  mappingFile.close();
  out_ << "Finished reading mapping." << std::endl;
  mapping_loaded = true;
  return true;
}

void TofUnpacker::process_microslice(fles::MicrosliceView ms,
                                     std::vector<CbmTofDigiExp>* digiVect) {

  const uint64_t* ms_data = reinterpret_cast<const uint64_t*>(ms.content());
  const uint32_t ms_size = ms.desc().size;

  unsigned long long int current_epoch_cycle;
  unsigned long long int lastEpoch = 0;

  auto it = mapping.find(ms.desc().eq_id);
  if (unlikely(it == mapping.end())) {
    // DPB not found, something wrong with configuration...
    return;
  } else {

    // First message in microslice = Epoch Cycle;
    current_epoch_cycle = ms_data[0] & gdpbv100::kulEpochCycleFieldSz;

    // Loop over messages in microslice
    for (unsigned int i = 1;
         i < (ms_size - (ms_size % BYTES_PER_MESSAGE)) / BYTES_PER_MESSAGE;
         ++i) {

      // cast current data to message, doesn't need constructor call
      const gdpbv100::Message* mess =
          reinterpret_cast<const gdpbv100::Message*>(&ms_data[i]);

      switch (mess->getMessageType()) {
      case gdpbv100::MSG_HIT: {
        if (lastEpoch == 0) {

#ifdef TOF_UNPACKER_USE_MISSING_EPOCH_QUIRKS_MODE
          if (i == 1)
            continue;
          // searches remaining messages for epoch information
          for (size_t j = i + 1; j < (ms_size - (ms_size % BYTES_PER_MESSAGE)) /
                                         BYTES_PER_MESSAGE;
               j++) {
            const gdpbv100::Message* tempMessage =
                reinterpret_cast<const gdpbv100::Message*>(&ms_data[j]);
            if (tempMessage->isEpochMsg()) {
              lastEpoch =
                  (tempMessage->getGdpbEpEpochNb() - 1) +
                  (gdpbv100::kuEpochCounterSz + 1) * current_epoch_cycle;
              //  out_ << "Missing EPOCH before message " << i
              //  << " using EPOCH " << lastEpoch << std::endl;
              break;
            }
          }
          if (lastEpoch == 0) {
            // No Epoch in microslice
            // Ignore current message and continue...
            errors++;
            break;
          }
#else
          // Missing Epoch information at start of microslice
          // Ignore current message and continue...
          errors++;
          break;
#endif
        }

        unsigned int detectorAddress =
            it->second[(mess->getGdpbGenChipId() << 2) |
                       mess->getGdpbHitChanId()];

        if (detectorAddress == 0) {
          unmapped_messages++;
          continue; // Hit not mapped to digi
        }

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
      default: {
#ifdef TOF_UNPACKER_PROCESS_INFO_MESSAGES
        // Try handling message by type and forward to error vector:
        if (mess->isGet4SlCtrMsg() ||
            mess->isSysMsg()) // Message with DPB and ASIC info
        {
          // searches previous messages for time information
          for (size_t j = i - 1; j > 0; j--) {
            const gdpbv100::Message* tempMessage =
                reinterpret_cast<const gdpbv100::Message*>(&ms_data[j]);
            if (tempMessage->isEpochMsg()) {
              error_vector.emplace_back(
                  (gdpbv100::kdEpochInNs * static_cast<double>(lastEpoch)),
                  mess);
              break; // No detailed Time information in current epoch
            }
            if (tempMessage->isHitMsg()) {
              if (mess->getGdpbGenChipId() == tempMessage->getGdpbGenChipId()) {
                error_vector.emplace_back(
                    tempMessage->getMsgFullTimeD(lastEpoch), mess);
                break;
              }
            }
          }
        } else { // message without ASIC info -> just EPOCH
          error_vector.emplace_back(
              (gdpbv100::kdEpochInNs * static_cast<double>(lastEpoch)), mess);
        }
#else
        unprocessed_messages++;
#endif
        break;
      }
      }
    }
  }
  return;
}
