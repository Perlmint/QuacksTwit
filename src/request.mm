#include "request.h"

namespace Quacks
{
  namespace Twit
  {
    void sendRequest(RequestType requestType,
                       Account &account,
                       const std::string &url,
                       const RequestArgType &args,
                       const CallbackFuncType &callback)
    {
      NSURL *requestURL = [NSURL 
             URLWithString:[NSString stringWithC];

            SLRequest *postRequest = [SLRequest 
                requestForServiceType:SLServiceTypeTwitter
                       requestMethod:SLRequestMethodPOST
                       URL:requestURL parameters:message];
    }
  }
}
