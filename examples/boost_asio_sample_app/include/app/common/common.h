#ifndef INCLUDE_APP_COMMON_COMMON_H_
#define INCLUDE_APP_COMMON_COMMON_H_

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <string>

#define DEBUG

#ifdef DEBUG
  #define DLOG(MSG) std::cout << MSG << std::endl
#else
  #define DLOG(MSG)
#endif


namespace app {

using boost::bind;
using boost::function;
using boost::make_shared;
using boost::shared_ptr;

using std::string;

}  // namespace app


#endif  // INCLUDE_APP_COMMON_COMMON_H_
