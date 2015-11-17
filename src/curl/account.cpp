#include "account_curl.h"
#include "curl_helper.h"

Quacks::Twit::CurlAccount::CurlAccount(const std::string &key,
                                       const std::string &secret,
                                       std::shared_ptr<FileAccountStore> store)
  : auth(*static_cast<const oAuth *>(store->getData()), key, secret)
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

struct Reciever
{
  static int recieve(char* data, size_t size, size_t nmemb, Reciever* reciever)
  {
    reciever->ss << data;
    return size * nmemb;
  }

  std::stringstream ss;
};

void Quacks::Twit::CurlAccount::endCreateAccount(const std::string &pin)
{
  auth.setOAuthPin(pin);
  const std::string accessTokenUrl = "https://api.twitter.com/oauth/access_token";
  CurlHelper helper(auth, CurlHelper::RequestType::POST, accessTokenUrl, "", true);
  if (helper.perform() == CURLE_OK)
  {
    std::string ret = helper.getData();
    std::string requestToken, requestSecret;
    while (!ret.empty())
    {
      auto pos = ret.find('='), field_end = ret.find('&');
      if (pos == std::string::npos)
      {
        return;
      }
      if (field_end == std::string::npos)
      {
        field_end = ret.length();
      }
      if (strncmp("oauth_token", ret.c_str(), pos) == 0)
      {
        auth.setOAuthTokenKey(std::string(ret.c_str() + pos + 1, field_end - pos - 1));
      }
      else if (strncmp("oauth_token_secret", ret.c_str(), pos) == 0)
      {
        auth.setOAuthTokenSecret(std::string(ret.c_str() + pos + 1, field_end - pos - 1));
      }
      else if (strncmp("screen_name", ret.c_str(), pos) == 0)
      {
        auth.setOAuthScreenName(std::string(ret.c_str() + pos + 1, field_end - pos - 1));
      }
      ret.erase(0, field_end + 1);
    }
  }
  store.lock()->save();
}
