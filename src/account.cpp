#include "account.h"

namespace Quacks
{
  namespace Twit
  {
    Account::Account(void *data)
      :impl(new Impl(data))
    {
    }

    Account::~Account()
    {
    }

    void *Account::getData() const
    {
      return impl->getData();
    }

    std::string Account::username() const
    {
      return impl->username();
    }

    std::string Account::identifier() const
    {
      return impl->identifier();
    }
  }
}
