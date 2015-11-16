#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace Quacks
{
  namespace Twit
  {
    class Account;

    enum class RequestType
    {
      POST,
      GET
    };

    namespace Request
    {
      typedef
        std::function<void(std::shared_ptr<Account> account, int code, const std::string &ret)>
        CallbackFuncType;
      typedef
        std::vector< std::pair<std::string, std::string> >
        RequestArgType;

#define REQUEST_DEF(TYPE) \
      void sendRequest(RequestType requestType, \
                       std::shared_ptr<TYPE> account, \
                       const std::string &url, \
                       const RequestArgType &args, \
                       const CallbackFuncType &callback)

      REQUEST_DEF(Account);
    }
  }
}
