#ifndef INCLUDE_APP_COMMON_RANDOM_H_
#define INCLUDE_APP_COMMON_RANDOM_H_


#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/random.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>


namespace app {

int64_t GenerateRandomInt64();

}  // namespace app


#endif  // INCLUDE_APP_COMMON_RANDOM_H_
