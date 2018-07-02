#include <string.h>

#include "modules.h"

namespace sirius {

static const char *const ModuleName[] = {
    [MODULE_OTHERS]                 = "others",
    [MODULE_SIRIUS]                 = "sirius",
    [MODULE_SIRIUS_IMPL]            = "sirius_impl",
    [MODULE_SIRIUS_CORE]            = "sirius_core",
    [MODULE_SOCKET]                 = "socket",
    [MODULE_SOCKET_SERVER]          = "socket_server",
    [MODULE_SOCKET_SERVER_SM]       = "socket_sm",
    [MODULE_SOCKET_CLIENT]          = "socket_client",
    [MODULE_SOCKET_CLIENT_SM]       = "client_sm",
    [MODULE_SERVER_CLIENT_CONTROL]  = "sc_control",
    [MODULE_USER_BUF_MANAGER]       = "user_buf_mgr",
    [MODULE_ION_BUF_MANAGER]        = "ion_buf_mgr",
    [MODULE_SERVER_CB_THREAD]       = "cb_thread",
    [MODULE_RUN_ONCE_THREAD]        = "run_once",
    [MODULE_THREAD_POOL]            = "thread_pool",
    [MODULE_MEMORY_POOL]            = "memory_pool",
    [MODULE_REQUEST_HANDLER]        = "request_handler",
    [MODULE_SIRIUS_CLIENT_CORE]     = "client_core",
    [MODULE_PREVIEW_REQUEST]        = "preview_request",
    [MODULE_YUV_PICTURE_REQUEST]    = "picture_request",
    [MODULE_EVT_REQUEST_SERVER]     = "event_server",
    [MODULE_EVT_REQUEST_CLIENT]     = "event_client",
    [MODULE_UTILS]                  = "utils",
    [MODULE_TOOLS]                  = "tools",
    [MODULE_TESTER]                 = "tester",
    [MODULE_MAX_INVALID]            = "max_invalid",
};

static const char *const ModuleShortName[] = {
    [MODULE_OTHERS]                 = "[OTHER]",
    [MODULE_SIRIUS]                 = "[SIRIU]",
    [MODULE_SIRIUS_IMPL]            = "[ IMPL]",
    [MODULE_SIRIUS_CORE]            = "[ CORE]",
    [MODULE_SOCKET]                 = "[ SOKT]",
    [MODULE_SOCKET_SERVER]          = "[ SSVR]",
    [MODULE_SOCKET_SERVER_SM]       = "[ SSSM]",
    [MODULE_SOCKET_CLIENT]          = "[ SCLI]",
    [MODULE_SOCKET_CLIENT_SM]       = "[ SCSM]",
    [MODULE_SERVER_CLIENT_CONTROL]  = "[SCCTL]",
    [MODULE_USER_BUF_MANAGER]       = "[UBUFM]",
    [MODULE_ION_BUF_MANAGER]        = "[IBUFM]",
    [MODULE_SERVER_CB_THREAD]       = "[CBTHR]",
    [MODULE_RUN_ONCE_THREAD]        = "[ONCET]",
    [MODULE_THREAD_POOL]            = "[THPOL]",
    [MODULE_MEMORY_POOL]            = "[MMPOL]",
    [MODULE_REQUEST_HANDLER]        = "[REQHA]",
    [MODULE_SIRIUS_CLIENT_CORE]     = "[CLCOR]",
    [MODULE_PREVIEW_REQUEST]        = "[ PRER]",
    [MODULE_YUV_PICTURE_REQUEST]    = "[ PICR]",
    [MODULE_EVT_REQUEST_SERVER]     = "[EVTSV]",
    [MODULE_EVT_REQUEST_CLIENT]     = "[EVTCL]",
    [MODULE_UTILS]                  = "[ UTIL]",
    [MODULE_TOOLS]                  = "[TOOLS]",
    [MODULE_TESTER]                 = "[TESTE]",
    [MODULE_MAX_INVALID]            = "[ INVA]",
};

static bool checkValid(ModuleType type)
{
    bool rc = false;

    if (type >= 0 && type < MODULE_MAX_INVALID) {
        rc = true;
    }

    return rc;
}

static ModuleType getValidType(ModuleType type)
{
    return checkValid(type) ? type : MODULE_MAX_INVALID;
}

uint32_t getMaxLenofShortName()
{
    uint32_t result = 0, len = 0;
    for (uint32_t i = 0; i <= MODULE_MAX_INVALID; i++) {
        len = strlen(ModuleShortName[i]);
        result = result < len ? len : result;
    }

    return result;
}

const char *getModuleName(ModuleType type)
{
    return ModuleName[getValidType(type)];
}

const char *getModuleShortName(ModuleType type)
{
    return ModuleShortName[getValidType(type)];
}

};

