#include "gDpbMessv100.hpp"

class TofError {
private:
  const gdpbv100::Message* _data = 0;
  double _time = 0.0;

public:
  TofError(double time, const gdpbv100::Message* data);
  ~TofError();
  const gdpbv100::Message* getData();
  double getTime();
};
