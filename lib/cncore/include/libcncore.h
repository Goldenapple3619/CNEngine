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

    #define STRING_INDIVIDUAL_ALLOCATION 1 // are we duping every string ? or are they handled with an atlas

    typedef float cnnumber; // less memory, more performance but less accuracy and capacity
    // typedef double cnnumber;
    
    typedef void * cnany;
    typedef uint64_t cntime; // timestamp
    typedef unsigned char cnbool;
    typedef uint64_t cnflags; // 64bits falgs

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
        cntime old_time; // sdl_gettick64()
        cntime last_dt;
    };

    typedef enum {
        CN_TYPE_INT,
        CN_TYPE_FLOAT,
        CN_TYPE_STRING,
        CN_TYPE_OBJECT,
        CN_TYPE_FUNCTION,
        CN_TYPE_GENERIC_UNIQ_PTR // custom things that may be handled by user in the dtor
    } cn_type;

    typedef struct {
        cn_type type;
        union {
            int64_t i;
            double f;
            char *str;
            void *ptr; // every possible value entries for our types, prevent over allocation
        } as;
    } cn_value;

    struct object_attribute_s {
        #ifdef STRING_INDIVIDUAL_ALLOCATION
            char *name;
        #else
            const char *name;
        #endif
        cn_value value;
    };

    struct attr_map_s {
        struct object_attribute_s **attrs;
        uint64_t *keys; // keys[i] -> attrs[i]
    
        size_t size;
        size_t capacity;
    };

    struct object_s {
        struct object_s *base;

        struct attr_map_s attrs;
        struct attr_map_s methods;

        size_t ref_count;
    };

    struct object_vector_s {
        struct object_s **objects;

        size_t size;
        size_t capacity;
    };

    typedef struct vector2_s Vector2;
    typedef struct vector3_s Vector3;
    typedef struct rect_s Rect;
    typedef struct clock_s Clock;
    typedef struct object_s Object;
    typedef struct object_attribute_s OBJAttrib;

    CN_API Clock *new_clock(void);
    CN_API cnnumber clock_tick(Clock *c, int32_t tps);
    CN_API void delete_clock(Clock *c);

    CN_API Rect *new_rect(cnnumber x, cnnumber y, cnnumber w, cnnumber h);
    CN_API void delete_rect(Rect *r);

    CN_API Vector2 *new_vector2(cnnumber x, cnnumber y);
    CN_API void delete_vector2(Vector2 *v);

    CN_API Vector3 *new_vector3(cnnumber x, cnnumber y, cnnumber z);
    CN_API void delete_vector3(Vector3 *v);
    
    CN_API OBJAttrib *create_object_attribute(const char *name, cn_type type, cnany value);
    CN_API void delete_object_attribute(OBJAttrib *attribute);
    
    CN_API Object *new_object(void);
    CN_API Object *share_object(Object *object);
    CN_API Object *release_object(Object *object);
    CN_API void delete_object(Object *object);

    void _init_attribute_value(cn_value *dest, cnany value);
    void _delete_object_attribute_value(cn_value *val);

    void _init_object_attrs(struct attr_map_s *attribute_map);
    void _delete_object_attrs(struct attr_map_s *attribute_map);

#endif
