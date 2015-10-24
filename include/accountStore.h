#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace Quacks
{
  namespace Twit
  {
    class Account;

    class IAccountStore
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
      std::vector< std::shared_ptr<Account> > storedAccounts();
      void beginCreateAccount(const CreatingAccountResultCallback &callback);
      std::shared_ptr<Account> endCreateAccount(const std::string &pin);
    };

    #if defined( __APPLE__ ) || defined(ANDROID)
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
    #endif
  }
}
