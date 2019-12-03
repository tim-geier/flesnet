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

#include <map>
#include <unordered_map>

class TofUnpacker {
private:
  std::ostream& out_;
  std::unordered_map<unsigned int, unsigned int>
      mapping; // Mapping (DPB, ASIC, channel) -> unique address
               // using unordered_map gives better performance

public:
  TofUnpacker(std::ostream& arg_out);
  ~TofUnpacker();

  bool load_mapping(std::string path);

  void process_microslice(const uint64_t* data,
                          const uint32_t ms_size,
                          std::vector<CbmTofDigiExp>* digiVect);
};