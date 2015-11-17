#pragma once

#include <string>
#include <memory>

namespace Quacks
{
  namespace Twit
  {
    class Account : public std::enable_shared_from_this<Account>
    {
    public:
      virtual std::string username() const = 0;
      virtual std::string identifier() const = 0;
      virtual void endCreateAccount(const std::string &pin) = 0;
    };
  }
}
