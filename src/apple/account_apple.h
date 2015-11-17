#pragma once

#include "account.h"

namespace Quacks
{
  namespace Twit
  {
    class AppleAccount : public Account, public std::enable_shared_from_this<AppleAccount>
    {
    public:
      AppleAccount(void *data);
      ~AppleAccount();

      std::string username() const;
      std::string identifier() const;

      void *getAccount() const;
      void endCreateAccount(const std::string &pin) {}
    private:
      void *account;
    };
  }
}
