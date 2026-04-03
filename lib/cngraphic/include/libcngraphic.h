#ifndef _LIBCNGRAPGIC_H_
    #define _LIBCNGRAPGIC_H_

    #include <glad/gl.h>
    #include <SDL2/SDL.h>
    #include <SDL_image.h>
    #include "libcncore.h"

    #define INVALIDATE_GPU(t)                                                    \
    if ((t)) {                                                                   \
        switch ((t)->api) {                                                      \
            case R_API_SDL:                                                      \
                if ((t)->gpu_handler.sdl_texture.gpu_texture) {                  \
                    SDL_DestroyTexture((t)->gpu_handler.sdl_texture.gpu_texture);\
                    (t)->gpu_handler.sdl_texture.gpu_texture = NULL;             \
                }                                                                \
                break;                                                           \
            case R_API_GL:                                                       \
                if ((t)->gpu_handler.gl_id) {                                    \
                    glDeleteTextures(1, &(t)->gpu_handler.gl_id);                \
                    (t)->gpu_handler.gl_id = 0;                                  \
                }                                                                \
                break;                                                           \
            default:                                                             \
                break;                                                           \
        }                                                                        \
    }

    typedef enum {
        EV_NULL = 0x00,
        EV_CLOSE = 0x01,
        EV_RESIZE = 0x02,
        EV_MOVE = 0x03,
        EV_KEYDOWN = 0x04,
        EV_KEYUP = 0x05,
        EV_MOUSEMOVE = 0x06,
        EV_MOUSEDOWN = 0x07,
        EV_MOUSEUP = 0x08,
        EV_MOUSEWHEEL = 0x09
    } cn_event;

    enum VIDEOMODE_FLAGS {
        VDM_VSYNC = (1 << 0),
        VDM_CLOSABLE = (1 << 1),
        VDM_ACCELERATION = (1 << 2),
        VDM_INACTIVE = (1 << 3),
        VDM_GPU = (1 << 4),
        VDM_OPENGL = (1 << 5),
        VDM_CPU = (1 << 6)
    };

    enum VIDEOMODE_NATIVE_FLAGS {
        VDM_N_FSCRN = SDL_WINDOW_FULLSCREEN,
        VDM_N_OPENGL = SDL_WINDOW_OPENGL,
        VDM_N_SHWN = SDL_WINDOW_SHOWN,
        VDM_N_HDN = SDL_WINDOW_HIDDEN,
        VDM_N_BDLS = SDL_WINDOW_BORDERLESS,
        VDM_N_RSZL = SDL_WINDOW_RESIZABLE,
        VDM_N_MNZ = SDL_WINDOW_MINIMIZED,
        VDM_N_MXZ = SDL_WINDOW_MAXIMIZED,
        VDM_N_MSGBD = SDL_WINDOW_MOUSE_GRABBED,
        VMD_N_INPFOC = SDL_WINDOW_INPUT_FOCUS,
        VMD_N_MSFOC = SDL_WINDOW_MOUSE_FOCUS,
        VDM_N_FSCRND = SDL_WINDOW_FULLSCREEN_DESKTOP,
        VDM_N_MSCAPT = SDL_WINDOW_MOUSE_CAPTURE,
        VDM_N_ALSONTOP = SDL_WINDOW_ALWAYS_ON_TOP,
        VDM_N_SKPTB = SDL_WINDOW_SKIP_TASKBAR,
        VDM_N_WUTIL = SDL_WINDOW_UTILITY,
        VDM_N_WTLTP = SDL_WINDOW_TOOLTIP,
        VDM_N_WPOPMEN = SDL_WINDOW_POPUP_MENU,
        VDM_N_KBGD = SDL_WINDOW_KEYBOARD_GRABBED,
        VDM_N_VULKAN = SDL_WINDOW_VULKAN,
        VDM_N_METAL = SDL_WINDOW_METAL
    };

    typedef enum {
        R_API_NONE = 0x00,
        R_API_SDL,
        R_API_GL,
        R_API_VULKAN,
        R_API_METAL,
        R_API_DX11
    } rendering_api;

    typedef uint32_t cncolor;

    typedef struct {
        cnnumber x, y, z;
        cnnumber nx, ny, nz;
        cnnumber u, v;
    } Vertex;

    struct texture_s {
        Vector2 size;

        SDL_Surface *surface;

        rendering_api api;
        union {
            struct {
                SDL_Texture *gpu_texture;
                const SDL_Renderer *renderer;
            } sdl_texture;
            uint32_t gl_id;
        } gpu_handler;
    };

    typedef struct {
        struct texture_s *texture;
        cncolor color;

        union {
            uint32_t  gl_shader;
        } gpu_handler;

        cnnumber ambient;
        cnnumber diffuse;
        cnnumber specular;
        cnnumber shininess;

        rendering_api api;
    } Material;

    typedef struct {
        Vertex *vertices;
        uint32_t *indices;
        size_t vertex_count;
        size_t index_count;

        rendering_api api;
        cnbool uploaded;
        union {
            struct {
                uint32_t vao;
                uint32_t vbo;
                uint32_t ebo;
            } gl;
        } gpu_handler;
    } Mesh;

    struct videomode_s {
        Vector2 size;
        Vector2 position;

        cnflags flags;
        cnflags native_flags;
    };

    struct event_s {
        cnnumber x;
        cnnumber y;
        cn_event type;
        int64_t v;
    };

    struct event_map_entry_s {
        struct event_s **events;
        cn_event type;

        size_t size;
        size_t capacity;
    };

    struct window_s {
        char *title;
        struct videomode_s video_mode;

        uint32_t id;

        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_GLContext *gl_ctx;
        struct texture_s *texture;

        struct event_map_entry_s **event_map; // null terminated
    };

    struct window_universe_s {
        struct window_s **windows;

        size_t size;
        size_t capacity;
    };

    struct interface_s {
        struct window_s *window;

        struct object_vector_s elements;

        void (*draw)(struct object_s *__this);
        void (*update)(struct object_s *__this, cntime delta_time);
        void (*event)(struct object_s *__this);
    };

    struct texture_atlas_s {
        struct texture_s **content;
        uint64_t *keys; // keys[i] -> content[i]

        size_t size;
        size_t capacity;
    };

    typedef struct texture_s Texture;
    typedef struct window_s Window;
    typedef struct window_universe_s WindowUniverse;
    typedef struct videomode_s Videomode;
    typedef struct event_s Event;
    typedef struct interface_s Interface;
    typedef struct texture_atlas_s TextureAtlas;

    CN_API void blit(const Texture *__src, Texture *__dst, const Rect *__src_rect, const Vector2 *__dest_at);
    CN_API void blit_ratio(const Texture *__src, Texture *__dst, const Rect *__src_rect, const Vector2 *__dest_at, const Vector2 *__ratios);
    CN_API void draw_texture(Texture *__src_texture, SDL_Renderer *__dest_renderer, const Rect *__src_rect, const Vector2 *__dest_at, const Vector2 *__ratios, double __angle);

    CN_API Texture *new_texture(const Vector2 *size, cnbool alpha);
    CN_API cnbool texture_upload_gl(Texture *texture);
    CN_API uint8_t resize_texture(Texture *texture, const Vector2 *new_size);
    CN_API Texture *copy_texture(Texture *texture);
    CN_API Texture *new_texture_from_file(const char *path);
    CN_API Texture *new_texture_from_surface(SDL_Surface *surface);
    CN_API void clear_texture(Texture *texture, cncolor color);
    CN_API void set_opacity_texture(Texture *texture, uint8_t opacity);
    CN_API uint8_t get_opacity_texture(const Texture *texture);
    CN_API void draw_rect(Texture *texture, const Rect *rect, cncolor color);
    // CN_API void draw_ellipse(Texture *texture, const Rect *rect, cncolor color); // to implement
    CN_API void delete_texture(Texture *texture);

    CN_API TextureAtlas *new_texture_atlas(void);
    CN_API const Texture *get_texture(TextureAtlas *atlas, const char *key, Texture *(*tex_from_key)(const char *));
    CN_API void remove_texture_atlas(TextureAtlas *atlas, const char *key);
    CN_API uint8_t add_texture_atlas(TextureAtlas *atlas, Texture *texture, const char *key);
    CN_API uint8_t texture_atlas_resize(TextureAtlas *atlas, size_t new_capacity);
    CN_API void delete_texture_atlas(TextureAtlas *atlas);

    CN_API Window *new_window(const char *name, const Texture *icon, const Videomode *video_mode);
    CN_API void set_vsync_window(Window *window, cnbool value);
    CN_API cnbool get_vsync_window(const Window *window);
    CN_API void set_closable_window(Window *window, cnbool value);
    CN_API cnbool get_closable_window(const Window *window);
    CN_API void set_hidden_window(Window *window, cnbool value);
    CN_API cnbool get_hidden_window(const Window *window);
    CN_API void set_inactive_window(Window *window, cnbool value);
    CN_API cnbool get_inactive_window(const Window *window);
    CN_API void update_window(Window *window);
    CN_API void draw_window(Window *window);
    CN_API cnbool has_event_window(const Window *window, cn_event type);
    CN_API const struct event_map_entry_s *get_event_window(const Window *window, cn_event type);
    CN_API void clear_window(Window *window, cncolor color);
    CN_API uint8_t allow_event(Window *window, cn_event ev);
    CN_API uint8_t push_event_window(Window *window, cn_event type, cnnumber x, cnnumber y, int64_t v);
    CN_API void delete_window(Window *window);

    CN_API WindowUniverse *new_window_universe(void);
    CN_API cnbool are_all_window_closed(const WindowUniverse *universe);
    CN_API cnbool is_window_closed(const WindowUniverse *universe, uint32_t window_id);
    CN_API cnbool is_window_closed_addr(const WindowUniverse *universe, void *p);
    CN_API Window *get_window_in_universe(const WindowUniverse *universe, uint32_t window_id);
    CN_API void clear_events_all_window(WindowUniverse *universe);
    CN_API void fetch_events_all_window(WindowUniverse *universe);
    CN_API void update_all_window(WindowUniverse *universe);
    CN_API void draw_all_window(WindowUniverse *universe);
    CN_API uint8_t add_window_in_universe(WindowUniverse *universe, Window *window);
    CN_API void remove_window_from_universe(WindowUniverse *universe, uint32_t id);
    CN_API uint8_t resize_window_universe(WindowUniverse *universe, size_t new_capacity);
    CN_API void delete_window_universe(WindowUniverse *universe);

    CN_API Event *new_event(cn_event type, cnnumber x, cnnumber y, int64_t v);
    CN_API void delete_event(Event *ev);

    CN_API struct event_map_entry_s *new_event_map(cn_event type);
    CN_API void clear_events_in_map(struct event_map_entry_s *event_map);
    CN_API uint8_t resize_event_map(struct event_map_entry_s *event_map, size_t new_capacity);
    CN_API uint8_t push_event_in_map(struct event_map_entry_s *event_map, cnnumber x, cnnumber y, int64_t v);
    CN_API void delete_event_map(struct event_map_entry_s *event_map);

    CN_API cnbool start_graphics(void);
    CN_API void end_graphics(void);

    CN_API Object *new_interface(void);

    CN_API Object *new_graphic_submodule(void);

    CN_API Mesh *new_mesh(void);
    CN_API void mesh_draw_gl(const Mesh *m);
    CN_API cnbool mesh_upload_gl(Mesh *m);
    CN_API void delete_mesh(Mesh *mesh);

    CN_API Material *new_material(void);
    CN_API void material_use_gl(const Material *mat,
                  const float model[16],
                  const float view[16],
                  const float proj[16]);
    CN_API void delete_material(Material *material);

    CN_API GLuint gl_shader_compile(const char *vert_src, const char *frag_src);
    CN_API GLuint gl_shader_load(const char *vert_path, const char *frag_path);

#endif
