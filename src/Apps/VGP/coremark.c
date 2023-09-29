#include <vgp_error.h>
#include <vgp_runtime.h>
#include <m3_env.h>

static const char* ERR_NEW_ENV_FAILED = "m3_NewEnvironment failed";
static const char* ERR_NEW_RUNTIME_FAILED = "m3_NewRuntime failed";
static const char* ERR_NOT_INITED = "vgp not inited";

// Global Status
static IM3Environment env = NULL;
static IM3Runtime runtime = NULL;


#include <wasm3.h>
#include <llapi.h>
m3ApiRawFunction(__clock_ms) {
    m3ApiReturnType(uint32_t);
    uint32_t value = llapi_get_tick_ms();
    m3ApiReturn(value);
}

M3Result __m_link_runtime(IM3Module module) {
    M3Result result = m3Err_none;
    result = m3_LinkRawFunction(module, "env", "clock_ms", "i()", __clock_ms);
    if (result) DEBUG_PRINTF("link function clock_ms: %s", result);
    return m3Err_none;
}

// Export Functions
bool m_init(uint8_t* wasm, uint32_t fsize) {
    M3Result result = m3Err_none;
    // new runtime
    env = m3_NewEnvironment();
    runtime = m3_NewRuntime(env, WASM_STACK_SIZE, NULL);
    // parse modules
    IM3Module module;
    result = m3_ParseModule (env, &module, wasm, fsize);
    __ensure_m3_result(result, false);
    // init memory
    if (module->memoryImported) {
        uint32_t maxPages = module->memoryInfo.maxPages;
        runtime->memory.maxPages = maxPages ? maxPages : 65536;
        runtime->memory.maxPages = module->memoryInfo.maxPages;
        printf("init memory max %lu\n", maxPages);
        printf("init memory init %lu\n", module->memoryInfo.initPages);
        result = ResizeMemory(runtime, module->memoryInfo.initPages);
        __ensure_m3_result(result, false);
    }
    // load module
    result = m3_LoadModule (runtime, module);
    __ensure_m3_result(result, false);
    // link function
    result = __m_link_runtime(module);
    __ensure_m3_result(result, false);
    // call start function
    result = m3_RunStart(module);
    __ensure_m3_result(result, false);
    M3Function* func = NULL;
    m3_FindFunction(&func, runtime, "_initialize");
    if (func) {
        result = m3_CallV(func);
        __ensure_m3_result(result, false);
    }
    func = NULL;
    m3_FindFunction(&func, runtime, "_start");
    if (func) {
        result = m3_CallV(func);
        __ensure_m3_result(result, false);
    }
    // call vinit function
    printf("Running CoreMark 1.0...\n");
    func = NULL;
    result = m3_FindFunction(&func, runtime, "run");
    if (func) {
        result = m3_CallV(func);
        __ensure_m3_result(result, false);
        f32 value = 0;
        result = m3_GetResultsV (func, &value);
        printf("CoreMark Result: %f\n", value);
    } else {
        return false;
    }
}

void m_destory(void) {
    if (runtime) {
        m3_FreeRuntime(runtime);
        runtime = NULL;
    }
    if (env) {
        m3_FreeEnvironment(env);
        env = NULL;
    }
}

void m_get_wasm_error(M3ErrorInfo *o_info) {
    if (runtime != NULL) {
        m3_GetErrorInfo(runtime, o_info);
    }
}


#include <llapi.h>
#include "coremark_minimal.wasm.h"
void app_run_wasm_coremark(void) {
    printf("======== Start ========\n\n\n");
    
    // init
    bool successed = m_init(coremark_minimal_wasm, coremark_minimal_wasm_len);
    if (!successed) {
        DEBUG_PRINTF("init: %s", vgp_get_last_error());
    }
    // deinit
    m_destory();
}
