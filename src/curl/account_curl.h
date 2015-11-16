#pragma once

#include "account.h"
#include "accountStore.h"
#include <memory>

namespace Quacks
{
  namespace Twit
  {
    class CurlAccount : public Account
    {
    public:
      CurlAccount(const std::string &key, const std::string &secret,
                  std::shared_ptr<FileAccountStore> store);

      std::string username() const;
      std::string identifier() const;
    private:
      const std::string key;
      const std::string secret;
      std::string username_cache;
      std::string identifier_cache;
      std::weak_ptr<FileAccountStore> store;
    };
  }
}
