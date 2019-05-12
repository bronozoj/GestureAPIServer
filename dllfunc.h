#ifndef __DLL_FUNCTION_POINTERS_H__
#define __DLL_FUNCTION_POINTERS_H__

namespace imustd{
    typedef struct __driver_data_query{
        const char *name;
        const char *type;
    } driver_attribute;
    typedef const char* (__cdecl *versionString) (void);
    typedef uint16_t (__cdecl *getSettings)(driver_attribute ***val);
    typedef int (__cdecl *getErrno)();
    typedef bool (__cdecl *settingString)(char *name, char *value);
    typedef bool (__cdecl *settingValues)(char *name, void *value);
    typedef bool (__cdecl *commandDriver)(void);
    typedef bool (__cdecl *dataGrabber)(double *output);
    typedef struct __driver_subset_functions{
        const settingValues getter;
        const settingValues setter;
    } driver_setting_functions;
}

#endif