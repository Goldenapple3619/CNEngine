#ifndef _LIBCNINPUT_H_
    #define _LIBCNINPUT_H_

    #include "libcncore.h"
    #include "libcngraphic.h"

    typedef enum {
        INPUT_UKN = 0x00,
        INPUT_KEY_PRESS,
    } input_type;

    typedef struct {
        input_type target_type;
        int64_t target_value;
    } InputController;

    typedef struct {
        size_t cbs_size;
        size_t cbs_capacity;
        ObjMethodPair **cbs;

        size_t controllers_size;
        size_t controllers_capacity;
        InputController **controllers;

        char *name;
    } InputEntry;

    CN_API cnbool start_input(void);
    CN_API void end_input(void);

    InputEntry *new_input_entry(const char *name);
    void delete_input_entry(InputEntry *ie);

    InputController *new_input_controller(input_type target_type, int64_t target_value);
    void delete_input_controller(InputController *ic);

    CN_API Object *new_input_submodule(void);

#endif
