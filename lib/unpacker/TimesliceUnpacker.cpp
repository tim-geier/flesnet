// Copyright 2013, 2015 Jan de Cuveland <cmail@cuveland.de>

#include "TimesliceUnpacker.hpp"

#include "CbmTofDigiExp.hpp"
#include "gDpbMessv100.hpp"

#include <algorithm>
#include <cassert>
#include <sstream>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>

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

  if (ts.num_components() == 0) {
    out_ << "no component in timeslice " << ts.index() << std::endl;
    ++timeslice_error_count_;
    return false;
  }

  out_ << "Now unpacking TS " << ts.index() << std::endl;

  //#pragma omp parallel
  {
    auto start2 = std::chrono::steady_clock::now();
    std::vector<CbmTofDigiExp>
        tmpDigiVect; // private Vector for each Process/Thread
#pragma omp for nowait
    for (size_t c = 0; c < ts.num_components(); ++c) {
      if (ts.get_microslice(c, 0).desc().sys_id != 0x60)
        continue; // Ignore everything not TOF

      for (size_t s = 0; s < ts.num_microslices(c); ++s) {
        // Process MS
        const uint64_t* data = reinterpret_cast<const uint64_t*>(
            ts.get_microslice(c, s).content());
        tofUnpacker.process_microslice(
            data, ts.get_microslice(c, s).desc().size, &tmpDigiVect);
      }
    }
    auto finish2 = std::chrono::steady_clock::now();
    auto elapsed_seconds2 =
        std::chrono::duration_cast<std::chrono::duration<double>>(finish2 -
                                                                  start2)
            .count();
    out_ << "processing took " << elapsed_seconds2 << " seconds" << std::endl;

#pragma omp critical
    {
      if (!tmpDigiVect.empty()) {
        auto start = std::chrono::steady_clock::now();
        //    Merge slave into master
        //    use move iterators instead, avoid copy unless
        //    you want to use it for something else after this section
        digiVect.insert(digiVect.end(),
                        std::make_move_iterator(tmpDigiVect.begin()),
                        std::make_move_iterator(tmpDigiVect.end()));

        tmpDigiVect.clear();
        auto finish = std::chrono::steady_clock::now();
        auto elapsed_seconds =
            std::chrono::duration_cast<std::chrono::duration<double>>(finish -
                                                                      start)
                .count();
        out_ << "merging took " << elapsed_seconds << " seconds." << std::endl;
      }
    }

  } // end of parallel region

  auto start = std::chrono::steady_clock::now();
  std::sort(digiVect.begin(), digiVect.end());
  auto finish = std::chrono::steady_clock::now();
  double elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(finish - start)
          .count();
  out_ << "sorting took " << elapsed_seconds << " seconds." << std::endl;

  start = std::chrono::steady_clock::now();
  std::ofstream outFile;
  char filename[50];
  sprintf(filename, "ts_%lu.txt", ts.index());
  outFile.open(filename);

  { // Boost version
    boost::archive::binary_oarchive oa(outFile);
    oa& digiVect;
  }
  /*
    { // normal string representation
      for (auto digi = digiVect.begin(); digi != digiVect.end(); ++digi)
      {
        outFile << digi->ToString() << "\n";
      }
    }
  */
  outFile.close();
  digiVect.clear();
  timeslice_count_++;

  finish = std::chrono::steady_clock::now();
  elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(finish - start)
          .count();
  out_ << "Writing file took " << elapsed_seconds << " seconds." << std::endl;

#ifdef DEBUG
  out_ << "Dumping digiVect:" << std::endl;
  for (unsigned int i = 0; i < digiVect.size(); i++) {
    out_ << digiVect[i].ToString() << std::endl;
  }
#endif // DEBUG
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
