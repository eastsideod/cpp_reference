#include "include/app/common/random.h"

#include <ctime>


namespace app {

namespace {

typedef boost::uniform_int<> UniformInt;
typedef boost::variate_generator<boost::mt19937 &, UniformInt>
    IntegerRandomGenerater;

}  // unnamed namespace


int64_t GenerateRandomInt64() {
  UniformInt uniform_int;
  boost::mt19937 mt;
  IntegerRandomGenerater generator(mt, uniform_int);
  return generator();
}

}  // namespace app