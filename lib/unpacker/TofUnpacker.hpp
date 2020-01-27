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
#include "gDpbMessv100.hpp"
#include "MicrosliceView.hpp"

#include <map>

class TofUnpacker {
private:
  std::ostream& out_;
  std::map<unsigned short int, std::vector<unsigned int>>
      mapping; // Mapping (DPB, ASIC, channel) -> unique address

public:
  TofUnpacker(std::ostream& arg_out);
  ~TofUnpacker();

  bool load_mapping(std::string path);
  void process_microslice(fles::MicrosliceView ms,
                                     std::vector<CbmTofDigiExp>* digiVect);
};