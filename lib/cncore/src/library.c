#include "libcncore.h"

#if defined(_WIN32)
    #include <windows.h>

    CN_API void *cnopen_library(const char *path)
    {
        if (!path) {
            RAISE(ERR_INVALID_POINTER, "openlibrary with empty name argument.");
            return (NULL);
        }
    
        HMODULE h = LoadLibraryA(path);

        if (!h) {
            RAISE_FMT(ERR_OS, "failed to open library '%s'.", path);
            return (NULL);
        }

        return ((void *)h);
    }

    CN_API void *cnget_symbol(void *handle, const char *name)
    {
        if (!handle) {
            RAISE(ERR_INVALID_POINTER, "getsymbol on empty library pointer.");
            return (NULL);
        }

        if (!name) {
            RAISE(ERR_INVALID_POINTER, "getsymbol with empty name argument.");
            return (NULL);
        }
    
        FARPROC sym = GetProcAddress((HMODULE)handle, name);

        if (!sym) {
            RAISE_FMT(ERR_OS, "failed to extract symbol '%s'.", name);
            return (NULL);
        }

        return ((void *)sym);
    }

    CN_API void cnclose_library(void *handle)
    {
        if (!handle) {
            RAISE(ERR_INVALID_POINTER, "close on empty library pointer.");
            return;
        }
        (void)FreeLibrary((HMODULE)handle);
    }

#else
    #include <dlfcn.h>

    CN_API void *cnopen_library(const char *path)
    {
        if (!path) {
            RAISE(ERR_INVALID_POINTER, "openlibrary with empty name argument.");
            return (NULL);
        }
        dlerror();

        void *h = dlopen(path, RTLD_NOW | RTLD_LOCAL);

        if (!h) {
            const char *err = dlerror();
    
            RAISE_FMT(ERR_OS, "failed to openlibrary '%s' (%s).", path, err);
            return (NULL);
        }

        return (h);
    }

    CN_API void *cnget_symbol(void *handle, const char *name)
    {
        if (!handle) {
            RAISE(ERR_INVALID_POINTER, "getsymbol on empty library pointer.");
            return (NULL);
        }

        if (!name) {
            RAISE(ERR_INVALID_POINTER, "getsymbol with empty name argument.");
            return (NULL);
        }

        dlerror();

        void *sym = dlsym(handle, name);

        const char *err = dlerror();
        if (!sym || err) {
            RAISE_FMT(ERR_OS, "failed to getsymbol '%s' (%s).", name, err);
            return (NULL);
        }

        return (sym);
    }

    CN_API void cnclose_library(void *handle)
    {
        if (!handle) {
            RAISE(ERR_INVALID_POINTER, "close on empty library pointer.");
            return;
        }

        (void)dlclose(handle);
    }
#endif
