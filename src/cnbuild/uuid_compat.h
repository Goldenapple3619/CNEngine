#ifndef _UUID_COMPAT_H_
    #define _UUID_COMPAT_H_

    #if defined(_WIN32)
        #include <rpc.h>
        #include <rpcdce.h>
        #pragma comment(lib, "Rpcrt4.lib")

        typedef unsigned char uuid_t[16];

        static inline void uuid_generate(uuid_t out) {
            UUID w;
            UuidCreate(&w);
            memcpy(out, &w, 16);
        }

        static inline void uuid_unparse_lower(const uuid_t uu, char *out) {
            UUID *w = (UUID *)uu;
            unsigned char *p;
            UuidToStringA(w, &p);
            strncpy(out, (char *)p, 36);
            out[36] = '\0';
            RpcStringFreeA(&p);
        }

    #elif defined(__APPLE__)
        #include <CoreFoundation/CFUUID.h>
        #include <string.h>

        typedef unsigned char uuid_t[16];

        static inline void uuid_generate(uuid_t out) {
            CFUUIDRef u = CFUUIDCreate(NULL);
            CFUUIDBytes b = CFUUIDGetUUIDBytes(u);
            CFRelease(u);
            memcpy(out, &b, 16);
        }

        static inline void uuid_unparse_lower(const uuid_t uu, char *out) {
            snprintf(out, 37,
                "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                uu[0],uu[1],uu[2],uu[3], uu[4],uu[5], uu[6],uu[7],
                uu[8],uu[9], uu[10],uu[11],uu[12],uu[13],uu[14],uu[15]);
        }

    #else
        #include <uuid/uuid.h>
    #endif
#endif