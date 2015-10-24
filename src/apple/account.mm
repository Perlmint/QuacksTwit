#include "account.h"
#import <Accounts/Accounts.h>

namespace Quacks
{
  namespace Twit
  {
    class Account::Impl
    {
    public:
      Impl(void *data)
        : account(static_cast<ACAccount *>(data))
      {
      }

      Impl()
      {
      }

      std::string username() const
      {
        return account.username.UTF8String;
      }

      std::string identifier() const
      {
        return account.identifier.UTF8String;
      }
		
      void *getData() const
      {
        return static_cast<void *>(account);
      }
    private:
      ACAccount *account = nullptr;
    };
  }
}

#include "../account.cpp"
