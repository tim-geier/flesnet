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

#define OUTPUT_FILE_EXTENSION ".digi"
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
  unsigned long int tof_input_data_size = 0;
  unsigned long int output_data_size = 0;
  double processing_time_s = 0;
  std::vector<CbmTofDigiExp> digiVect;

#ifdef IGNORE_OVERLAP_MICROSLICES
  auto overlapms = ts.num_microslices(0) - ts.num_core_microslices();
#else
  auto overlapms = 0;
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
    if (ts.get_microslice(c, 0).desc().sys_id != 0x60)
      continue; // Ignore everything not TOF
    for (size_t s = 0; s < ts.num_microslices(c) - overlapms; ++s) {
      tof_input_data_size += ts.get_microslice(c, s).desc().size;
    }
  }

  digiVect.reserve(tof_input_data_size / 8);

  for (size_t c = 0; c < ts.num_components(); ++c) {
    if (ts.get_microslice(c, 0).desc().sys_id != 0x60)
      continue; // Ignore everything not TOF

    for (size_t s = 0; s < ts.num_microslices(c) - overlapms; ++s) {
      // Process MS
      tofUnpacker.process_microslice(ts.get_microslice(c, s), &digiVect);
    }
    out_ << "Unpacked " << ts.num_microslices(c) - overlapms
         << " microslices..." << std::endl;
  }

  // digiVect.shrink_to_fit();
  // don't shrink, takes O(n) time; forever in case of 1000 TS

  auto finish2 = std::chrono::steady_clock::now();
  processing_time_s = std::chrono::duration_cast<std::chrono::duration<double>>(
                          finish2 - start2)
                          .count();

  out_ << "processing took " << processing_time_s << " seconds" << std::endl;
  out_ << "Vector size allocated : " << tof_input_data_size / 8 << std::endl;
  out_ << "Vector size used : " << digiVect.size() << std::endl;

  out_ << "sorting..." << std::endl;

  // Using lambda comparison makes sorting way faster
  std::sort(digiVect.begin(), digiVect.end(),
            [](const CbmTofDigiExp& a, const CbmTofDigiExp& b) -> bool {
              return a.GetTime() < b.GetTime();
            });

  output_data_size = digiVect.size() * sizeof(decltype(digiVect)::value_type);
  out_ << "writing to disk..." << std::endl;
  std::ofstream outFile;
  char filename[50];
  sprintf(filename, "ts_%lu%s", ts.index(), OUTPUT_FILE_EXTENSION);
  outFile.open(filename);

  {
    boost::archive::binary_oarchive oa(outFile);
    oa& digiVect;
  }
  /*
    { // normal string representation
      for (auto digi = digiVect.begin(); digi != digiVect.end(); ++digi)
    { outFile << digi->ToString() << "\n";
      }
    }
  */
  outFile.close();
  digiVect.clear();
  digiVect.shrink_to_fit();
  timeslice_count_++;

  out_ << "Input size:  " << tof_input_data_size << " bytes." << std::endl;
  out_ << "Output size: " << output_data_size << " bytes." << std::endl;
  out_ << "Input rate:  " << static_cast<unsigned long int>(
                                 tof_input_data_size / processing_time_s)
       << " bytes/second" << std::endl;

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
