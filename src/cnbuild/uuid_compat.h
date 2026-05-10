#ifndef _UUID_COMPAT_H_
    #define _UUID_COMPAT_H_

    #if defined(_WIN32)
        #include <rpc.h>

        #ifdef _MSC_VER
            #pragma comment(lib, "Rpcrt4.lib")
        #endif

        typedef unsigned char cn_uuid_bytes[16];

        static inline void uuid_generate_random(cn_uuid_bytes out) {
            UUID w;
            UuidCreate(&w);
            memcpy(out, &w, sizeof(UUID));
        }

        static inline void uuid_unparse_lower(const cn_uuid_bytes uu, char *out) {
            UUID w;
            RPC_CSTR p;
            memcpy(&w, uu, sizeof(UUID));
            UuidToStringA(&w, &p);
            strncpy(out, (char *)p, 36);
            out[36] = '\0';
            RpcStringFreeA(&p);
        }

        #ifdef uuid_t
            #undef uuid_t
        #endif
        #define uuid_t cn_uuid_bytes

    #elif defined(__APPLE__)
        #include <CoreFoundation/CFUUID.h>
        #include <string.h>

        typedef unsigned char uuid_t[16];

        static inline void uuid_generate_random(uuid_t out) {
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