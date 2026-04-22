#include "libcngraphic.h"

CN_API Material *new_material(void)
{
    Material *material = malloc(sizeof(Material));

    if (!material)
        return (NULL);
    material->color = 0;
    material->texture = NULL;
    material->shader = NULL;
    material->ambient   = 0.1f;
    material->diffuse   = 0.8f;
    material->specular  = 0.5f;
    material->shininess = 32.0f;
    return (material);
}

void material_use_gl(const Material *mat,
                  const cnnumber model[16],
                  const cnnumber view[16],
                  const cnnumber proj[16])
{
    if (!mat || !mat->shader || mat->shader->api != R_API_GL || !mat->shader->gpu_handler.gl_shader)
        return;

    glUseProgram(mat->shader->gpu_handler.gl_shader);

    glUniformMatrix4fv(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_model"),
                       1, GL_FALSE, model);
    glUniformMatrix4fv(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_view"),
                       1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_proj"),
                       1, GL_FALSE, proj);

    glUniform4f(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_color"),
                ((mat->color & 0xff000000) >> 24) / 255.0f,
            ((mat->color & 0x00ff0000) >> 16) / 255.0f,
            ((mat->color & 0x0000ff00) >> 8) / 255.0f,
            (mat->color & 0x000000ff) / 255.0f);
    glUniform1f(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_ambient"),   mat->shader->gpu_handler.gl_shader);
    glUniform1f(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_diffuse"),   mat->diffuse);
    glUniform1f(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_specular"),  mat->specular);
    glUniform1f(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_shininess"), mat->shininess);

    /* Texture slot 0 */
    if (mat->texture && mat->texture->api == R_API_GL) {
        if (!mat->texture->gpu_handler.gl_texture.gl_id || mat->texture->gpu_handler.gl_texture.gl_ctx != SDL_GL_GetCurrentContext()) {
            if (!texture_upload_gl(mat->texture)) {
                glUniform1i(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_has_texture"), 0);
                return;
            }
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat->texture->gpu_handler.gl_texture.gl_id);
        glUniform1i(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_texture"), 0);
        glUniform1i(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_has_texture"), 1);
    } else {
        glUniform1i(glGetUniformLocation(mat->shader->gpu_handler.gl_shader, "u_has_texture"), 0);
    }
}

CN_API void delete_material(Material *material)
{
    if (material)
        return;
    (void)free(material);
}
