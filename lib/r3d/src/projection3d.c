#include "libr3d.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

CN_API void _make_proj(cnnumber out[16],
                       cnnumber fov_deg, cnnumber aspect,
                       cnnumber near, cnnumber far)
{
    memset(out, 0, 16 * sizeof(cnnumber));
    cnnumber f = 1.0f / tanf(fov_deg * (M_PI / 180.0f) * 0.5f);
    out[0]  =  f / aspect;
    out[5]  =  f;
    out[10] = (far + near) / (near - far);
    out[11] = -1.0f;
    out[14] = (2.0f * far * near) / (near - far);
}

CN_API void _make_view(cnnumber out[16],
                       const Vector3 *pos, const Vector3 *rot)
{
    cnnumber yaw   = rot->x;
    cnnumber pitch = rot->y;

    cnnumber fx = -cosf(pitch) * sinf(yaw);
    cnnumber fy =  sinf(pitch);
    cnnumber fz = -cosf(pitch) * cosf(yaw);

    cnnumber rx =  cosf(yaw);
    cnnumber ry =  0.0f;
    cnnumber rz = -sinf(yaw);

    cnnumber ux = ry * fz - rz * fy;
    cnnumber uy = rz * fx - rx * fz;
    cnnumber uz = rx * fy - ry * fx;

    out[0]  =  rx;  out[4]  =  ry;  out[8]   =  rz;
    out[1]  =  ux;  out[5]  =  uy;  out[9]   =  uz;
    out[2]  = -fx;  out[6]  = -fy;  out[10]  = -fz;
    out[3]  =  0;   out[7]  =  0;   out[11]  =  0;

    out[12] = -(rx * pos->x + ry * pos->y + rz * pos->z);
    out[13] = -(ux * pos->x + uy * pos->y + uz * pos->z);
    out[14] =  (fx * pos->x + fy * pos->y + fz * pos->z);
    out[15] =  1.0f;
}

CN_API void _make_model(cnnumber out[16],
                        const Vector3 *pos,
                        const Vector3 *scl,
                        const Vector3 *rot)
{
    cnnumber cx = cosf(rot->x), sx = sinf(rot->x);
    cnnumber cy = cosf(rot->y), sy = sinf(rot->y);
    cnnumber cz = cosf(rot->z), sz = sinf(rot->z);

    out[0]  = (cy * cz + sy * sx * sz) * scl->x;
    out[1]  = (cx * sz)                * scl->x;
    out[2]  = (cy * sx * sz - sy * cz) * scl->x;
    out[3]  = 0.0f;

    out[4]  = (sy * sx * cz - cy * sz) * scl->y;
    out[5]  = (cx * cz)                * scl->y;
    out[6]  = (cy * cz * sx + sy * sz) * scl->y;
    out[7]  = 0.0f;

    out[8]  =  (sy * cx)               * scl->z;
    out[9]  = -sx                      * scl->z;
    out[10] =  (cy * cx)               * scl->z;
    out[11] = 0.0f;

    out[12] = pos->x;
    out[13] = pos->y;
    out[14] = pos->z;
    out[15] = 1.0f;
}
