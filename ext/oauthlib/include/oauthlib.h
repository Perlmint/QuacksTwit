#ifndef __OAUTHLIB_H__
#define __OAUTHLIB_H__

#include "time.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>

typedef enum _eOAuthHttpRequestType
{
    eOAuthHttpInvalid = 0,
    eOAuthHttpGet,
    eOAuthHttpPost,
    eOAuthHttpDelete
} eOAuthHttpRequestType;

typedef std::list<std::string> oAuthKeyValueList;
typedef std::map<std::string, std::string> oAuthKeyValuePairs;

class oAuth
{
public:
    oAuth(const std::string &consumerKey, const std::string &consumerSecret);
    oAuth(const oAuth &base, const std::string &accessToken, const std::string &accessTokenSecret);
    oAuth();
    ~oAuth();

    /* OAuth public methods used by twitCurl */
    const std::string &getConsumerKey() const
    {
        return m_consumerKey;
    }
    void setConsumerKey( const std::string& consumerKey /* in */ );

    const std::string &getConsumerSecret() const
    {
        return m_consumerSecret;
    }
    void setConsumerSecret( const std::string& consumerSecret /* in */ );

    const std::string &getOAuthTokenKey() const
    {
        return m_oAuthTokenKey;
    }
    void setOAuthTokenKey( const std::string& oAuthTokenKey /* in */ );

    const std::string &getOAuthTokenSecret() const
    {
        return m_oAuthTokenSecret;
    }
    void setOAuthTokenSecret( const std::string& oAuthTokenSecret /* in */ );

    const std::string &getOAuthScreenName() const
    {
        return m_oAuthScreenName;
    }
    void setOAuthScreenName( const std::string& oAuthScreenName /* in */ );

    const std::string &getOAuthPin() const
    {
        return m_oAuthPin;
    }
    void setOAuthPin( const std::string& oAuthPin /* in */ );

    std::string getOAuthHeader( const eOAuthHttpRequestType eType, /* in */
                         const std::string& rawUrl, /* in */
                         const std::string& rawData, /* in */
                         const bool includeOAuthVerifierPin = false /* in */ );

    bool extractOAuthTokenKeySecret( const std::string& requestTokenResponse /* in */ );

    oAuth clone();

private:

    /* OAuth data */
    std::string m_consumerKey;
    std::string m_consumerSecret;
    std::string m_oAuthTokenKey;
    std::string m_oAuthTokenSecret;
    std::string m_oAuthPin;
    std::string m_nonce;
    std::string m_timeStamp;
    std::string m_oAuthScreenName;

    /* OAuth twitter related utility methods */
    void buildOAuthRawDataKeyValPairs( const std::string& rawData, /* in */
                                       bool urlencodeData, /* in */
                                       oAuthKeyValuePairs& rawDataKeyValuePairs /* out */ );

    bool buildOAuthTokenKeyValuePairs( const bool includeOAuthVerifierPin, /* in */
                                       const std::string& oauthSignature, /* in */
                                       oAuthKeyValuePairs& keyValueMap /* out */,
                                       const bool generateTimestamp /* in */ );

    void getStringFromOAuthKeyValuePairs( const oAuthKeyValuePairs& rawParamMap, /* in */
                                          std::string& rawParams, /* out */
                                          const std::string& paramsSeperator /* in */ );

    bool getSignature( const eOAuthHttpRequestType eType, /* in */
                       const std::string& rawUrl, /* in */
                       const oAuthKeyValuePairs& rawKeyValuePairs, /* in */
                       std::string& oAuthSignature /* out */ );

    void generateNonceTimeStamp();
};

#endif // __OAUTHLIB_H__
