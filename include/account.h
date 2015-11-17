#pragma once

#include <string>
#include <memory>

namespace Quacks
{
  namespace Twit
  {
    class Account
    {
    public:
      virtual std::string username() const = 0;
      virtual std::string identifier() const = 0;
      virtual void endCreateAccount(const std::string &pin) = 0;
    };
  }
}
