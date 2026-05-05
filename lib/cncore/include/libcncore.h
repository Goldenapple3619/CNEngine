#ifndef _LIBCNCORE_H_
    #define _LIBCNCORE_H_

    #include <SDL2/SDL.h>

    #ifdef _WIN32
        #define CN_API __declspec(dllexport)
    #else
        #define CN_API
    #endif

    #define GC_MAX_SIZE 256

    #define true ~(0 << 1)
    #define false 0

    #define STRING_INDIVIDUAL_ALLOCATION 1 // are we duping every string ? or are they handled with an atlas

    #define PREP_INIT() void *__temp_alloc;
    #define PREP_CLASS_BUILD() PREP_INIT()
    #define INIT_CUSTOM_ALLOCATION(__this, expr_alloc, expr_free, name) \
        __temp_alloc = (void *)expr_alloc; \
        if (!__temp_alloc) \
            return (VALUE_ERR); \
        if (!set_attr(__this, name, CN_TYPE_GENERIC_UNIQ_PTR, (cnany)__temp_alloc)) { \
            (void)expr_free(__temp_alloc); \
            return (VALUE_ERR); \
        }
    #define INIT_OBJECT_STATIC(__this, expr_alloc, args, name) \
        __temp_alloc = (void *)build_object(expr_alloc, args); \
        if (!__temp_alloc) \
            return (VALUE_ERR); \
        if (!set_attr(__this, name, CN_TYPE_OBJECT, (cnany)__temp_alloc)) { \
            (void)delete_object(__temp_alloc); \
            return (VALUE_ERR); \
        }
    #define INIT_OBJECT_SHR(__this, obj, name) \
        if (!obj) \
            return (VALUE_ERR); \
        if (!set_attr(__this, name, CN_TYPE_OBJECT, (cnany)obj)) { \
            return (VALUE_ERR); \
        }
    #define INIT_OBJECT_SHR_WEAK(__this, obj, name) \
        if (!obj) \
            return (VALUE_ERR); \
        if (!set_attr(__this, name, CN_TYPE_WEAK_OBJECT, (cnany)obj)) { \
            return (VALUE_ERR); \
        }
    #define PREP_DEL() cn_value *__temp_alloc;
    #define DEL_CUSTOM_ALLOCAION(__this, expr_free, name) \
        __temp_alloc = get_attr(__this, name); \
        if (__temp_alloc && __temp_alloc->as.ptr) { \
            expr_free(__temp_alloc->as.ptr); \
            __temp_alloc->as.ptr = NULL; \
        }
    #define INIT_STRING(__this, string, name) \
        if (!set_attr(__this, name, CN_TYPE_STRING, (cnany)string)) \
            return (VALUE_ERR);
    #define INIT_INT(__this, number, name) \
        if (!set_attr(__this, name, CN_TYPE_INT, (cnany)((int64_t [1]){number}))) \
            return (VALUE_ERR);
    #define INIT_NUMBER(__this, number, name) \
        if (!set_attr(__this, name, CN_TYPE_NUMBER, (cnany)((cnnumber [1]){number}))) \
            return (VALUE_ERR);
    #define INIT_VEC2(__this, vec2, name) \
        if (!set_attr(__this, name, CN_TYPE_VEC2, (cnany)(&vec2))) \
            return (VALUE_ERR);
    #define INIT_VEC3(__this, vec3, name) \
        if (!set_attr(__this, name, CN_TYPE_VEC3, (cnany)(&vec3))) \
            return (VALUE_ERR);
    #define INIT_RECT(__this, rect, name) \
        if (!set_attr(__this, name, CN_TYPE_RECT, (cnany)(&rect))) \
            return (VALUE_ERR);
    #define INIT_FLOAT(__this, number, name) \
        if (!set_attr(__this, name, CN_TYPE_FLOAT, (cnany)((double [1]){number}))) \
            return (VALUE_ERR);
    #define INIT_METHOD(__this, name, callback) \
        if (!set_method(__this, name, callback)) \
            return (VALUE_ERR);
    #define CREATE_METHOD_CLASS_BUILD(__class, name, callback) \
        if (!set_method(__class, name, callback)) { \
            (void)delete_object(__class); \
            return (NULL); \
        }
    #define SET_PARENT_CLASS_BUILD(__class, __parent) \
        __class->base = __parent; \
        if (!__class->base) { \
            (void)delete_object(__class); \
            return (NULL); \
        }
    #define CREATE_CUSTOM_ALLOCATION_CLASS_BUILD(__class, expr_alloc, expr_free, name) \
        __temp_alloc = expr_alloc; \
        if (!__temp_alloc) { \
            (void)delete_object(__class); \
            return (NULL); \
        } \
        if (!set_attr(__class, name, CN_TYPE_GENERIC_UNIQ_PTR, (cnany)__temp_alloc)) { \
            (void)expr_free(__temp_alloc); \
            (void)delete_object(__class); \
            return (NULL); \
        }

    #define PACK_ARG(...) (cnany []){ __VA_ARGS__ }
    #define INLNE_PRIM_T_ARG(number) ((typeof((number)) [1]){(number)})
    #define INLN_STRCT_T_ARG(constructor) (&(constructor))

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
        double last_dt;
    };

    typedef enum {
        CN_TYPE_NULL,
        CN_TYPE_INT,
        CN_TYPE_FLOAT,
        CN_TYPE_NUMBER,
        CN_TYPE_VEC2,
        CN_TYPE_VEC3,
        CN_TYPE_RECT,
        CN_TYPE_BOOL,
        CN_TYPE_STRING,
        CN_TYPE_OBJECT,
        CN_TYPE_WEAK_OBJECT,
        CN_TYPE_FUNCTION,
        CN_TYPE_GENERIC_UNIQ_PTR // custom things that may be handled by user in the dtor
    } cn_type;

    typedef enum {
        CN_OBJ_NULL = 0x00,
        CN_OBJ_DRAWABLE = (1 << 0),
        CN_OBJ_REPLICATE = (1 << 1),
        CN_OBJ_HOST = (1 << 2)
    } scene_object_flags;

    typedef struct {
        cn_type type;
        union {            
            cnbool b;

            int64_t i;
            double f;
            cnnumber num;
            
            char *str;
            void *ptr;

            struct vector2_s vec2;
            struct vector3_s vec3;
            struct rect_s rect;
        } as;
    } cn_value;

    struct object_s;

    #define null_value (cn_value){CN_TYPE_NULL, .as.i = 0}
    #define VALUE_ERR (cn_value){CN_TYPE_INT, .as.i = 1}
    #define VALUE_OK (cn_value){CN_TYPE_INT, .as.i = 0}
    typedef cn_value (*cn_method)(struct object_s *self, void **args);

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

    struct cn_value_vector_s {
        cn_value **values;

        size_t size;
        size_t capacity;
    };

    struct scene_object_mode_s {
        struct vector3_s coords;
        struct vector3_s scale;
        struct vector3_s rotation;
        scene_object_flags flags;
    };

    struct list_iterator_s {
        size_t pos;
        size_t size;
        cn_method get_element;
        cn_value val;
        struct object_s *_obj;
    };

    struct generic_map_s {
        void **content;
        uint64_t *keys; // keys[i] -> content[i]

        size_t size;
        size_t capacity;
    };

    struct generic_vector_s {
        void **content;

        size_t size;
        size_t capacity;
    };

    typedef struct {
        struct object_s *obj;
        cn_method method;
    } ObjMethodPair;

    typedef struct vector2_s Vector2;
    typedef struct vector3_s Vector3;
    typedef struct rect_s Rect;
    typedef struct clock_s Clock;
    typedef struct object_s Object;
    typedef struct object_attribute_s OBJAttrib;
    typedef struct object_vector_s ObjectVector;

    /******************************************************************************
     * create a new clock
     *
     * this function is used to allocate a new clock
     *
     * @return a pointer to a new clock that you must free yourself
     * @see delete_clock
     ******************************************************************************/
    CN_API Clock *new_clock(void);
    /******************************************************************************
     * regulate ticking using a clock based on set TPS (tick per secondes)
     *
     * this function is used to regulate the tick time/frame rate for main loops
     *
     * @param c a pointer to a valid clock
     * @param tps tick per second or frame per second
     * @return time that passed between last tick and actual tick (aka: delta time) in ms
     ******************************************************************************/
    CN_API double clock_tick(Clock *c, int32_t tps);
    /******************************************************************************
     * delete a clock
     *
     * this function is used to free an allocated clock created using new_clock
     *
     * @param c a pointer to a valid clock allocated with new_clock
     * @see new_clock
     ******************************************************************************/
    CN_API void delete_clock(Clock *c);

    CN_API Rect *new_rect(cnnumber x, cnnumber y, cnnumber w, cnnumber h);
    CN_API void delete_rect(Rect *r);

    CN_API Vector2 *new_vector2(cnnumber x, cnnumber y);
    CN_API Vector2 add_vector2(const Vector2 *vec0, const Vector2 *vec1);
    CN_API void delete_vector2(Vector2 *v);

    CN_API Vector3 *new_vector3(cnnumber x, cnnumber y, cnnumber z);
    CN_API void delete_vector3(Vector3 *v);
    
    CN_API OBJAttrib *create_object_attribute(const char *name, cn_type type, cnany value);
    CN_API OBJAttrib *create_object_attribute_from_cnvalue(const char *name, const cn_value *value);
    CN_API void delete_object_attribute(OBJAttrib *attribute);
    
    CN_API Object *new_object(void);
    CN_API Object *build_object(Object *obj, void **args);
    CN_API Object *share_object(Object *object);
    CN_API void collect_object(Object *object);
    CN_API void run_gc(void);
    CN_API cnbool set_attr(Object *object, const char *name, cn_type type, cnany value);
    CN_API cn_value *get_attr(const Object *object, const char *name);
    CN_API cnbool has_attr(const Object *object, const char *name);
    CN_API cnbool set_method(Object *object, const char *name, cn_method func);
    CN_API cn_method get_method(const Object *object, const char *name);
    CN_API cn_value *get_method_holder(const Object *object, const char *name);
    CN_API cn_value call_method(Object *object, const char *name, void **args);
    CN_API void print_object(const Object *object);
    CN_API Object *create_default_object(void);
    CN_API cnbool has_method(const Object *object, const char *name);
    CN_API Object *release_object(Object *object);
    CN_API void delete_object(Object *object);

    CN_API ObjectVector *new_object_vector(void);
    CN_API void delete_object_vector(ObjectVector *vec);
    CN_API uint8_t resize_object_vector(ObjectVector *vec, size_t new_capacity);
    CN_API uint8_t insert_object_vector(ObjectVector *vec, Object *obj);
    CN_API void remove_object_ordered_vector(ObjectVector *vec, size_t i);
    void remove_object_vector(ObjectVector *vec, size_t i);

    CN_API struct generic_map_s *new_generic_map(void);
    CN_API uint8_t generic_map_resize(struct generic_map_s *gen_map, size_t new_capacity);
    CN_API uint8_t add_generic_map(struct generic_map_s *gen_map, void *element, const char *key, void (*_delete_obj)(void *));
    CN_API void remove_generic_map(struct generic_map_s *gen_map, const char *key, void (*_delete_obj)(void *));
    CN_API const void *get_generic_map(struct generic_map_s *gen_map, const char *key, void *(*_obj_from_key_default)(const char *), void (*_delete_obj)(void *));
    CN_API void delete_generic_map(struct generic_map_s *gen_map, void (*_delete_obj)(void *));

    CN_API struct generic_vector_s *new_generic_vector(void);
    CN_API uint8_t resize_generic_vector(struct generic_vector_s *vec, size_t new_capacity);
    CN_API uint8_t insert_generic_vector(struct generic_vector_s *vec, void *obj);
    CN_API void remove_generic_vector(struct generic_vector_s *vec, size_t i, void (*_delete_obj)(void *));
    CN_API void remove_generic_ordered_vector(struct generic_vector_s *vec, size_t i, void (*_delete_obj)(void *));
    CN_API void delete_generic_vector(struct generic_vector_s *vec, void (*_delete_obj)(void *));

    CN_API Object *new_ctx(void);
    CN_API cnbool submodule_ctx(Object *ctx, Object *module);

    void _init_attribute_value(cn_value *dest, cnany value);
    void _delete_object_attribute_value(cn_value *val);
    void *_attribute_value_extract(const cn_value *src);

    void _init_object_attrs(struct attr_map_s *attribute_map);
    CN_API uint64_t _get_attrs_hash(const char *str);
    OBJAttrib *_find_object_attrs(const struct attr_map_s *attribute_map, uint64_t k);
    void _remove_object_attrs(struct attr_map_s *attribute_map, uint64_t k);
    uint8_t _insert_object_attrs(struct attr_map_s *attribute_map, uint64_t k, OBJAttrib *attr);
    uint8_t _attr_map_resize(struct attr_map_s *map, size_t new_capacity);
    void _delete_object_attrs(struct attr_map_s *attribute_map);

    CN_API struct cn_value_vector_s *new_value_vector(void);
    CN_API uint8_t resize_value_vector(struct cn_value_vector_s *vec, size_t new_capacity);
    CN_API uint8_t insert_value_vector(struct cn_value_vector_s *vec, cn_value value);
    CN_API void remove_value_vector(struct cn_value_vector_s *vec, size_t i);
    CN_API void remove_value_ordered_vector(struct cn_value_vector_s *vec, size_t i);
    CN_API void delete_value_vector(struct cn_value_vector_s *vec);

    CN_API struct list_iterator_s list_get_iterator(Object *__list);
    CN_API void list_iterator_next(struct list_iterator_s *iterator);
    CN_API cnbool list_iterator_isend(const struct list_iterator_s *iterator);
    CN_API cnbool list_iterator_value_isnull(const struct list_iterator_s *iterator);

    CN_API struct list_iterator_s atlas_get_iterator(Object *__atlas, cnbool get_value_instead_of_key);
    CN_API void atlas_iterator_next(struct list_iterator_s *iterator);
    CN_API cnbool atlas_iterator_value_isnull(const struct list_iterator_s *iterator);
    CN_API cnbool atlas_iterator_isend(const struct list_iterator_s *iterator);

    CN_API ObjMethodPair *new_object_method_pair(Object *obj, cn_method method);
    CN_API ObjMethodPair *new_weak_object_method_pair(Object *obj, cn_method method);
    CN_API void delete_object_method_pair(ObjMethodPair *pair);
    CN_API void delete_weak_object_method_pair(ObjMethodPair *pair);

    CN_API Object *new_list(void);
    CN_API Object *new_scene(void);
    CN_API Object *new_scene_object(void);
    CN_API Object *new_atlas(void *(*_fetch_default)(const char *), void (*_delete_obj)(void *));

    CN_API cnbool start_core(void);
    CN_API void end_core(void);

#endif
