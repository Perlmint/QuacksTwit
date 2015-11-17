#include "request.h"
#include "account_curl.h"
#include "curl_helper.h"

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
        std::string paramStr;
        for (const auto &kv : args)
        {
          if (!paramStr.empty())
          {
            paramStr.push_back('&');
          }
          paramStr.append(kv.first);
          if (!kv.second.empty())
          {
            paramStr.push_back('=');
            paramStr.append(kv.second);
          }
        }

        std::string modifiedUrl(url);

        CurlHelper::RequestType type;
        switch (requestType)
        {
        case RequestType::GET:
          type = CurlHelper::RequestType::GET;
          if (!paramStr.empty())
          {
            modifiedUrl.push_back('&');
            modifiedUrl.append(paramStr);
            paramStr = "";
          }
          break;
        case RequestType::POST:
          type = CurlHelper::RequestType::POST;
          break;
        }

        CurlHelper helper(account->getAuth(), type, modifiedUrl, paramStr);
        if (helper.perform() == CURLE_OK)
        {
          callback(account, 200, helper.getData());
        }
      }
    }
  }
}
