#include "TofError.hpp"
TofError::TofError(double time, const gdpbv100::Message* data) {
  data_ = data;
  time_ = time;
}

TofError::~TofError() {}

const gdpbv100::Message* TofError::getData() { return data_; }

double TofError::getTime() { return time_; }
