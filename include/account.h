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
      Account(void *data);
      ~Account();
      ///
      /// @brief Get account data
      /// It differs with backend
      ///
      void *getData() const;
      std::string username() const;
      std::string identifier() const;

    private:
      class Impl;
      std::unique_ptr<Impl> impl;
    };
  }
}
