#pragma once

#include <memory>

namespace Quacks
{
  namespace Twit
  {
    class Account;

    struct APIObject
    {
      std::shared_ptr<Account> account;
    };
  }
}
