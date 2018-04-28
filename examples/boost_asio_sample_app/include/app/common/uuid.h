#ifndef INCLUDE_APP_COMMON_UUID_H_
#define INCLUDE_APP_COMMON_UUID_H_

#include "common.h"

#include <boost/uuid/uuid.hpp>


namespace app {

class Uuid {
 public:
  static Uuid FromString(const string &uuid_str);
  static Uuid Generate();

  static boost::uuids::uuid kNilUuid;

  Uuid(const boost::uuids::uuid &uuid);

  const bool IsNil() const;
  const string ToString() const;

  const boost::uuids::uuid &raw_uuid() const;

 private:
  boost::uuids::uuid uuid_;
};

}  // namespace app

#endif  // INCLUDE_APP_COMMON_UUID_H_