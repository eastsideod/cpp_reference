#include "include/app/common/uuid.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>


namespace app {

Uuid Uuid::FromString(const string &uuid_str) {
  boost::uuids::string_generator gen;
  return Uuid(gen(uuid_str));
}


// TODO: use TLS uuid pool.
Uuid Uuid::Generate() {
  boost::uuids::random_generator gen;
  return Uuid(gen());
}


Uuid::Uuid(const boost::uuids::uuid &uuid) {
  uuid_ = uuid;
}


const bool Uuid::IsNil() const {
  return uuid_ == Uuid::kNilUuid;
}


const string Uuid::ToString() const {
  // TODO: try catch bad_cast
  return boost::lexical_cast<string>(uuid_);
}


const boost::uuids::uuid &Uuid::raw_uuid() const {
  return uuid_;
}


boost::uuids::uuid Uuid::kNilUuid = boost::uuids::nil_uuid();

}  // namespace app