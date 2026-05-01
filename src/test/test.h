#ifndef _TEST_H_
    #define _TEST_H_

    #include "../engine.h"

    cn_value _move_left(Object *__this, void **args);
    cn_value _move_right(Object *__this, void **args);
    cn_value _move_forward(Object *__this, void **args);
    cn_value _move_backward(Object *__this, void **args);

    cn_value show_fps(Object *__this, void **args);

    Object *set_up_camera(Object *ctx, Object *board);
    Object *create_threed_view(Object *ctx, Vector2 position, Vector2 size);
    Object *build_hud_test(Vector2 position, Vector2 size);
    Object *add_test_window(Object *ctx);
    Object *build_engine(void);

#endif
