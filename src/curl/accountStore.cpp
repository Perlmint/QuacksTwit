#include <cstdio>
#include <algorithm>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "accountStore.h"
#include "curl/account_curl.h"
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

      Impl(const std::string &filename, const std::string &key, const std::string &secret)
        : storeFilePath(filename)
        , key(key)
        , secret(secret)
      {
      }

      const std::vector< std::shared_ptr<Quacks::Twit::Account> > &getAccounts()
      {
        return accounts;
      }

      void unlock(const std::string &key)
      {
        this->storekey = key;
        read();
      }
    private:
      void read()
      {
        FILE *fp = nullptr;
        FOPEN(fp, storeFilePath.c_str(), "r");

        // File notfound - ignore
        if (fp == nullptr)
        {
          return;
        }

        McbDES mcbdes;
        std::unique_ptr<unsigned char[]> keybuffer(new unsigned char[storekey.length()]),
                      key2buffer(new unsigned char[storekey.length()]);

        memcpy(keybuffer.get(), storekey.c_str(), storekey.length());
        memcpy(key2buffer.get(), storekey.c_str(), storekey.length());
        std::reverse(key2buffer.get(), key2buffer.get() + storekey.length());

        mcbdes.McbSetKey1(keybuffer.get());
        mcbdes.McbSetKey2(key2buffer.get());

        fseek(fp, SEEK_END, 0);
        unsigned long size = ftell(fp);
        std::unique_ptr<unsigned char[]> buffer(new unsigned char[size]);
        fseek(fp, SEEK_SET, 0);
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
        doc.Parse(reinterpret_cast<char *>(buffer.get()));
        accounts.clear();
        {
            auto sharedStore = store.lock();
            for (auto itr = doc.Begin(), end = doc.End(); itr != end; ++itr)
            {
                const auto &val = *itr;
                accounts.push_back(
                    (new CurlAccount(val["key"].GetString(), val["secret"].GetString(), sharedStore))->shared_from_this());
            }
        }

        fclose(fp);
      }

      void write()
      {
        FILE *fp = nullptr;
        FOPEN(fp, storeFilePath.c_str(), "w");

        if (fp == nullptr)
        {
        throw std::exception();
        }

        std::unique_ptr<unsigned char[]> keybuffer(new unsigned char[key.length()]),
          key2buffer(new unsigned char[storekey.length()]);

        memcpy(keybuffer.get(), storekey.c_str(), storekey.length());
        memcpy(key2buffer.get(), storekey.c_str(), storekey.length());
        std::reverse(key2buffer.get(), key2buffer.get() + storekey.length());

        McbDES mcbdes;

        mcbdes.McbSetKey1(keybuffer.get());
        mcbdes.McbSetKey2(key2buffer.get());

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        writer.StartArray();
        for (const auto &account : accounts)
        {
            auto curlAccount = std::dynamic_pointer_cast<CurlAccount>(account);
            writer.StartObject();
            writer.Key("key");
            writer.String(curlAccount->getKey().c_str());
            writer.Key("secret");
            writer.String(curlAccount->getSecret().c_str());
            writer.EndObject();
        }
        writer.EndArray();

        fclose(fp);
      }

      std::vector< std::shared_ptr<Quacks::Twit::Account> > accounts;
      const std::string storeFilePath;
      std::string storekey;
      std::string key;
      std::string secret;
      bool unlocked = false;
      std::weak_ptr<FileAccountStore> store;

      friend class FileAccountStore;
    };
  }
}

Quacks::Twit::FileAccountStore &Quacks::Twit::FileAccountStore::GetAccountStore(const std::string &filename)
{
  auto itr = storeMap.find(filename);
  if (itr != storeMap.end())
  {
    return *itr->second.get();
  }

  std::shared_ptr<Quacks::Twit::FileAccountStore> newStore(new FileAccountStore(filename));
  storeMap.insert(std::make_pair(filename, newStore));

  return *newStore;
}

Quacks::Twit::FileAccountStore &Quacks::Twit::FileAccountStore::CreateAccountStore(const std::string &filename, const std::string &key, const std::string &secret, const std::string &pass)
{
  std::shared_ptr<Quacks::Twit::FileAccountStore> newStore(new FileAccountStore(filename, key, secret, pass));
  storeMap.insert(std::make_pair(filename, newStore));

  return *newStore;
}

std::vector< std::shared_ptr<Quacks::Twit::Account> > Quacks::Twit::FileAccountStore::storedAccounts()
{
  return impl->getAccounts();
}

void Quacks::Twit::FileAccountStore::beginCreateAccount(const CreatingAccountResultCallback &callback)
{
  if (impl->secret.empty())
  {
    callback(false, "", *this);
  }
}

std::shared_ptr<Quacks::Twit::Account> Quacks::Twit::FileAccountStore::endCreateAccount(const std::string &pin)
{
  return nullptr;
}

Quacks::Twit::FileAccountStore::FileAccountStore(const std::string &filename)
  : impl(new Impl(filename))
{
}

Quacks::Twit::FileAccountStore::FileAccountStore(const std::string &filename, const std::string &key, const std::string &secret, const std::string &pass)
  : impl(new Impl(filename, key, secret))
{
  impl->unlock(pass);
}

bool Quacks::Twit::FileAccountStore::unlock(const std::string &pass)
{
  return false;
}

bool Quacks::Twit::FileAccountStore::isLocked()
{
  return true;
}
