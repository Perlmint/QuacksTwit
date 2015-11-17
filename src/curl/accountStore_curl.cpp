#include <cstdio>
#include <algorithm>
#include <sstream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "curl_helper.h"
#include "accountStore.h"
#include "account_curl.h"
#include "oauthlib.h"
#include "McbDES2.hpp"

#ifdef _MSC_VER
#define FOPEN(FP, FILENAME, MODE) fopen_s(&FP, FILENAME, MODE)
#else
#define FOPEN(FP, FILENAME, MODE) FP = fopen(FILENAME, MODE)
#endif

std::map<std::string, std::shared_ptr<Quacks::Twit::FileAccountStore>> Quacks::Twit::FileAccountStore::storeMap;

namespace Quacks
{
  namespace Twit
  {
    class FileAccountStore::Impl
    {
    public:
      Impl(const std::string &filename)
        : storeFilePath(filename)
      {
      }

      Impl(const std::string &filename, const std::string &key, const std::string &secret, const std::string &pass)
        : storeFilePath(filename)
        , storekey(pass)
        , auth(key, secret)
        , unlocked(true)
      {
        write();
      }

      const std::vector< std::shared_ptr<Quacks::Twit::Account> > &getAccounts()
      {
        return accounts;
      }

      bool unlock(std::shared_ptr<FileAccountStore> &store, const std::string &key)
      {
        try
        {
          storekey = key;
          read(store);
          unlocked = true;
        }
        catch (std::exception e)
        {
          return false;
        }

        return true;
      }

    private:
      void read(std::shared_ptr<FileAccountStore> &accountStore)
      {
        store = accountStore;
        FILE *fp = nullptr;
        FOPEN(fp, storeFilePath.c_str(), "rb");

        // File notfound - ignore
        if (fp == nullptr)
        {
          throw std::exception();
        }

        McbDES mcbdes;
        std::unique_ptr<unsigned char[]> keybuffer(new unsigned char[storekey.length()]),
                      key2buffer(new unsigned char[storekey.length()]);

        memcpy(keybuffer.get(), storekey.c_str(), storekey.length());
        memcpy(key2buffer.get(), storekey.c_str(), storekey.length());
        std::reverse(key2buffer.get(), key2buffer.get() + storekey.length());

        mcbdes.McbSetKey1(keybuffer.get());
        mcbdes.McbSetKey2(key2buffer.get());

        fseek(fp, 0, SEEK_END);
        unsigned long size = ftell(fp);
        std::unique_ptr<unsigned char[]> buffer(new unsigned char[size]);
        fseek(fp, 0, SEEK_SET);
        for (unsigned long i = 0; i < size;)
        {
          unsigned long readSize = std::min<unsigned int>(size - i, 256);
          fread(buffer.get() + i, 1, readSize, fp);
          i += readSize;
        }

        if (!mcbdes.McbDecrypt(buffer.get(), size))
        {
          throw std::exception();
        }

        rapidjson::Document doc;
        doc.Parse(reinterpret_cast<const char *>(mcbdes.McbGetPlainText()));
        accounts.clear();
        {
          const auto &accountsInDoc = doc["accounts"];
          auth.setConsumerKey(doc["key"].GetString());
          auth.setConsumerSecret(doc["secret"].GetString());
          auto sharedStore = store.lock();
          for (auto itr = accountsInDoc.Begin(), end = accountsInDoc.End(); itr != end; ++itr)
          {
            const auto &val = *itr;
            auto newAccount = new CurlAccount(val["key"].GetString(), val["secret"].GetString(), sharedStore);
            accounts.push_back(std::shared_ptr<Account>(newAccount));
            newAccount->getAuth().setOAuthScreenName(val["screen_name"].GetString());
          }
        }

        fclose(fp);
      }

      void write()
      {
        FILE *fp = nullptr;
        FOPEN(fp, storeFilePath.c_str(), "wb");

        if (fp == nullptr)
        {
        throw std::exception();
        }

        std::unique_ptr<unsigned char[]> keybuffer(new unsigned char[storekey.length()]),
          key2buffer(new unsigned char[storekey.length()]);

        memcpy(keybuffer.get(), storekey.c_str(), storekey.length());
        memcpy(key2buffer.get(), storekey.c_str(), storekey.length());
        std::reverse(key2buffer.get(), key2buffer.get() + storekey.length());

        McbDES mcbdes;

        mcbdes.McbSetKey1(keybuffer.get());
        mcbdes.McbSetKey2(key2buffer.get());

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        writer.StartObject();
        writer.Key("key");
        writer.String(auth.getConsumerKey().c_str());
        writer.Key("secret");
        writer.String(auth.getConsumerSecret().c_str());
        writer.Key("accounts");
        writer.StartArray();
        for (const auto &account : accounts)
        {
          auto curlAccount = std::dynamic_pointer_cast<CurlAccount>(account);
          writer.StartObject();
          writer.Key("key");
          writer.String(curlAccount->getKey().c_str());
          writer.Key("secret");
          writer.String(curlAccount->getSecret().c_str());
          writer.Key("screen_name");
          writer.String(curlAccount->getAuth().getOAuthScreenName().c_str());
          writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();

        if (!mcbdes.McbEncrypt(buffer.GetString()))
        {
          throw std::exception();
        }
        fwrite(mcbdes.McbGetCryptogram(), 1, mcbdes.McbGetCryptogramSize(), fp);

        fclose(fp);
      }

      std::vector< std::shared_ptr<Quacks::Twit::Account> > accounts;
      const std::string storeFilePath;
      std::string storekey;
      oAuth auth;
      bool unlocked = false;
      std::weak_ptr<FileAccountStore> store;

      friend class FileAccountStore;
    };
  }
}

std::shared_ptr<Quacks::Twit::FileAccountStore> Quacks::Twit::FileAccountStore::GetAccountStore(const std::string &filename)
{
  auto itr = storeMap.find(filename);
  if (itr != storeMap.end())
  {
    return itr->second;
  }

  std::shared_ptr<Quacks::Twit::FileAccountStore> newStore(new FileAccountStore(filename));
  storeMap.insert(std::make_pair(filename, newStore));

  return newStore;
}

std::shared_ptr<Quacks::Twit::FileAccountStore> Quacks::Twit::FileAccountStore::CreateAccountStore(const std::string &filename, const std::string &key, const std::string &secret, const std::string &pass)
{
  std::shared_ptr<Quacks::Twit::FileAccountStore> newStore(new FileAccountStore(filename, key, secret, pass));
  storeMap.insert(std::make_pair(filename, newStore));

  return newStore;
}

std::vector< std::shared_ptr<Quacks::Twit::Account> > Quacks::Twit::FileAccountStore::storedAccounts()
{
  return impl->getAccounts();
}

void Quacks::Twit::FileAccountStore::beginCreateAccount(const CreatingAccountResultCallback &callback)
{
  const std::string request_token_url("https://api.twitter.com/oauth/request_token");
  CurlHelper helper(impl->auth, CurlHelper::RequestType::POST, request_token_url, "");

  auto res = helper.perform();
  if (res == CURLE_OK)
  {
    std::string ret = helper.getData();
    std::string requestToken, requestSecret;
    while (!ret.empty())
    {
      auto pos = ret.find('='), field_end = ret.find('&');
      if (pos == std::string::npos)
      {
        callback(false, ret, nullptr);
        return;
      }
      if (field_end == std::string::npos)
      {
        field_end = ret.length();
      }
      if (strncmp("oauth_token", ret.c_str(), pos) == 0)
      {
        requestToken.assign(ret.c_str() + pos + 1, field_end - pos - 1);
      }
      else if (strncmp("oauth_token_secret", ret.c_str(), pos) == 0)
      {
        requestSecret.assign(ret.c_str() + pos + 1, field_end - pos - 1);
      }
      ret.erase(0, field_end + 1);
    }
    auto newAccount = new CurlAccount(requestToken, requestSecret, shared_from_this());
    impl->accounts.push_back(std::shared_ptr<Account>(newAccount));
    callback(true, "https://api.twitter.com/oauth/authenticate?oauth_token=" + requestToken, newAccount->shared_from_this());
  }
  else
  {
    callback(false, curl_easy_strerror(res), nullptr);
  }
}

Quacks::Twit::FileAccountStore::FileAccountStore(const std::string &filename)
  : impl(new Impl(filename))
{
}

Quacks::Twit::FileAccountStore::FileAccountStore(const std::string &filename, const std::string &key, const std::string &secret, const std::string &pass)
  : impl(new Impl(filename, key, secret, pass))
{
}

bool Quacks::Twit::FileAccountStore::unlock(const std::string &pass)
{
  return impl->unlock(shared_from_this(), pass);
}

bool Quacks::Twit::FileAccountStore::isLocked()
{
  return !impl->unlocked;
}

const void *Quacks::Twit::FileAccountStore::getData() const
{
  return &impl->auth;
}

void Quacks::Twit::FileAccountStore::save()
{
  impl->write();
}