#include <iostream>

#include "include/app/common/uuid.h"


int main() {

  app::Uuid uuid = app::Uuid::Generate();

  std::cout << uuid.ToString();
}