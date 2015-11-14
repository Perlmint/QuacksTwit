#include "account.h"
#include <tuple>

namespace Quacks
{
  namespace Twit
  {
    class Account::Impl
    {
    public:
      using AccountInfoWithName =
        std::tuple<std::string, std::string, std::string, std::string>;
      using AccountInfo = std::tuple<std::string, std::string>;
      Impl(void *data)
        : info(std::get<0>(*static_cast<AccountInfoWithName *>(data)),
          std::get<1>(*static_cast<AccountInfoWithName *>(data)))
        , username_cache(std::get<2>(*static_cast<AccountInfoWithName *>(data)))
        , identifier_cache(std::get<3>(*static_cast<AccountInfoWithName *>(data)))
      {
      }

      Impl()
      {
      }

      std::string username() const
      {
        return username_cache;
      }

      std::string identifier() const
      {
        return identifier_cache;
      }

      void *getData() const
      {
        return const_cast<void *>(static_cast<const void *>(&info));
      }
    private:
      const AccountInfo info;
      std::string username_cache;
      std::string identifier_cache;
    };
  }
}

#include "../account.cpp"
