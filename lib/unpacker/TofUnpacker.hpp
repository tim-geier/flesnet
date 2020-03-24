// Copyright 2019-2020 Tim Geier <mail@tim-geier.de>
#include <ostream>
#include <string>
// C/C++
#include <map>
#include <vector>

#include <algorithm>
#include <cassert>
#include <sstream>

#include <fstream>
#include <iomanip>
#include <iostream>

#include "CbmTofDigiExp.hpp"
#include "MicrosliceView.hpp"
#include "TofError.hpp"
#include "gDpbMessv100.hpp"

#include <map>

class TofUnpacker {
private:
  std::ostream& out_;
  std::map<unsigned short int, std::vector<unsigned int>>
      mapping; // Mapping (DPB, ASIC, channel) -> unique address
  bool mapping_loaded = false;
  unsigned int errors = 0;
  unsigned int unprocessed_messages = 0;
  std::vector<TofError> error_vector;

public:
  TofUnpacker(std::ostream& arg_out);
  ~TofUnpacker();

  bool load_mapping(std::string path);
  void process_microslice(fles::MicrosliceView ms,
                          std::vector<CbmTofDigiExp>* digiVect);

  bool is_mapping_loaded() { return mapping_loaded; };

  unsigned int get_errors();
  unsigned int get_unprocessed_messages();
  bool reset_error_counters();

  std::vector<TofError> get_error_vector();
};