#include <ostream>
#include <string>
// C/C++
#include <vector>
#include <map>

#include <algorithm>
#include <cassert>
#include <sstream>

#include <iostream>
#include <iomanip>
#include <fstream>

#include "gDpbMessv100.hpp"
#include "CbmTofDigiExp.hpp"

#include<map>

class TofUnpacker
{
private:
    std::ostream& out_;
    std::map <unsigned int, unsigned int> mapping; // Mapping (DPB, ASIC, channel) -> unique address


public:

    TofUnpacker(std::ostream& arg_out);
    ~TofUnpacker();


    bool load_mapping(std::string path);

    void process_microslice(const uint64_t* data, const uint32_t ms_size, std::vector<CbmTofDigiExp> * digiVect);


};