namespace oauth
{
  const int OAUTH_BUFFER_SIZE = 1024;
  const int OAUTH_BUFFER_SIZE_LARGE = 1024;
  const std::string OAUTH_CONSUMER_KEY = "oauth_consumer_key";
  const std::string OAUTH_CALLBACK = "oauth_callback";
  const std::string OAUTH_VERSION = "oauth_version";
  const std::string OAUTH_SIGNATUREMETHOD = "oauth_signature_method";
  const std::string OAUTH_SIGNATURE = "oauth_signature";
  const std::string OAUTH_TIMESTAMP = "oauth_timestamp";
  const std::string OAUTH_NONCE = "oauth_nonce";
  const std::string OAUTH_TOKEN = "oauth_token";
  const std::string OAUTH_TOKENSECRET = "oauth_token_secret";
  const std::string OAUTH_VERIFIER = "oauth_verifier";
  const std::string OAUTH_SCREENNAME = "screen_name";
  const std::string OAUTH_AUTHENTICITY_TOKEN = "authenticity_token";
  const std::string OAUTH_SESSIONUSERNAME = "session[username_or_email]";
  const std::string OAUTH_SESSIONPASSWORD = "session[password]";
  const std::string OAUTH_AUTHENTICITY_TOKEN_TWITTER_RESP = "authenticity_token\" type=\"hidden\" value=\"";
  const std::string OAUTH_TOKEN_TWITTER_RESP = "oauth_token\" type=\"hidden\" value=\"";
  const std::string OAUTH_PIN_TWITTER_RESP = "code-desc\"><code>";
  const std::string OAUTH_TOKEN_END_TAG_TWITTER_RESP = "\">";
  const std::string OAUTH_PIN_END_TAG_TWITTER_RESP = "</code>";

  const std::string OAUTH_AUTHHEADER_STRING = "Authorization: OAuth ";
}
