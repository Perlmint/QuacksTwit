#include <iostream>
#include <mutex>
#include <string>
#include <memory>
#include <condition_variable>
#include "accountStore.h"
#include "account.h"
#include "rest.h"

int main(int argc, const char *argv[])
{
  auto store = Quacks::Twit::SystemAccountStore::GetAccountStore();
  int trialCount = 0;
  while (!store->waitGrant())
  {
    std::cout << "Access request rejected" << std::endl;
    if (trialCount++ > 3)
    {
      return -1;
    }
    store->requestAccess();
  }

  auto accounts = store->storedAccounts();
  int i = 0;
  for (const auto &account : accounts)
  {
    std::cout << i << " : " << account->username() << std::endl;
  }
  if (accounts.empty()) {
    std::cout << "Accounts not found" << std::endl;
    return -1;
  }

  std::cin >> i;

  std::mutex mtx;
  std::condition_variable cv;

  std::unique_lock<std::mutex> lock(mtx);

  auto callback =[&](std::shared_ptr<Quacks::Twit::Account> account, int code, const std::string &ret) {
    std::cout << code << std::endl << ret << std::endl;
    cv.notify_all();
  };

  Quacks::Twit::statuses::home_timeline timeline;
  timeline.account = accounts.at(i);
  timeline();

  cv.wait(lock);
  return 0;
}
