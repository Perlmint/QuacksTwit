#include <cstdio>
#include "accountStore.h"
#include "McbDES/McbDES2.h"

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
    this->key = key;
    read();
    }
  private:
    void read()
    {
    FILE *fp = fopen(storeFilePath.c_str(), "r");

    // File notfound - ignore
    if (fp == nullptr)
      {
      return;
      }

    fclose(fp);
    }

    void write()
    {
    FILE *fp = fopen(storeFilePath.c_str(), "w");

    if (fp == nullptr)
      {
      throw std::exception("File creation failed");
      }

    fclose(fp);
    }

    std::vector< std::shared_ptr<Quacks::Twit::Account> > accounts;
    const std::string storeFilePath;
    std::string storekey;
    std::string key;
    std::string secret;
    bool unlocked = false;

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
