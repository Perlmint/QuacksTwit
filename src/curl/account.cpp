#include "account_curl.h"

Quacks::Twit::CurlAccount::CurlAccount(const std::string &key,
                                       const std::string &secret,
                                       std::shared_ptr<FileAccountStore> store)
  : key(key)
  , secret(secret)
  , store(store)
{
}

std::string Quacks::Twit::CurlAccount::username() const
{
  return username_cache;
}

std::string Quacks::Twit::CurlAccount::identifier() const
{
  return identifier_cache;
}
