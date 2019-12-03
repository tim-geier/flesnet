// Copyright 2013, 2015 Jan de Cuveland <cmail@cuveland.de>

#include "TimesliceUnpacker.hpp"

#include "CbmTofDigiExp.hpp"
#include "gDpbMessv100.hpp"

#include <algorithm>
#include <sstream>
#include <chrono>

#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

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
  unsigned int input_data_size = 0;
  unsigned int output_data_size = 0;
  double processing_time_s = 0;

  if (ts.num_components() == 0) {
    out_ << "no component in timeslice " << ts.index() << std::endl;
    ++timeslice_error_count_;
    return false;
  }

  out_ << "Now unpacking TS " << ts.index() << std::endl;

  //#pragma omp parallel
  {

    std::vector<CbmTofDigiExp>
        tmpDigiVect; // private Vector for each Process/Thread
    auto start2 = std::chrono::steady_clock::now();
#pragma omp for nowait
    for (size_t c = 0; c < ts.num_components(); ++c) {
      if (ts.get_microslice(c, 0).desc().sys_id != 0x60)
        continue; // Ignore everything not TOF

      for (size_t s = 0; s < ts.num_microslices(c); ++s) {
        // Process MS

        tofUnpacker.process_microslice(reinterpret_cast<const uint64_t*>(
                                           ts.get_microslice(c, s).content()),
                                       ts.get_microslice(c, s).desc().size,
                                       &tmpDigiVect);
        input_data_size += ts.get_microslice(c, s).desc().size;
      }
    }
    auto finish2 = std::chrono::steady_clock::now();
    processing_time_s =
        std::chrono::duration_cast<std::chrono::duration<double>>(finish2 -
                                                                  start2)
            .count();

#pragma omp critical
    {
      out_ << "processing took " << processing_time_s << " seconds"
           << std::endl;
      if (!tmpDigiVect.empty()) {
        digiVect.insert(digiVect.end(),
                        std::make_move_iterator(tmpDigiVect.begin()),
                        std::make_move_iterator(tmpDigiVect.end()));

        tmpDigiVect.clear();
      }
    }

  } // end of parallel region

  // std::sort(digiVect.begin(), digiVect.end());

  output_data_size = digiVect.size() * sizeof(decltype(digiVect)::value_type);

  std::ofstream outFile;
  char filename[50];
  sprintf(filename, "ts_%lu.txt", ts.index());
  outFile.open(filename);
  {
    boost::archive::binary_oarchive oa(outFile);
    // oa& digiVect;
  }
  outFile.close();
  digiVect.clear();
  timeslice_count_++;

  out_ << "Input size:  " << input_data_size << " bytes." << std::endl;
  out_ << "Output size: " << output_data_size << " bytes." << std::endl;
  out_ << "Input rate:  "
       << static_cast<int>(input_data_size / processing_time_s)
       << " bytes/second" << std::endl;
  out_ << "Output rate: "
       << static_cast<int>(output_data_size / processing_time_s)
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
