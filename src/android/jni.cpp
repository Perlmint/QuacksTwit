#include "jni_helper.h"

static JavaVM *_vm = nullptr;

JNIENV *Quacks::Twit::GetEnv()
{
  JNIENV *env = nullptr;
  if ((*_vm)->GetEnv(static_cast<void **>(&env)) != JNI_OK)
  {
    return nullptr;
  }

  return env;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
  _vm = vm;
}
