
#include "log_impl.h"

namespace sirius {

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef DBG_ASSERT_RAISE_TRAP
#include "signal.h"
#endif
#ifdef BUILD_ANDROID_AP
#include <utils/Log.h>

#undef LOG_TAG
#define LOG_TAG PROJNAME VERSION
#endif

int8_t gDebugController[][LOG_TYPE_MAX_INVALID + 1] = {
    // NONE,  DBG,  INF, WARN,  ERR, FATA, INVA
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_OTHERS
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SIRIUS
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SIRIUS_IMPL
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SIRIUS_CORE
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SOCKET
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SOCKET_SERVER
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SOCKET_SERVER_SM
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SOCKET_CLIENT
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SOCKET_CLIENT_SM
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SERVER_CLIENT_CONTROL
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_BUF_MANAGER
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_ION_BUF_MANAGER
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SERVER_CB_THREAD
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_RUN_ONCE_THREAD
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_THREAD_POOL
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_MEMORY_POOL
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_REQUEST_HANDLER
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_REQUEST_CLIENT_HANDLER
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_SIRIUS_CLIENT_CORE
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_PREVIEW_REQUEST
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_YUV_PICTURE_REQUEST
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_BAYER_PICTURE_REQUEST
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_EVT_REQUEST,
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_DATA_REQUEST,
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_UTILS
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_TOOLS
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_TESTER
    {     0,    1,    1,    1,    1,    1,    0}, // MODULE_MAX_INVALID
};

static const char *const gLogType[] = {
    [LOG_TYPE_NONE]        = "<NONE>",
    [LOG_TYPE_DEBUG]       = "< DBG>",
    [LOG_TYPE_INFO]        = "< INF>",
    [LOG_TYPE_WARN]        = "<WARN>",
    [LOG_TYPE_ERROR]       = "< ERR>",
    [LOG_TYPE_FATAL]       = "<FATA>",
    [LOG_TYPE_MAX_INVALID] = "<INVA>",
};

static char process[PATH_MAX] = { '\0' };
#include "errno.h"
static int32_t getMaxInvalidId(char *text, int32_t len)
{
    int32_t i = 0;

    for (; i < len; i++) {
        if (!((text[i] == '/') ||
            (text[i] == '.') || (text[i] == '_') ||
            (text[i] >= '0' && text[i] <= '9') ||
            (text[i] >= 'A' && text[i] <= 'Z') ||
            (text[i] >= 'a' && text[i] <= 'z'))) {
            text[i + 1] = '\0';
            break;
        }
    }

    return i;
}

static char *getProcessName()
{
    if (process[0] == '\0') {
        pid_t pid = getpid();
        char path[32];
        char text[PATH_MAX] = { '\0' };

        sprintf(path, "/proc/%d/cmdline", pid);
        int32_t fd = open(path, O_RDONLY);
        if (fd > 0) {
            ssize_t len = read(fd, text, PATH_MAX);
            if (len > 0) {
                text[len] = text[getMaxInvalidId(text, len)] = '\0';
                char *index = strrchr(text, '/');
                if (index != NULL) {
                    strcpy(process, index + 1);
                }
            }
            close(fd);
        }

        if (process[0] == '\0') {
            text[0] = '\0';
            strcpy(path, "/proc/self/exe");
            ssize_t len = readlink(path, text, PATH_MAX);
            if (len > 0) {
                text[len] = text[getMaxInvalidId(text, len)] = '\0';
                char *index = strrchr(text, '/');
                if (index != NULL) {
                    strcpy(process, index + 1);
                }
            }
        }

        if (process[0] == '\0') {
            strcpy(process, "Unknown");
        }
    }

    return process;
}

static bool checkValid(LogType type)
{
    bool rc = false;

    if (type > 0 && type < LOG_TYPE_MAX_INVALID) {
        rc = true;
    }

    return rc;
}

static LogType getValidType(LogType type)
{
    return checkValid(type) ? type : LOG_TYPE_MAX_INVALID;
}

static const char *getLogType(LogType type)
{
    return gLogType[getValidType(type)];
}

static int32_t __log_vsnprintf(char* pdst, int32_t size,
    const char* pfmt, va_list argptr)
{
    int32_t written = 0;

    pdst[0] = '\0';
    written = vsnprintf(pdst, size, pfmt, argptr);

    if ((written >= size) && (size > 0)) {
       // Message length exceeds the buffer limit size
       written = size - 1;
       pdst[size - 1] = '\0';
    }

    return written;
}

void print_log(const LogType logt, const char *fmt,
    char *process, const char *module, const char *type,
    const char *func, const int line, const char *buf);

void __debug_log(const ModuleType module, const LogType type,
    const char *func, const int line, const char *fmt, ...)
{
    char    buf[DBG_LOG_MAX_LEN];
    va_list args;

    va_start(args, fmt);
    __log_vsnprintf(buf, DBG_LOG_MAX_LEN, fmt, args);
    va_end(args);

    print_log(type, "%s %s%s: %s:+%d: %s",
        getProcessName(), getModuleShortName(module),
        getLogType(type), func, line, buf);

    // TODO: write to file if necessary
}

void __assert_log(const ModuleType module, const unsigned char cond,
    const char *func, const int line, const char *fmt, ...)
{
    char    buf[DBG_LOG_MAX_LEN];
    va_list args;

    if (cond == 0) {
        va_start(args, fmt);
        __log_vsnprintf(buf, DBG_LOG_MAX_LEN, fmt, args);
        va_end(args);

        print_log(LOG_TYPE_ERROR, "[<! ASSERT !>]%s %s%s: %s:+%d: %s",
            getProcessName(), getModuleShortName(module),
            "<ASSERT>", func, line, buf);

        // TODO: write to file if necessary

#ifdef DBG_ASSERT_RAISE_TRAP
        raise(SIGTRAP);
#endif
    }
}

void print_log(const LogType logt, const char *fmt,
    char *process, const char *module, const char *type,
    const char *func, const int line, const char *buf)
{
#if defined(BUILD_ANDROID_AP)
    switch (logt) {
        case LOG_TYPE_NONE:
        case LOG_TYPE_DEBUG:
            ALOGD(fmt, process, module, type, func, line, buf);
            break;
        case LOG_TYPE_INFO:
            ALOGI(fmt, process, module, type, func, line, buf);
            break;
        case LOG_TYPE_WARN:
            ALOGW(fmt, process, module, type, func, line, buf);
            break;
        case LOG_TYPE_ERROR:
        case LOG_TYPE_FATAL:
            ALOGE(fmt, process, module, type, func, line, buf);
            break;
        case LOG_TYPE_MAX_INVALID:
        default:
            ALOGE(fmt, process, module, type, func, line, buf);
            break;
    }
#elif defined(BUILD_LINUX_X86_64)
    printf(fmt, process, module, type, func, line, buf);
#else
#error Not supported system arch.
#endif
}

};

