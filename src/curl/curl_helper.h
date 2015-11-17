#pragma once
#include "curl/curl.h"
#include <stdlib.h>
#include <string>

class oAuth;

namespace Quacks
{
  namespace Twit
  {
    class CurlHelper
    {
    public:
      static CURL *getCurl();

      enum class RequestType
      {
        Invalid = 0,
        GET,
        POST,
        DELETE
      };

      CurlHelper(oAuth &auth, RequestType type, const std::string &url, const std::string &params = "");

      ~CurlHelper()
      {
        free(memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
      }

      CURLcode perform()
      {
        return curl_easy_perform(curl);
      }

      static size_t callback(void *contents, size_t size, size_t nmemb, void *userp);

      std::string getData()
      {
        return std::string(memory, size);
      }
    private:
      char *memory;
      size_t size;
      CURL *curl;
      curl_slist *headers;
    };
  }
}
