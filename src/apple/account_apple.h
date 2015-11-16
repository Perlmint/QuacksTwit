#pragma once

#include "account.h"

namespace Quacks
{
  namespace Twit
  {
    class AppleAccount : public Account
    {
    public:
      AppleAccount(void *data);
      ~AppleAccount();

      std::string username() const;
      std::string identifier() const;

      void *getAccount() const;
    private:
      void *account;
    };
  }
}
