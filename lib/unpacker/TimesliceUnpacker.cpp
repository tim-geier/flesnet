// Copyright 2013, 2015 Jan de Cuveland <cmail@cuveland.de>

#include "TimesliceUnpacker.hpp"
//#include "PatternChecker.hpp"
//#include "TimesliceDebugger.hpp"
//#include "Utility.hpp"
#include "gDpbMessv100.hpp"
#include "CbmTofDigiExp.hpp"
//#include "Parameter.hpp"
#include <algorithm>
#include <cassert>
#include <sstream>

#include<iostream>
#include<iomanip>

#include<map>

//#define DEBUG
//#define DEBUG_ALL
//#define LOG_MISSING_EPOCH

#ifdef DEBUG_ALL
#ifndef DEBUG
#define DEBUG
#endif
#endif

uint32_t getField(const uint64_t* data, uint32_t shift, uint32_t len)
  { return (*data >> shift) & (((static_cast<uint64_t>(1)) << len) - 1); }

TimesliceUnpacker::TimesliceUnpacker(uint64_t arg_output_interval,
                                     std::ostream& arg_out,
                                     std::string arg_output_prefix,
                                     std::ostream* arg_hist)
    : output_interval_(arg_output_interval), out_(arg_out),
      output_prefix_(std::move(arg_output_prefix)), hist_(arg_hist)
    {

      out_ << "Reading mapping.par" << std::endl;
      std::ifstream paramFile;
      paramFile.open("mapping.par");
      unsigned int key;
      unsigned int value;
      while ( paramFile >> key >> value ) {
        mapping[key] = value; // input them into the map 
#ifdef DEBUG_ALL
        out_ << "Put " << key << ":" << value << " in Map\n";
#endif
      }
      paramFile.close();
      out_ << "Finished reading mapping.par" << std::endl;
}

TimesliceUnpacker::~TimesliceUnpacker() {
}

bool TimesliceUnpacker::check_timeslice(const fles::Timeslice& ts) {

  if (ts.num_components() == 0) {
    out_ << "no component in timeslice " << ts.index() << std::endl;
    ++timeslice_error_count_;
    return false;
  }

  out_ << "Now unpacking TS " << ts.index() << std::endl;

  for (size_t c = 0; c < ts.num_components(); ++c) {
    if (ts.index() == 0) continue;
    if (ts.get_microslice(c, 0).desc().sys_id != 0x60) continue; // Ignore everything not TOF
    for (size_t s = 0; s < ts.num_microslices(c); ++s) {
       //Process MS

      const uint64_t* data = reinterpret_cast<const uint64_t*>(ts.get_microslice(c, s).content());
      auto ms_descriptor = ts.get_microslice(c, s).desc();
      const uint32_t ms_size = ms_descriptor.size;

       
#ifdef DEBUG
      out_ << "Microslice (" << c <<  ", " << s << ") ";
      out_ << "Sys-ID: " << std::hex << ms_descriptor.sys_id;
      out_ << " Time: " << (ts.get_microslice(c, s).desc().idx);
      out_ << " Data address: " << static_cast<void const*>(ts.get_microslice(c, s).content()) << " ";
      out_ << "Data size: " << std::dec << ms_size << " Bytes" << std::endl ;
#endif //DEBUG

       
        int counter_ = 0;
        uint64_t lastEpoch = 0;
        unsigned int msg_count = ( ms_size - ( ms_size % 8 ) ) / 8;
        for (size_t i = 0; i < msg_count; i++)
        {
          // Loop over messages in microslice
          if(i == 0) //First MS = Epoch Cycle; ignored at the moment
          {
            current_epoch_cycle = data[i] & gdpbv100::kulEpochCycleFieldSz;
          }
          
          gdpbv100::Message mess(data[i]);
          gdpbv100::MessageTypes mtMsgTyp = static_cast<gdpbv100::MessageTypes>(mess.getMessageType());

          //out_ << "Datensatz: " << i << " Typ: " << static_cast<int>(mtMsgTyp) << " Daten: " /*<< std::hex << data[i] << std::dec */ << std::endl ;
          switch (mtMsgTyp) {
            case gdpbv100::MSG_HIT:
            {
              if (lastEpoch == 0) 
              {
#ifdef LOG_MISSING_EPOCH
                out_ << "Missing EPOCH in TS " << ts.index() << " MS " << s << " component " << c << " message " << i << " ignoring HIT..." << std::endl;
#endif //LOG_MISSING_EPOCH
                timeslice_error_count_ ++;
                //break;

              }

#ifdef DEBUG_ALL
              out_ << "HIT: " << std::endl; 
              out_ << "\t MAC: \t" << std::hex << mess.getGdpbGenGdpbId()  << std::dec << std::endl;
              out_ << "\t ASIC: \t" << mess.getGdpbGenChipId() << std::endl;
              if (mess.getGdpbHitIs24b() == 0) 
              {
                out_ << "\t TOT: \t" << mess.getGdpbHit32Tot() << std::endl;
              } else {
                out_ << "\t TOT: \t Hit 24b - not implemented" << std::endl;
                timeslice_error_count_ ++;
              }
              out_ << "\t Time (ns): \t" << mess.getMsgFullTime(lastEpoch) << std::endl;
#endif //DEBUG_ALL

              unsigned int detectorAddress = (mess.getGdpbGenGdpbId() << 12) + (mess.getGdpbGenChipId() << 4) + mess.getGdpbHitChanId();
              auto it = mapping.find(detectorAddress);
              if (it == mapping.end())
              {
                // Hit not mapped to digi
                continue;
              }
              CbmTofDigiExp *digi = new CbmTofDigiExp(it->second, mess.getMsgFullTimeD(lastEpoch), mess.getGdpbHit32Tot());
              digiVect.push_back(*digi);
              counter_++;
#ifdef DEBUG
              out_ << "New digi: " << digi->ToString() << std::endl;
#endif //DEBUG
              
              break;
            }
            case gdpbv100::MSG_EPOCH:
            {
              lastEpoch = mess.getGdpbEpEpochNb() + ( gdpbv100::kuEpochCounterSz + 1 ) * current_epoch_cycle;
#ifdef DEBUG
              out_ << "EPOCH: " << lastEpoch << " Typ: " << (mess.getGdpbGenChipId()==255?"Merged":"Unsupported type!") << std::endl;
#endif //DEBUG
              break;
            }
            case gdpbv100::MSG_SYST:
            {
              #ifdef DEBUG
              out_ << "SYST Message" << std::endl;
              #endif //DEBUG
              break;
            }
            default:
            {
              #ifdef DEBUG
                out_ << "Unbekannter Typ: " << mtMsgTyp << std::endl;
              #endif //DEBUG
            }
          }
         
        }
        out_ << "Processed " << counter_ << " Datapackages in MS " << s << " in component " << c << std::endl;
  }}
  std::ofstream outFile;
  outFile.open ("ts_out.txt");
  std::stable_sort(digiVect.begin(), digiVect.end());
  for (auto digi = digiVect.begin(); digi != digiVect.end(); ++digi)
  {  
    outFile << digi->ToString() << "\n";
  }
  outFile.close();
  timeslice_count_ ++;

#ifdef DEBUG
  out_ << "Dumping digiVect:" << std::endl;
  for (unsigned int i = 0; i < digiVect.size() ; i++)
  {
    out_ << digiVect[i].ToString() << std::endl;
  }
#endif //DEBUG
  return true ;
}

std::string TimesliceUnpacker::statistics() const {
  std::stringstream s;
  s << "timeslices unpacked: " << timeslice_count_ << " ("
    /*<< human_readable_count(content_bytes_) << " in " << microslice_count_*/
    << " microslices, avg: "
    << static_cast<double>(content_bytes_) / microslice_count_ << ")";
  if (timeslice_error_count_ > 0) {
    s << " [" << timeslice_error_count_ << " errors]";
  }
  return s.str();
}

void TimesliceUnpacker::put(std::shared_ptr<const fles::Timeslice> timeslice) {
  check_timeslice(*timeslice);
  if ((timeslice_count_ % output_interval_) == 0) {
    out_ << output_prefix_ << statistics() << std::endl;
    reset();
  }
}

