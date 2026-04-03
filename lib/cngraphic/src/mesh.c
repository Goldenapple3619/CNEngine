#include "libcngraphic.h"

CN_API Mesh *new_mesh(void)
{
    Mesh *mesh = malloc(sizeof(Mesh));

    if (!mesh)
        return (NULL);
    mesh->vertex_count = 0;
    mesh->index_count = 0;
    (void)memset(&mesh->gpu_handler, 0, sizeof(mesh->gpu_handler));
    mesh->indices = NULL;
    mesh->vertices = NULL;
    mesh->uploaded = false;
    mesh->api = R_API_NONE;
    return (mesh);
}

cnbool mesh_upload_gl(Mesh *m)
{
    if (!m || !m->vertices || !m->vertex_count)
        return (false);

    if (m->uploaded) {
        glDeleteVertexArrays(1, &m->gpu_handler.gl.vao);
        glDeleteBuffers(1, &m->gpu_handler.gl.vbo);
        if (m->index_count)
            glDeleteBuffers(1, &m->gpu_handler.gl.ebo);
    }

    glGenVertexArrays(1, &m->gpu_handler.gl.vao);
    glGenBuffers(1, &m->gpu_handler.gl.vbo);
    glBindVertexArray(m->gpu_handler.gl.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m->gpu_handler.gl.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(m->vertex_count * sizeof(Vertex)),
                 m->vertices, GL_STATIC_DRAW);

    /* position location 0 */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);

    /* normal location 1 */
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, nx));
    glEnableVertexAttribArray(1);

    /* texcoords location 2 */
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);

    if (m->index_count && m->indices) {
        glGenBuffers(1, &m->gpu_handler.gl.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->gpu_handler.gl.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     (GLsizeiptr)(m->index_count * sizeof(uint32_t)),
                     m->indices, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
    m->uploaded = true;
    return true;
}

void mesh_draw_gl(const Mesh *m)
{
    if (!m || !m->uploaded)
        return;

    glBindVertexArray(m->gpu_handler.gl.vao);

    if (m->index_count)
        glDrawElements(GL_TRIANGLES, (GLsizei)m->index_count,
                       GL_UNSIGNED_INT, NULL);
    else
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m->vertex_count);

    glBindVertexArray(0);
}

CN_API void delete_mesh(Mesh *mesh)
{
    if (!mesh)
        return;
    
    if (mesh->uploaded) {
        if (mesh->api == R_API_GL) {
            (void)glDeleteVertexArrays(1, &mesh->gpu_handler.gl.vao);
            (void)glDeleteBuffers(1, &mesh->gpu_handler.gl.vbo);
            if (mesh->index_count)
                (void)glDeleteBuffers(1, &mesh->gpu_handler.gl.ebo);
            mesh->uploaded = false;
        }
    }

    if (mesh->vertices)
        (void)free(mesh->vertices);
    if (mesh->indices)
        (void)free(mesh->indices);
    (void)free(mesh);
}
