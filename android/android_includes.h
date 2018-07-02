#ifndef _ANDROID_INCLUDES_H_
#define _ANDROID_INCLUDES_H_

#ifdef BUILD_ANDROID_AP

#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/RefBase.h>
#include <utils/RWLock.h>
#include <utils/String8.h>
#include <utils/List.h>
#include <utils/Vector.h>
#include <utils/Errors.h>
#include <cutils/atomic.h>

using android::Mutex;
using android::Condition;
using android::RWLock;
using android::List;
using android::String8;
using android::Vector;
using android::sp;
using android::wp;
using android::RefBase;

#define ATOMIC_INC(intAddr) (android_atomic_inc(intAddr))
#define ATOMIC_DEC(intAddr) (android_atomic_dec(intAddr))

#endif

#endif
