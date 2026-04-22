#include "libr3d.h"

static cn_value _init(Object *__this, void **args)
{
    (void)args;

    PREP_INIT()

    call_method(__this->base, "_init", args);

    // temporary testing purpose
    Material *mat = new_material();
    if (mat) {
        mat->color = 0x0000ff00;
        mat->shader = new_shader(); // <- not freed
        (void)gl_shader_load(mat->shader,
            "assets/shaders/default.vert",
            "assets/shaders/default.frag"
        ); // <- not freed
        mat->texture = new_texture_from_file("assets/dirt.png"); // <- not freed also
        if (mat->texture)
            mat->texture->api = R_API_GL; 
    }
    INIT_CUSTOM_ALLOCATION(__this, mat, delete_material, "material");

    Mesh *mesh = new_mesh();

    if (mesh) {
        mesh->vertices = malloc(24 * sizeof(Vertex));
        mesh->indices  = malloc(36 * sizeof(uint32_t));

        if (mesh->vertices && mesh->indices) {
            mesh->vertex_count = 24;
            mesh->index_count  = 36;

            mesh->vertices[0]  = (Vertex){-0.5f, -0.5f,  0.5f,   0, 0, 1,   0, 0};
            mesh->vertices[1]  = (Vertex){ 0.5f, -0.5f,  0.5f,   0, 0, 1,   1, 0};
            mesh->vertices[2]  = (Vertex){ 0.5f,  0.5f,  0.5f,   0, 0, 1,   1, 1};
            mesh->vertices[3]  = (Vertex){-0.5f,  0.5f,  0.5f,   0, 0, 1,   0, 1};

            mesh->vertices[4]  = (Vertex){ 0.5f, -0.5f, -0.5f,   0, 0,-1,   0, 0};
            mesh->vertices[5]  = (Vertex){-0.5f, -0.5f, -0.5f,   0, 0,-1,   1, 0};
            mesh->vertices[6]  = (Vertex){-0.5f,  0.5f, -0.5f,   0, 0,-1,   1, 1};
            mesh->vertices[7]  = (Vertex){ 0.5f,  0.5f, -0.5f,   0, 0,-1,   0, 1};

            mesh->vertices[8]  = (Vertex){-0.5f, -0.5f, -0.5f,  -1, 0, 0,   0, 0};
            mesh->vertices[9]  = (Vertex){-0.5f, -0.5f,  0.5f,  -1, 0, 0,   1, 0};
            mesh->vertices[10] = (Vertex){-0.5f,  0.5f,  0.5f,  -1, 0, 0,   1, 1};
            mesh->vertices[11] = (Vertex){-0.5f,  0.5f, -0.5f,  -1, 0, 0,   0, 1};

            mesh->vertices[12] = (Vertex){ 0.5f, -0.5f,  0.5f,   1, 0, 0,   0, 0};
            mesh->vertices[13] = (Vertex){ 0.5f, -0.5f, -0.5f,   1, 0, 0,   1, 0};
            mesh->vertices[14] = (Vertex){ 0.5f,  0.5f, -0.5f,   1, 0, 0,   1, 1};
            mesh->vertices[15] = (Vertex){ 0.5f,  0.5f,  0.5f,   1, 0, 0,   0, 1};

            mesh->vertices[16] = (Vertex){-0.5f,  0.5f,  0.5f,   0, 1, 0,   0, 0};
            mesh->vertices[17] = (Vertex){ 0.5f,  0.5f,  0.5f,   0, 1, 0,   1, 0};
            mesh->vertices[18] = (Vertex){ 0.5f,  0.5f, -0.5f,   0, 1, 0,   1, 1};
            mesh->vertices[19] = (Vertex){-0.5f,  0.5f, -0.5f,   0, 1, 0,   0, 1};

            mesh->vertices[20] = (Vertex){-0.5f, -0.5f, -0.5f,   0,-1, 0,   0, 0};
            mesh->vertices[21] = (Vertex){ 0.5f, -0.5f, -0.5f,   0,-1, 0,   1, 0};
            mesh->vertices[22] = (Vertex){ 0.5f, -0.5f,  0.5f,   0,-1, 0,   1, 1};
            mesh->vertices[23] = (Vertex){-0.5f, -0.5f,  0.5f,   0,-1, 0,   0, 1};

            uint32_t idx[] = {
                0,  1,  2,   2,  3,  0,
                4,  5,  6,   6,  7,  4,
                8,  9, 10,  10, 11,  8,
                12, 13, 14,  14, 15, 12,
                16, 17, 18,  18, 19, 16,
                20, 21, 22,  22, 23, 20, 
            };
            memcpy(mesh->indices, idx, 36 * sizeof(uint32_t));
        }
    }

    INIT_CUSTOM_ALLOCATION(__this, mesh, delete_mesh, "mesh");

    return (VALUE_OK);
}

static cn_value _update(Object *__this, void **args)
{
    double delta_time = *(double *)args[0];
    Vector3 *rotation = &get_attr(__this, "rotation")->as.vec3;
    rotation->y += 1.0f * delta_time;
    return (null_value);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    DEL_CUSTOM_ALLOCAION(__this, delete_material, "material");
    DEL_CUSTOM_ALLOCAION(__this, delete_mesh, "mesh");

    return (null_value);
}

CN_API Object *new_object3d(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, new_scene_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_update", &_update);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}