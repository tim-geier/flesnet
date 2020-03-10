#include "TofError.hpp"
TofError::TofError(double time,const  gdpbv100::Message* data) {
  _data = data;
  _time = time;
}

TofError::~TofError() {}

const gdpbv100::Message* TofError::getData() { return _data; }

double TofError::getTime() { return _time; }

