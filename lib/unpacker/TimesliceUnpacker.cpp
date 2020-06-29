// Copyright 2013, 2015 Jan de Cuveland <cmail@cuveland.de>

#include "TimesliceUnpacker.hpp"

#include "CbmTofDigiExp.hpp"
#include "gDpbMessv100.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>

#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#define TOF_UNPACKER_OUTPUT_FILE_EXTENSION ".digi"
#define TOF_UNPACKER_IGNORE_OVERLAP_MICROSLICES
// Vector reserve speedup depends on timeslice size, for small (normal size)
// timeslices vector growth becomes linear and programm gets slower
// -> disabled by default
// #define TOF_UNPACKER_USE_VECTOR_RESERVE

TimesliceUnpacker::TimesliceUnpacker(uint64_t arg_output_interval,
                                     std::ostream& arg_out,
                                     std::string arg_output_prefix,
                                     std::ostream* arg_hist)
    : output_interval_(arg_output_interval), out_(arg_out),
      output_prefix_(std::move(arg_output_prefix)), hist_(arg_hist),
      tofUnpacker_(arg_out) {}

TimesliceUnpacker::~TimesliceUnpacker() {}

void TimesliceUnpacker::set_tof_unpacker_mapping_file(
    std::string mapping_file) {
  tofUnpacker_.load_mapping(mapping_file);
}

void TimesliceUnpacker::set_tof_unpacker_output_filename(
    std::string tof_output_filename) {
  tof_output_filename_ = tof_output_filename;
}

bool TimesliceUnpacker::process_timeslice(const fles::Timeslice& ts) {
  if (!tofUnpacker_.is_mapping_loaded()) {
    out_ << "TofUnpacker not ready, mapping not initialized!" << std::endl;
    out_ << "Not unpacking anything..." << std::endl;
    return false;
  }

  // unsigned long int tof_output_data_size = 0;

#ifdef TOF_UNPACKER_IGNORE_OVERLAP_MICROSLICES
  auto overlap_ms = ts.num_microslices(0) - ts.num_core_microslices();
#else
  auto overlap_ms = 0;
#endif

  if (ts.num_components() == 0) {
    out_ << "no component in timeslice " << ts.index() << std::endl;
    ++timeslice_error_count_;
    return false;
  }

  // out_ << "Now unpacking TS " << ts.index() << std::endl;

  auto start2 = std::chrono::steady_clock::now();

#ifdef TOF_UNPACKER_USE_VECTOR_RESERVE
  unsigned long int tof_input_data_size = 0;
  // Allocate memory for Digi objects, before inserting objects
  // speedup is huge so extra runtime for counting elements is negligible
  for (size_t c = 0; c < ts.num_components(); ++c) {
    if (!(ts.get_microslice(c, 0).desc().sys_id ==
              static_cast<int>(fles::SubsystemIdentifier::RPC) ||
          ts.get_microslice(c, 0).desc().sys_id ==
              static_cast<int>(fles::SubsystemIdentifier::T0)))
      continue; // Ignore everything not TOF
    for (size_t s = 0; s < ts.num_microslices(c) - overlap_ms; ++s) {
      tof_input_data_size += ts.get_microslice(c, s).desc().size;
    }
  }

  // TODO:
  // calculate better vector size
  // (epoch-cycle messages and EPOCH messages are irrelevant for output)
  tof_output_DigiVector_.reserve(tof_output_DigiVector_.capacity() +
                                 (tof_input_data_size >> 3));
#endif

  for (size_t c = 0; c < ts.num_components(); ++c) {
    switch (ts.get_microslice(c, 0).desc().sys_id) {
    case static_cast<int>(fles::SubsystemIdentifier::RPC): // 0x60 = Tof
    {
      if (unpack_tof_) {
        for (size_t s = 0; s < ts.num_microslices(c) - overlap_ms; ++s) {
          tofUnpacker_.process_microslice(ts.get_microslice(c, s),
                                          &tof_output_DigiVector_);
        }
      }
      break;
    }
    case static_cast<int>(
        fles::SubsystemIdentifier::T0): // 0x90 = T0, uses Tof unpacker due to
                                        // similar data format
    {
      if (unpack_t0_) {
        for (size_t s = 0; s < ts.num_microslices(c) - overlap_ms; ++s) {
          tofUnpacker_.process_microslice(ts.get_microslice(c, s),
                                          &tof_output_DigiVector_);
        }
      }
      break;
    }
    default:
      // No unpacker for other detectors yet.
      break;
    }
  }

  auto finish2 = std::chrono::steady_clock::now();
  tof_processing_time_s_ +=
      std::chrono::duration_cast<std::chrono::duration<double>>(finish2 -
                                                                start2)
          .count();

  // out_ << "processing took " << tof_processing_time_s << " seconds"
  //     << std::endl;
  // out_ << "Vector size used : " << tof_output_DigiVector_.size() << "/"
  //     << (tof_input_data_size / 8) << std::endl;

  // tof_output_DigiVector_.shrink_to_fit();
  // don't shrink, takes O(n) time; forever in case of 1000 TS

  // tof_output_data_size = tof_output_DigiVector_.size() *
  //                       sizeof(decltype(tof_output_DigiVector_)::value_type);

  timeslice_count_++;

  // out_ << "Input size:  " << tof_input_data_size << " bytes." << std::endl;
  // out_ << "Output size: " << tof_output_data_size << " bytes." << std::endl;
  // out_ << "Input rate:  "
  //     << static_cast<unsigned long int>(tof_input_data_size /
  //                                       tof_processing_time_s)
  //     << " bytes/second" << std::endl;

  return true;
}

std::string TimesliceUnpacker::statistics() const {
  std::stringstream s;
  s << "timeslices unpacked: " << timeslice_count_;
  if (timeslice_error_count_ > 0) {
    s << " [" << timeslice_error_count_ << " errors]";
  }
  return s.str();
}

void TimesliceUnpacker::put(std::shared_ptr<const fles::Timeslice> timeslice) {
  process_timeslice(*timeslice);

  if ((timeslice_count_ % output_interval_) == 0) {
    out_ << output_prefix_ << statistics() << std::endl;
  }
}

void TimesliceUnpacker::saveTofDigiVectorToDisk() {
  out_ << "processing took " << tof_processing_time_s_ << " seconds"
       << std::endl;
  tof_processing_time_s_ = 0;

  out_ << "sorting and writing to disk..." << std::endl;
  // tof_output_DigiVector_.shrink_to_fit();
  // Using lambda comparison makes sorting faster
  std::sort(tof_output_DigiVector_.begin(), tof_output_DigiVector_.end(),
            [](const CbmTofDigiExp& a, const CbmTofDigiExp& b) -> bool {
              return a.GetTime() < b.GetTime();
            });

  std::ofstream outFile;
  std::string filename =
      tof_output_filename_ + TOF_UNPACKER_OUTPUT_FILE_EXTENSION;
  outFile.open(filename);
  if (outFile.is_open()) {
    boost::archive::binary_oarchive oa(outFile);
    oa& tof_output_DigiVector_;
  } else {
    out_ << "ERROR: Cannot write ouput file " << filename << std::endl;
  }
  outFile.close();

  out_ << "Successfully generated " << tof_output_DigiVector_.size()
       << " CbmTofDigis" << std::endl;
  out_ << "Errors: " << tofUnpacker_.get_errors() << std::endl;
  out_ << "Unprocessed (info/debug) messages: "
       << tofUnpacker_.get_unprocessed_messages() << std::endl;
  out_ << "Unmapped messages: " << tofUnpacker_.get_unmapped_messages()
       << std::endl;

  tof_output_DigiVector_.clear();
  tof_output_DigiVector_.shrink_to_fit();
  tofUnpacker_.reset_error_counters();
}
