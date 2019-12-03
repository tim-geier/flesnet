// Copyright 2013, 2015 Jan de Cuveland <cmail@cuveland.de>
#pragma once

#include "MicrosliceDescriptor.hpp"
#include "Sink.hpp"
#include "Timeslice.hpp"
//#include "interface.h" // crcutil_interface
#include <memory>
#include <ostream>
#include <string>
// C/C++
#include <map>
#include <vector>

#include "CbmTofDigiExp.hpp"
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
  bool process_timeslice(const fles::Timeslice& ts);

private:
  std::string statistics() const;
  void reset() {
    microslice_count_ = 0;
    content_bytes_ = 0;
  }

  std::vector<CbmTofDigiExp> digiVect;

  uint64_t output_interval_ = UINT64_MAX;
  std::ostream& out_;
  std::string output_prefix_;
  std::ostream* hist_;

  TofUnpacker tofUnpacker;

  size_t timeslice_count_ = 0;
  size_t timeslice_error_count_ = 0;
  size_t microslice_count_ = 0;
  size_t content_bytes_ = 0;
};
