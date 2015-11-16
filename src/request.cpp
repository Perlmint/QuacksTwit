#include "request.h"
#include "config.h"
#ifdef USE_CURL
#include "curl/account_curl.h"
#endif
#ifdef USE_APPLE
#include "apple/account_apple.h"
#endif

namespace Quacks
{
  namespace Twit
  {
    namespace Request
    {
#ifdef USE_CURL
      REQUEST_DEF(CurlAccount);
#endif
#ifdef USE_APPLE
      REQUEST_DEF(AppleAccount);
#endif
    }
  }
}

void Quacks::Twit::Request::sendRequest(RequestType requestType,
                 std::shared_ptr<Account> account,
                 const std::string &url,
                 const RequestArgType &args,
                 const CallbackFuncType &callback)
{
#define CALL_REQUEST(TYPE) \
  if (std::dynamic_pointer_cast<TYPE>(account)) \
  { \
    sendRequest(requestType, std::dynamic_pointer_cast<TYPE>(account), \
                url, args, callback); \
  }
#ifdef USE_CURL
  CALL_REQUEST(CurlAccount);
#endif
#ifdef USE_APPLE
  CALL_REQUEST(AppleAccount);
#endif
}
