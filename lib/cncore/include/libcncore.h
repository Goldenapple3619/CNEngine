#ifndef _LIBCNCORE_H_
    #define _LIBCNCORE_H_

    #include <SDL2/SDL.h>

    #ifdef _WIN32
        #define CN_API __declspec(dllexport)
    #else
        #define CN_API
    #endif

    #define true ~(0 << 1)
    #define false 0

    typedef float cnnumber;
    // typedef double cnnumber;
    
    typedef void * cnany;
    typedef uint64_t cntime;
    typedef unsigned char cnbool;
    typedef uint64_t cnflags;

    struct vector2_s {
        cnnumber x;
        cnnumber y;
    };

    struct vector3_s {
        cnnumber x;
        cnnumber y;
        cnnumber z;
    };
    
    struct rect_s {
        cnnumber x;
        cnnumber y;
        cnnumber w;
        cnnumber h;
    };

    struct clock_s {
        cntime old_time;
        cntime last_dt;
    };

    typedef enum {
        CN_TYPE_INT,
        CN_TYPE_FLOAT,
        CN_TYPE_STRING,
        CN_TYPE_OBJECT,
        CN_TYPE_FUNCTION,
        CN_TYPE_GENERIC_UNIQ_PTR
    } cn_type;

    typedef struct {
        cn_type type;
        union {
            int64_t i;
            double f;
            char *str;
            void *ptr;
        } as;
    } cn_value;

    struct object_attribute_s {
        char *name;
        cn_value value;
    };

    struct attr_map_s {
        struct object_attribute_s **attrs;
        uint64_t *keys;
    
        size_t size;
        size_t capacity;
    };

    struct object_s {
        char *name;

        struct object_s *base;

        struct attr_map_s attrs;
        struct attr_map_s methods;

        size_t ref_count;
    };

    struct object_vector_s {
        Object **objects;

        size_t size;
        size_t capacity;
    };

    typedef struct vector2_s Vector2;
    typedef struct vector3_s Vector3;
    typedef struct rect_s Rect;
    typedef struct clock_s Clock;
    typedef struct object_s Object;

    CN_API Clock *new_clock(void);
    CN_API cnnumber clock_tick(Clock *c, int32_t tps);
    CN_API void delete_clock(Clock *c);

    CN_API Rect *new_rect(cnnumber x, cnnumber y, cnnumber w, cnnumber h);
    CN_API void delete_rect(Rect *r);

    CN_API Vector2 *new_vector2(cnnumber x, cnnumber y);
    CN_API void delete_vector2(Vector2 *v);

    CN_API Vector3 *new_vector3(cnnumber x, cnnumber y, cnnumber z);
    CN_API void delete_vector3(Vector3 *v);
#endif
