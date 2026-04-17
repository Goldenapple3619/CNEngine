#ifndef _LIBCNINPUT_H_
    #define _LIBCNINPUT_H_

    #include "libcncore.h"
    #include "libcngraphic.h"

    typedef enum {
        INPUT_UKN = 0x00,
        INPUT_KEY_PRESS,
    } input_type;

    typedef enum {
        INPUT_STATE_ACTIVATE = 0,
        INPUT_STATE_STOP
    } INPUT_STATE;

    typedef struct {
        input_type target_type;
        int64_t target_value;

        cnbool ignore_value;
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
    INPUT_STATE input_state_from_event(cn_event ev);
    void input_entry_activate(const InputEntry *ie, const Event *event);
    cnbool input_entry_cmp(const InputEntry *ie, const Event *ev);
    uint8_t input_entry_resize_callback(InputEntry *ie, size_t new_capacity);
    uint8_t input_entry_resize_controller(InputEntry *ie, size_t new_capacity);
    uint8_t input_entry_add_callback(InputEntry *ie, cn_method method, Object *obj);
    uint8_t input_entry_add_controller(InputEntry *ie, input_type target_type, int64_t target_value, cnbool value_ignored);
    void input_entry_remove_callback(InputEntry *ie, size_t i);
    void input_entry_remove_controller(InputEntry *ie, size_t i);
    void delete_input_entry(InputEntry *ie);

    InputController *new_input_controller(input_type target_type, int64_t target_value, cnbool value_ignored);
    cnbool input_controller_cmp(const InputController *ic, const Event *ev);
    input_type input_type_from_event(cn_event ev);
    void delete_input_controller(InputController *ic);

    CN_API Object *new_input_submodule(void);

#endif
