#include "request.h"
#include "account_curl.h"

namespace Quacks
{
  namespace Twit
  {
    namespace Request
    {
      void sendRequest(RequestType requestType,
                       std::shared_ptr<CurlAccount> account,
                       const std::string &url,
                       const RequestArgType &args,
                       const CallbackFuncType &callback)
      {
        // TODO: implement
      }
    }
  }
}
