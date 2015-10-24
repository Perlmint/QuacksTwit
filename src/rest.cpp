#include "rest.h"
#include "request.h"

void Quacks::Twit::statuses::home_timeline::operator() (const std::function<void(std::shared_ptr<Account> account, int code, const std::string &ret)> &callback)
{
    Quacks::Twit::Request::sendRequest(
    Quacks::Twit::RequestType::GET, account,
    "https://api.twitter.com/1.1/statuses/home_timeline.json",
    Quacks::Twit::Request::RequestArgType({}),
    callback);
}
