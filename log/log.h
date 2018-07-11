#ifndef _LOG_H_
#define _LOG_H_

#include "log_impl.h"

namespace sirius {

#ifdef ENABLE_LOGGER
#define LOGD(module, fmt, args...) PLOGD(module, fmt, ##args)
#define LOGI(module, fmt, args...) PLOGI(module, fmt, ##args)
#define LOGW(module, fmt, args...) PLOGW(module, fmt, ##args)
#define LOGE(module, fmt, args...) PLOGE(module, fmt, ##args)
#define LOGF(module, fmt, args...) PLOGF(module, fmt, ##args)
#define ASSERT_LOG(module, cond, fmt, args...) ASSERT_PLOG(module, cond, fmt, ##args)
#else
#define LOGD(module, fmt, args)
#define LOGI(module, fmt, args)
#define LOGW(module, fmt, args)
#define LOGE(module, fmt, args)
#define LOGF(module, fmt, args)
#define ASSERT_LOG(module, cond, fmt, args)
#endif

};

#endif

