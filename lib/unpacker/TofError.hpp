#include "gDpbMessv100.hpp"

class TofError {
private:
  const gdpbv100::Message* data_ = 0;
  double time_ = 0.0;

public:
  TofError(double time, const gdpbv100::Message* data);
  ~TofError();
  const gdpbv100::Message* getData();
  double getTime();
};
