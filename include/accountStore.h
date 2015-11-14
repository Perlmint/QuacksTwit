#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>

namespace Quacks
{
  namespace Twit
  {
    class Account;

    class IAccountStore : public std::enable_shared_from_this<IAccountStore>
    {
    public:
      using CreatingAccountResultCallback =
        std::function<void(bool, const std::string &, IAccountStore &)>;
      virtual std::vector< std::shared_ptr<Account> > storedAccounts() = 0;
      virtual void beginCreateAccount(const CreatingAccountResultCallback &callback) = 0;
      virtual std::shared_ptr<Account> endCreateAccount(const std::string &pin) = 0;
    };

    class FileAccountStore : public IAccountStore
    {
    public:
      static FileAccountStore &GetAccountStore(const std::string &filename);
      static FileAccountStore &CreateAccountStore(const std::string &filename, const std::string &key, const std::string &secret, const std::string &pass);
      std::vector< std::shared_ptr<Account> > storedAccounts();
      void beginCreateAccount(const CreatingAccountResultCallback &callback);
      std::shared_ptr<Account> endCreateAccount(const std::string &pin);

    bool unlock(const std::string &pass);
    bool isLocked();
  private:
    FileAccountStore(const std::string &filename);
    FileAccountStore(const std::string &filename, const std::string &key, const std::string &secret, const std::string &pass);
    class Impl;
    std::unique_ptr<Impl> impl;
    static std::map<std::string, std::shared_ptr<FileAccountStore>> storeMap;
    };

    #if defined( __APPLE__ ) || defined(ANDROID)
  #define SYSTEM_ACCOUNT_STORE_EXISTS 1
    class SystemAccountStore : public IAccountStore
    {
    public:
      #ifdef ANDROID
      static void Init(void *context);
      #endif
      static SystemAccountStore *GetAccountStore();
      void requestAccess();
      bool waitGrant();
      std::vector< std::shared_ptr<Account> > storedAccounts();
      void beginCreateAccount(const CreatingAccountResultCallback &callback);
      std::shared_ptr<Account> endCreateAccount(const std::string &pin);

    private:
      SystemAccountStore();
      class Impl;
      static std::unique_ptr<SystemAccountStore> instance;
      std::unique_ptr<Impl> impl;
    };
  #else
    #define SYSTEM_ACCOUNT_STORE_EXISTS 0
  #endif
  }
}
