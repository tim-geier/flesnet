// Copyright 2013, 2015 Jan de Cuveland <cmail@cuveland.de>
#pragma once

#include "MicrosliceDescriptor.hpp"
#include "MicrosliceView.hpp"
#include "Sink.hpp"
#include "Timeslice.hpp"
//#include "interface.h" // crcutil_interface
#include <memory>
#include <ostream>
#include <string>
// C/C++
#include <map>
#include <vector>

#include "TofUnpacker.hpp"
#include "gDpbMessv100.hpp"

class TimesliceUnpacker : public fles::TimesliceSink {
public:
  TimesliceUnpacker(uint64_t arg_output_interval,
                    std::ostream& arg_out,
                    std::string arg_output_prefix,
                    std::ostream* arg_hist);
  ~TimesliceUnpacker() override;

  void put(std::shared_ptr<const fles::Timeslice> timeslice) override;
  void saveTofDigiVectorToDisk();
  std::vector<CbmTofDigiExp> getTofDigiVector() {
    return tof_output_DigiVector_;
  }
  void clearTofDigiVector() {
    tof_output_DigiVector_.clear();
    tof_output_DigiVector_.shrink_to_fit();
  }
  bool process_timeslice(const fles::Timeslice& ts);

  void set_tof_unpacker_mapping_file(std::string);
  void set_tof_unpacker_output_filename(std::string);

  void setTofProcessing(bool value) { unpack_tof_ = value; }
  void setT0Processing(bool value) { unpack_t0_ = value; }

private:
  std::string statistics() const;

  uint64_t output_interval_ = UINT64_MAX;
  std::ostream& out_;
  std::string output_prefix_;
  std::ostream* hist_;

  // Enabled unpacker options
  bool unpack_tof_ = false;
  bool unpack_t0_ = false;

  // Tof Unpacker
  TofUnpacker tofUnpacker_;
  std::string tof_output_filename_;
  std::vector<CbmTofDigiExp> tof_output_DigiVector_;
  double tof_processing_time_s_ = 0;

  size_t timeslice_count_ = 0;
  size_t timeslice_error_count_ = 0;
};
