#ifdef WIN32
#include <Windows.h>
#include <Shlwapi.h>
#else
#include <termios.h>
#include <unistd.h>
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
bool isFileExists(const std::string &filepath);

#if defined(USE_APPLE) || defined(USE_ANDROID)
std::shared_ptr<Quacks::Twit::IAccountStore> getSystemAccountStore();
#endif
#if defined(USE_CURL)
std::shared_ptr<Quacks::Twit::IAccountStore> openFileAccountStore();
#endif

int main(int argc, const char *argv[])
{
#if defined(WIN32)
  SetConsoleOutputCP(65001);
#endif
  std::shared_ptr<Quacks::Twit::IAccountStore> store(nullptr);

  int index = 0, selectedIndex = 0;
  do
  {
    index = 0;
    std::cout << "Select account store type" << std::endl;

#if defined(USE_APPLE) || defined(USE_ANDROID)
    std::cout << "  " << index << ": System Account Store" << std::endl;
    ++index;
#endif
#if defined(USE_CURL)
    std::cout << "  " << index << ": File Account Store" << std::endl;
#endif
    std::cout << "enter store type : ";
    std::cin >> selectedIndex;
  } while (index < selectedIndex);

  index = 0;
#if defined(USE_APPLE) || defined(USE_ANDROID)
  if (selectedIndex == index)
  {
    store = getSystemAccountStore();
  }
  ++index;
#endif
#if defined(USE_CURL)
  if (selectedIndex == index)
  {
    store = openFileAccountStore();
  }
#endif

  std::mutex mtx;
  std::condition_variable cv;

  auto accounts = store->storedAccounts();
  std::shared_ptr<Quacks::Twit::Account> account(nullptr);
  int count = 0, i;
  do
  {
    for (const auto &account : accounts)
    {
      std::cout << count++ << " : " << account->username() << std::endl;
    }
    if (accounts.empty())
    {
      std::cout << "Accounts not found" << std::endl;
    }
    std::cout << count << " : Add new account" << std::endl;

    std::cin >> i;
    if (i == count)
    {
      std::unique_lock<std::mutex> lock(mtx);
      store->beginCreateAccount([&cv, &account](bool success, const std::string &url, std::shared_ptr<Quacks::Twit::Account> newAccount) {
        if (!success)
        {
          std::cerr << url;
          return;
        }

        std::string pin;
#if defined(WIN32)
        ShellExecute(nullptr, nullptr, url.c_str(), nullptr, nullptr, SW_SHOW);
#else
        std::cout << "URL : " << url << std::endl;
#endif
        std::cout << "enter pin : ";
        std::cin >> pin;
        account = newAccount;
        newAccount->endCreateAccount(pin);
      });
    }
  } while(i > count || i < 0);

  if (!account)
  {
    account = accounts.at(i);
  }

  const auto &classic_table = std::ctype<char>::classic_table();
  std::vector<std::ctype<char>::mask> multilineTable(classic_table, classic_table + std::ctype<char>::table_size);
  multilineTable[' '] ^= std::ctype_base::space;
  multilineTable['\t'] &= ~(std::ctype_base::space | std::ctype_base::cntrl);
  multilineTable['\n'] &= ~(std::ctype_base::space);
  multilineTable['\r'] &= ~(std::ctype_base::space);
  std::locale originalLocale = std::cin.getloc();
  std::locale multilineLocale(originalLocale, new std::ctype<char>(multilineTable.data()));

  while(true)
  {
    int menu;
    std::cout << "Enter command" << std::endl;
    std::cout << "0 : quit" << std::endl;
    std::cout << "1 : timeline" << std::endl;
    std::cout << "2 : update" << std::endl << ">> ";
    std::cin >> menu;
    switch(menu)
    {
    case 0:
      return 0;
    case 1:
    {
      Quacks::Twit::statuses::home_timeline timeline;
      timeline.account = account;
      timeline([](std::shared_ptr<Quacks::Twit::Account> account, const std::deque<Quacks::Twit::twit> &ret) {
      for (const auto &twit : ret)
      {
        std::cout << twit.user.screen_name << "(@" << twit.user.user_id << ")" << std::endl;
        std::cout << twit.text << std::endl;
      }
      });
      break;
    }
    case 2:
    {
      Quacks::Twit::statuses::update update;
      update.account = account;
      std::string text;
      std::cout << "Enter your status : ";
      std::cin.imbue(multilineLocale);
      std::cin >> text;
      std::cin.imbue(originalLocale);
      update.status(text);
      update([](std::shared_ptr<Quacks::Twit::Account> account, const std::string &ret) {
      });
      break;
    }
    }
  }

  return 0;
}

#if defined(USE_APPLE) || defined(USE_ANDROID)
std::shared_ptr<Quacks::Twit::IAccountStore> getSystemAccountStore()
{
  Quacks::Twit::SystemAccountStore *systemStore = nullptr;
  systemStore = Quacks::Twit::SystemAccountStore::GetAccountStore();

  int trialCount = 0;
  while (!systemStore->waitGrant())
  {
    std::cout << "Access request rejected" << std::endl;
    if (trialCount++ > 3)
    {
      throw std::exception();
    }
    systemStore->requestAccess();
  }

  return systemStore->shared_from_this();
}
#endif

#if defined(USE_CURL)
std::string getSecret(const std::string &message)
{
  std::string storePass;
  setStdinEcho(false);
  std::cout << message;
  std::cin >> storePass;
  setStdinEcho(true);
  std::cout << std::endl;
  return storePass;
}

std::shared_ptr<Quacks::Twit::IAccountStore> openFileAccountStore()
{
  std::string storePath;
  std::cout << "Account Store Path : ";
  std::cin >> storePath;

  std::shared_ptr<Quacks::Twit::FileAccountStore> store(nullptr);

  if (!isFileExists(storePath))
  {
    store = Quacks::Twit::FileAccountStore::CreateAccountStore(storePath, getSecret("Consumer Key : "), getSecret("Consumer Secret : "), getSecret("Account Store Pass : "));
  }
  else
  {
    store = Quacks::Twit::FileAccountStore::GetAccountStore(storePath);

    while (store->isLocked())
    {
      std::string storePass = getSecret("Account Store Pass : ");
      store->unlock(storePass);
    }
  }

  return store;
}
#endif

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

bool isFileExists(const std::string &filepath)
{
#ifdef WIN32
  return PathFileExists(filepath.c_str());
#else
  return access(filepath.c_str(), R_OK | W_OK | F_OK) != -1;
#endif
}
