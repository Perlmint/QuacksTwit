#include "curl_helper.h"
#include "oauthlib.h"
#include <memory.h>

static bool curlInited = false;

CURL *Quacks::Twit::CurlHelper::getCurl()
{
  if (!curlInited)
  {
    curl_global_init(CURL_GLOBAL_ALL);
    curlInited = true;
  }

  return curl_easy_init();
}

Quacks::Twit::CurlHelper::CurlHelper(oAuth &auth, RequestType type, const std::string &url, const std::string &params, bool pinVerify)
  : memory(reinterpret_cast<char *>(malloc(1)))
  , size(0)
  , curl(getCurl())
  , headers(nullptr)
{
  headers = curl_slist_append(headers, auth.getOAuthHeader(static_cast<_eOAuthHttpRequestType>(type), url, params, pinVerify).c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  const char *requestTypeStr = nullptr;
  switch (type)
  {
  case RequestType::POST:
    requestTypeStr = "POST";
    break;
  case RequestType::GET:
    requestTypeStr = "GET";
    break;
  case RequestType::DEL:
    requestTypeStr = "DELETE";
    break;
  }
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, requestTypeStr);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void *>(this));
  if (!params.empty())
  {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
  }
}

size_t Quacks::Twit::CurlHelper::callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  CurlHelper *mem = static_cast<CurlHelper *>(userp);

  mem->memory = static_cast<char *>(realloc(mem->memory, mem->size + realsize + 1));
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}
