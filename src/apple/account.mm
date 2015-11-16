#include "account_apple.h"
#import <Accounts/Accounts.h>

#define THIS_ACCOUNT static_cast<ACAccount *>(this->account)

Quacks:::Twit::AppleAccount::AppleAccount(void *data)
  : account(data)
{
  [THIS_ACCOUNT retain];
}

Quacks::Twit::AppleAccount::~AppleAccount()
{
  [THIS_ACCOUNT release];
}

std::string Quacks:::Twit::AppleAccount::username() const
{
  return THIS_ACCOUNT.username.UTF8String;
}

std::string Quacks:::Twit::AppleAccount::identifier() const
{
  return THIS_ACCOUNT.identifier.UTF8String;
}

void *Quacks:::Twit::AppleAccount::getAccount() const
{
  return const_cast<void *>(static_cast<const void *>(THIS_ACCOUNT);
}

