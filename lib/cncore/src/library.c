#include "libcncore.h"

#if defined(_WIN32)
    #include <windows.h>

    CN_API void *cnopen_library(const char *path)
    {
        HMODULE h = LoadLibraryA(path);

        return ((void *)h);
    }

    CN_API void *cnget_symbol(void *handle, const char *name)
    {
        FARPROC sym = GetProcAddress((HMODULE)handle, name);

        return ((void *)sym);
    }

    CN_API void cnclose_library(void *handle)
    {
        if (handle)
            (void)FreeLibrary((HMODULE)handle);
    }

#else
    #include <dlfcn.h>

    CN_API void *cnopen_library(const char *path)
    {
        dlerror();

        void *h = dlopen(path, RTLD_NOW | RTLD_LOCAL);

        return (h);
    }

    CN_API void *cnget_symbol(void *handle, const char *name)
    {
        dlerror();

        void *sym = dlsym(handle, name);

        const char *err = dlerror();
        if (err)
        {
            return (NULL);
        }

        return (sym);
    }

    CN_API void cnclose_library(void *handle)
    {
        if (handle)
            dlclose(handle);
    }
#endif
