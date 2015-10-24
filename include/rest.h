#pragma once

#include <functional>
#include <string>
#include "api.h"

namespace Quacks
{
  namespace Twit
  {
    class Account;
    namespace statuses
    {
      struct home_timeline : APIObject
      {
        void operator() (const std::function<void(std::shared_ptr<Account> account, int code, const std::string &ret)> &callback);
      };
    }
  }
}
