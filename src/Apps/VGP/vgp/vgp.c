#include <vgp_config.h>
#include <vgp.h>
#include <vgp_error.h>
#include <vgp_runtime.h>
#include <m3_env.h>
#include <stdlib.h>

static const char* ERR_NEW_ENV_FAILED = "m3_NewEnvironment failed";
static const char* ERR_NEW_RUNTIME_FAILED = "m3_NewRuntime failed";
static const char* ERR_VINIT_FUNC_NOT_FOUND = "'vinit' function not found";
static const char* ERR_VLOOP_FUNC_NOT_FOUND = "'vloop' function not found";
static const char* ERR_NOT_INITED = "vgp not inited";

// Global Status
static IM3Environment env = NULL;
static IM3Runtime runtime = NULL;
static IM3Function vloop_func = NULL;
static uint8_t * mem = NULL;
static uint32_t mem_size = 0;

// Export Functions
bool vgp_init(uint8_t* wasm, uint32_t fsize) {
    M3Result result = m3Err_none;
    // new runtime
    env = m3_NewEnvironment();
    if (env == NULL) {
        __vgp_set_last_error(ERR_NEW_ENV_FAILED);
        return false;
    };
    runtime = m3_NewRuntime(env, WASM_STACK_SIZE, NULL);
    if (runtime == NULL) {
        __vgp_set_last_error(ERR_NEW_RUNTIME_FAILED);
        return false;
    };
    // parse modules
    IM3Module module;
    result = m3_ParseModule (env, &module, wasm, fsize);
    __ensure_m3_result(result, false);
    // init memory
    if (module->memoryImported) {
        uint32_t maxPages = module->memoryInfo.maxPages;
        runtime->memory.maxPages = maxPages ? maxPages : 65536;
        runtime->memory.maxPages = module->memoryInfo.maxPages;
        result = ResizeMemory(runtime, module->memoryInfo.initPages);
        __ensure_m3_result(result, false);
    }
    // load module
    result = m3_LoadModule (runtime, module);
    __ensure_m3_result(result, false);
    // link function
    result = __vgp_link_runtime(module);
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
    func = NULL;
    result = m3_FindFunction(&func, runtime, "vinit");
    if (func) {
        result = m3_CallV(func);
        __ensure_m3_result(result, false);
    } else {
        if (result == m3Err_functionLookupFailed) {
            __vgp_set_last_error(ERR_VINIT_FUNC_NOT_FOUND);
        } else {
            __vgp_set_last_error(result);
        }
        return false;
    }
    // cache vloop function
    vloop_func = NULL;
    result = m3_FindFunction(&vloop_func, runtime, "vloop");
    if (!vloop_func) {
        if (result == m3Err_functionLookupFailed) {
            __vgp_set_last_error(ERR_VLOOP_FUNC_NOT_FOUND);
        } else {
            __vgp_set_last_error(result);
        }
        return false;
    }
    return true;
}

void vgp_destory(void) {
    vloop_func = NULL;
    if (runtime) {
        m3_FreeRuntime(runtime);
        runtime = NULL;
    }
    if (env) {
        m3_FreeEnvironment(env);
        env = NULL;
    }
}

bool vgp_loop_once(void) {
    if (env && runtime && vloop_func) {
        M3Result result = m3_CallV(vloop_func);
        __ensure_m3_result(result, false);
        return true;
    } else {
        __vgp_set_last_error(ERR_NOT_INITED);
        return false;
    }
}

void vgp_get_wasm_error(M3ErrorInfo *o_info) {
    if (runtime != NULL) {
        m3_GetErrorInfo(runtime, o_info);
    }
}

uint8_t * vgp_get_mem_start(void) {
    return mem;
}

uint8_t * vgp_get_mem_end(void) {
    return mem + mem_size;
}

// internal function
void vgp_get_memory(void) {
    if (runtime != NULL) {
        mem = m3_GetMemory(runtime, &mem_size, 0);
    }
}

uint8_t * __vpointer_to_real(uint32_t vp) {
    if (runtime != NULL) {
        vgp_get_memory();
        if (vp >= mem_size) {
            return NULL; // out of range
        }
        return mem + vp;
    }
    return NULL;
}
