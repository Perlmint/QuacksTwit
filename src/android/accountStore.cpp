#include "accountStore.h"
#include "account.h"
#include "jni_helper.h"

namespace Quacks
{
  namespace Twit
  {
    static jobject *SystemAccountStoreContext = nullptr;
    void SystemAccountStore::Init(void *context)
    {
      SystemAccountStoreContext = static_cast<jobject *>(context);
    }

    SystemAccountStore *SystemAccountStore::GetAccountStore()
    {
      if (SystemAccountStoreContext != nullptr)
      {
        return nullptr;
      }

      if (instance == nullptr)
      {
        instance.reset(new SystemAccountStore());
      }

      return instance.get();
    }

    class SystemAccountStore::Impl : public IAccountStore
    {
    public:
      Impl()
      {
        auto env = GetEnv();
        managerClass = env->FindClass("android/accounts/AccountManager");
        auto method = env->GetStaticMethodID(managerClass,
                                       "GetMethodID",
                                       "(Landroid/content/Context;)Landroid/accounts/AccountManager;");
        manager = env->CallStaticObjectMethod(managerClass, method, context);
      }

      void requestAccess()
      {
        // Do nothing
      }

      bool waitGrant()
      {
        return true;
      }

      std::vector< std::shared_ptr<Account> > storedAccounts()
      {
        auto env = GetEnv();
        if (!getAccountMethod)
        {
          getAccountMethod = env->GetMethodID(managerClass,
                                              "getAccountsByType",
                                              "(Ljava/lang/String;)[Landroid/accounts/Account;");
        }

        auto accounts = env->CallObjectMethod(manager, getAccountMethod, );
      }

      void beginCreateAccount(const CreatingAccountResultCallback &callback)
      {
      }

      std::shared_ptr<Account> endCreateAccount(const std::string &pin)
      {
      }

    private:
      jobject context = nullptr;
      jobject manager = nullptr;
      jclass managerClass = nullptr;
      jmethodID getAccountMethod = nullptr;
    }

    SystemAccountStore::SystemAccountStore()
      : impl(new Impl())
    {

    }
  }
}
