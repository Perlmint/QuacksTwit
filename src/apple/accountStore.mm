#include "accountStore.h"
#include "account.h"
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>
#import <Accounts/Accounts.h>

namespace Quacks
{
  namespace Twit
  {
    class SystemAccountStore::Impl : public IAccountStore
    {
    public:
      Impl()
      {
        requestAccess();
      }

      ~Impl()
      {
        if (store != nullptr)
        {
          [store release];
          store = nullptr;
        }
        if (accountType != nullptr)
        {
          [accountType release];
          accountType = nullptr;
        }
      }

      void requestAccess()
      {
        if (granted)
        {
          return;
        }

        store = [[ACAccountStore alloc] init];
        accountType = [store accountTypeWithAccountTypeIdentifier:
                                                ACAccountTypeIdentifierTwitter];
        [accountType retain];
        [store requestAccessToAccountsWithType:accountType options:nil
                                      completion:^(BOOL success, NSError *error)
                 {
                   granted = success == YES;
                   if (error != nullptr)
                   {
                     std::cerr << error.localizedDescription.UTF8String;
                   }
                   cv.notify_all();
                 }];
      }

      std::vector<std::shared_ptr<Account>> storedAccounts()
      {
        if (!granted)
        {
          return accounts;
        }

        NSArray *arrayOfAccounts = [store accountsWithAccountType:accountType];
        std::vector<void *> accountsList;
        for (id account in arrayOfAccounts)
        {
          accountsList.push_back(static_cast<void *>(account));
        }

        {
          auto list_begin = std::begin(accounts),
            list_end = std::end(accounts);
          auto pred = [](const std::shared_ptr<Account> &em, void *data) -> bool {
            return em->getData() == data;
          };
          accountsList.erase(std::remove_if(std::begin(accountsList), std::end(accountsList), [&](void *data) -> bool {
                return std::find_if(list_begin, list_end, std::bind(pred, std::placeholders::_1, data)) != list_end;
              }), std::end(accountsList));
        }

        {
          auto list_begin = std::begin(accountsList),
            list_end = std::end(accountsList);
          accounts.erase(std::remove_if(std::begin(accounts), std::end(accounts), [&](const std::shared_ptr<Account> &em) -> bool {
                return std::find(list_begin, list_end, em->getData()) == list_end;
              }), std::end(accounts));
        }

        for (const auto &account : accountsList)
        {
          accounts.push_back(std::shared_ptr<Account>(new Account(account)));
        }

        return accounts;
      }

      void beginCreateAccount(const CreatingAccountResultCallback &callback)
      {
        callback(false, std::string(""), *this);
      }

      std::shared_ptr<Account> endCreateAccount(const std::string &pin)
      {
        return nullptr;
      }

      bool waitGrant()
      {
        if (granted)
        {
          return true;
        }

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, std::chrono::seconds(60));
        return granted;
      }

    private:
      ACAccountStore *store = nullptr;
      ACAccountType *accountType = nullptr;
      bool granted = false;
      std::vector<std::shared_ptr<Account>> accounts;
      std::condition_variable cv;
      std::mutex mtx;
    };

    std::unique_ptr<SystemAccountStore> SystemAccountStore::instance = nullptr;

    SystemAccountStore *SystemAccountStore::GetAccountStore()
    {
      if (instance == nullptr)
      {
        instance.reset(new SystemAccountStore());
      }

      return instance.get();
    }

    SystemAccountStore::SystemAccountStore()
      : impl(new Impl())
    {
    }

    void SystemAccountStore::requestAccess()
    {
      impl->requestAccess();
    }

    std::vector< std::shared_ptr<Account> > SystemAccountStore::storedAccounts()
    {
      return impl->storedAccounts();
    }

    void SystemAccountStore::beginCreateAccount(const CreatingAccountResultCallback &callback)
    {
      impl->beginCreateAccount(callback);
    }

    std::shared_ptr<Account> SystemAccountStore::endCreateAccount(const std::string &pin)
    {
      return impl->endCreateAccount(pin);
    }

    bool SystemAccountStore::waitGrant()
    {
      return impl->waitGrant();
    }
  }
}
