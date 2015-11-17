#pragma once

#include "account.h"
#include "accountStore.h"
#include <memory>
#include "oauthlib.h"

namespace Quacks
{
  namespace Twit
  {
    class CurlAccount : public Account, public std::enable_shared_from_this<CurlAccount>
    {
    public:
      CurlAccount(const std::string &key, const std::string &secret,
                  std::shared_ptr<FileAccountStore> store);

      std::string username() const;
      std::string identifier() const;
      const std::string &getKey() const
      {
        return auth.getOAuthTokenKey();
      }
      const std::string &getSecret() const
      {
        return auth.getOAuthTokenSecret();
      }
      oAuth &getAuth()
      {
        return auth;
      }
      void endCreateAccount(const std::string &pin);
    private:
      oAuth auth;
      std::string identifier_cache;
      std::weak_ptr<FileAccountStore> store;
    };
  }
}
