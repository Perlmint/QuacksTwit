#include "../request.h"
#include "Account.h"
#import <Foundation/Foundation.h>
#import <Social/Social.h>
#import <Accounts/Accounts.h>
#include <iostream>

void Quacks::Twit::Request::sendRequest(RequestType requestType,
										std::shared_ptr<Account> account,
                                        const std::string &url,
                                        const RequestArgType &args,
                                        const CallbackFuncType &callback)
{
  NSURL *requestURL = [NSURL
                        URLWithString:
                          [NSString stringWithUTF8String:url.c_str()]];

  NSMutableDictionary *message = [NSMutableDictionary
                                   dictionaryWithCapacity:args.size()];

  for (const auto &kv : args)
  {
    [message
      setObject:[NSString stringWithUTF8String:kv.second.c_str()]
         forKey:[NSString stringWithUTF8String:kv.first.c_str()]];
  }


  SLRequestMethod method = SLRequestMethodGET;
  if (requestType == RequestType::POST)
  {
    method = SLRequestMethodPOST;
  }

  SLRequest *request = [SLRequest
                         requestForServiceType:SLServiceTypeTwitter
                                 requestMethod:method
                                           URL:requestURL
                                    parameters:message];

  request.account = (ACAccount *)account->getData();
	std::cout << request.account.description.UTF8String << std::endl;
  [request
    performRequestWithHandler:^(NSData *responseData,
                                NSHTTPURLResponse *urlResponse,
                                NSError *error)
    {
      int statusCode = [urlResponse statusCode];
      std::string ret;
        NSString *buffer = [[NSString alloc]
                             initWithData:responseData
                                 encoding:NSUTF8StringEncoding];
        ret = [buffer cStringUsingEncoding:NSUTF8StringEncoding];

      callback(account, statusCode, ret);
    }];
}
