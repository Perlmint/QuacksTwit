#ifdef WIN32
#include <Windows.h>
#else
#include <termios.h>
#endif
#include <iostream>
#include <mutex>
#include <string>
#include <memory>
#include <condition_variable>
#include "config.h"
#include "accountStore.h"
#include "account.h"
#include "gen/rest.h"
#include "gen/twit.h"

void setStdinEcho(bool enable = true);

int main(int argc, const char *argv[])
{
  Quacks::Twit::IAccountStore *store = nullptr;
#if defined(USE_APPLE) || defined(USE_ANDROID)
  Quacks::Twit::SystemAccountStore *systemStore = nullptr;
  systemStore = Quacks::Twit::SystemAccountStore::GetAccountStore();

  int trialCount = 0;
  while (!systemStore->waitGrant())
  {
    std::cout << "Access request rejected" << std::endl;
    if (trialCount++ > 3)
    {
      return -1;
    }
    systemStore->requestAccess();
  }

  store = systemStore;
#else
  std::string storePath;
  if (argc > 2)
  {
    storePath = argv[1];
  }
  else
  {
    std::cout << "Account Store Path : ";
    std::cin >> storePath;
  }
  Quacks::Twit::FileAccountStore &fileStore = Quacks::Twit::FileAccountStore::GetAccountStore(storePath);

  while (fileStore.isLocked())
  {
    std::string storePass;
    setStdinEcho(false);
    std::cout << "Account Store Pass : ";
    std::cin >> storePass;
    setStdinEcho(true);
    fileStore.unlock(storePass);
  }
  store = &fileStore;
#endif

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

  Quacks::Twit::statuses::home_timeline timeline;
  timeline.account = accounts.at(i);
  timeline([](std::shared_ptr<Quacks::Twit::Account> account, const std::deque<Quacks::Twit::twit> &ret) {

  });

  cv.wait(lock);
  return 0;
}

void setStdinEcho(bool enable)
{
#ifdef WIN32
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode;
  GetConsoleMode(hStdin, &mode);

  if (!enable)
  {
    mode &= ~ENABLE_ECHO_INPUT;
  }
  else
  {
    mode |= ENABLE_ECHO_INPUT;
  }

  SetConsoleMode(hStdin, mode);
#else
  struct termios tty;
  tcgetattr(STDIN_FILENO, &tty);
  if (!enable)
  {
    tty.c_lflag &= ~ECHO;
  }
  else
  {
    tty.c_lflag |= ECHO;
  }

  (void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}