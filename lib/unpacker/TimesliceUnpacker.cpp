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
#define IGNORE_OVERLAP_MICROSLICES

TimesliceUnpacker::TimesliceUnpacker(uint64_t arg_output_interval,
                                     std::ostream& arg_out,
                                     std::string arg_output_prefix,
                                     std::ostream* arg_hist)
    : output_interval_(arg_output_interval), out_(arg_out),
      output_prefix_(std::move(arg_output_prefix)), hist_(arg_hist),
      tofUnpacker(arg_out) {

  tofUnpacker.load_mapping("/home/geier/flesnet/build/mapping.par");
}

TimesliceUnpacker::~TimesliceUnpacker() {}

bool TimesliceUnpacker::process_timeslice(const fles::Timeslice& ts) {
  if (!tofUnpacker.is_mapping_loaded()) {
    out_ << "TofUnpacker not ready, mapping not initialized!" << std::endl;
    out_ << "Not unpacking anything..." << std::endl;
    return false;
  }
  unsigned long int tof_input_data_size = 0;
  unsigned long int tof_output_data_size = 0;
  double tof_processing_time_s = 0;
  std::vector<CbmTofDigiExp> tof_output_DigiVector;

#ifdef IGNORE_OVERLAP_MICROSLICES
  auto overlap_ms = ts.num_microslices(0) - ts.num_core_microslices();
#else
  auto overlap_ms = 0;
#endif

  if (ts.num_components() == 0) {
    out_ << "no component in timeslice " << ts.index() << std::endl;
    ++timeslice_error_count_;
    return false;
  }

  out_ << "Now unpacking TS " << ts.index() << std::endl;

  auto start2 = std::chrono::steady_clock::now();

  // Allocate memory for Digi objects, before inserting objects
  // speedup is huge so extra runtime for counting elements is negligible
  for (size_t c = 0; c < ts.num_components(); ++c) {
    if (!(ts.get_microslice(c, 0).desc().sys_id == 0x60 ||
          ts.get_microslice(c, 0).desc().sys_id == 0x90))
      continue; // Ignore everything not TOF
    for (size_t s = 0; s < ts.num_microslices(c) - overlap_ms; ++s) {
      tof_input_data_size += ts.get_microslice(c, s).desc().size;
    }
  }

  // TODO:
  // calculate better vector size
  // (epoch-cycle messages and EPOCH messages are irrelevant for output)
  tof_output_DigiVector.reserve(tof_input_data_size / 8);

  for (size_t c = 0; c < ts.num_components(); ++c) {
    switch (ts.get_microslice(c, 0).desc().sys_id) {
    case 0x60: // 0x60 = Tof
    {

      for (size_t s = 0; s < ts.num_microslices(c) - overlap_ms; ++s) {
        tofUnpacker.process_microslice(ts.get_microslice(c, s),
                                       &tof_output_DigiVector);
      }

      break;
    }
    case 0x90: // 0x90 = T0, uses Tof unpacker due to similar data format
    {
      for (size_t s = 0; s < ts.num_microslices(c) - overlap_ms; ++s) {
        tofUnpacker.process_microslice(ts.get_microslice(c, s),
                                       &tof_output_DigiVector);
      }

      break;
    }
    default:
      // No unpacker for other detectors yet.
      break;
    }
  }

  auto finish2 = std::chrono::steady_clock::now();
  tof_processing_time_s =
      std::chrono::duration_cast<std::chrono::duration<double>>(finish2 -
                                                                start2)
          .count();

  out_ << "processing took " << tof_processing_time_s << " seconds"
       << std::endl;
  out_ << "Vector size used : " << tof_output_DigiVector.size() << "/"
       << (tof_input_data_size / 8) << std::endl;

  // tof_output_DigiVector.shrink_to_fit();
  // don't shrink, takes O(n) time; forever in case of 1000 TS

  // Using lambda comparison makes sorting faster
  std::sort(tof_output_DigiVector.begin(), tof_output_DigiVector.end(),
            [](const CbmTofDigiExp& a, const CbmTofDigiExp& b) -> bool {
              return a.GetTime() < b.GetTime();
            });

  tof_output_data_size = tof_output_DigiVector.size() *
                         sizeof(decltype(tof_output_DigiVector)::value_type);

  std::ofstream outFile;
  char filename[50];
  sprintf(filename, "ts_%lu%s", ts.index(), TOF_UNPACKER_OUTPUT_FILE_EXTENSION);
  outFile.open(filename);

  {
    boost::archive::binary_oarchive oa(outFile);
    oa& tof_output_DigiVector;
  }

  outFile.close();
  tof_output_DigiVector.clear();
  tof_output_DigiVector.shrink_to_fit();
  timeslice_count_++;

  out_ << "Input size:  " << tof_input_data_size << " bytes." << std::endl;
  out_ << "Output size: " << tof_output_data_size << " bytes." << std::endl;
  out_ << "Input rate:  " << static_cast<unsigned long int>(
                                 tof_input_data_size / tof_processing_time_s)
       << " bytes/second" << std::endl;
  out_ << "Errors: " << tofUnpacker.get_errors() << std::endl;
  out_ << "Unprocessed messages; " << tofUnpacker.get_unprocessed_messages()
       << std::endl;

  tofUnpacker.reset_error_counters();

  return true;
}

std::string TimesliceUnpacker::statistics() const {
  std::stringstream s;
  s << "timeslices unpacked: " << timeslice_count_ << " ( avg"
    /*<< human_readable_count(content_bytes_) << " in " << microslice_count_*/
    << " microslices, avg: "
    << static_cast<double>(content_bytes_) / microslice_count_ << " bytes/ms)";
  if (timeslice_error_count_ > 0) {
    s << " [" << timeslice_error_count_ << " errors]";
  }
  return s.str();
}

void TimesliceUnpacker::put(std::shared_ptr<const fles::Timeslice> timeslice) {
  process_timeslice(*timeslice);

  if ((timeslice_count_ % output_interval_) == 0) {
    out_ << output_prefix_ << statistics() << std::endl;
    reset();
  }
}
