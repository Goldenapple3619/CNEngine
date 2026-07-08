#include "libcncore.h"

CN_API struct generic_kv_pair_s *new_kv_pair(const char *k, void *v, void (*v_deletor)(void *))
{
    struct generic_kv_pair_s *kvp = malloc(sizeof(struct generic_kv_pair_s));

    if (!kvp) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new kvp (%s, %p).", k, v)
        return (NULL);
    }

    kvp->v = NULL;
    kvp->k._k = NULL;
    kvp->_v_deletor = NULL;
    kvp->_k_alloc = true;

    if (init_kv_pair(kvp, k, v, v_deletor)) {
        PROPAGATE_ERR();
        (void)delete_kv_pair(kvp);
        return (NULL);
    }

    return (kvp);
}

CN_API struct generic_kv_pair_s *new_ckv_pair(const char *k, void *v, void (*v_deletor)(void *))
{
    struct generic_kv_pair_s *kvp = malloc(sizeof(struct generic_kv_pair_s));

    if (!kvp) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new ckvp (%s, %p).", k, v)
        return (NULL);
    }

    kvp->v = NULL;
    kvp->k._k = NULL;
    kvp->_v_deletor = NULL;
    kvp->_k_alloc = false;

    if (init_ckv_pair(kvp, k, v, v_deletor)) {
        PROPAGATE_ERR();
        (void)delete_kv_pair(kvp);
        return (NULL);
    }

    return (kvp);
}

CN_API uint8_t init_kv_pair(struct generic_kv_pair_s *kvp, const char *k, void *v, void (*v_deletor)(void *))
{
    if (!kvp) {
        RAISE(ERR_INVALID_POINTER, "can't init empty kv pair.");
        return (1);
    }

    if (k) {
        kvp->k._k = strdup(k);

        if (!kvp->k._k) {
            RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate kvp %p string '%s'.", kvp, k);
            return (1);
        }
    } else
        kvp->k._k = NULL;

    kvp->v = v;
    kvp->_v_deletor = v_deletor;
    kvp->_k_alloc = true;
    return (0);
}

CN_API uint8_t init_ckv_pair(struct generic_kv_pair_s *kvp, const char *k, void *v, void (*v_deletor)(void *))
{
    if (!kvp) {
        RAISE(ERR_INVALID_POINTER, "can't init empty ckv pair.");
        return (1);
    }

    kvp->k._sk = k;
    kvp->v = v;
    kvp->_v_deletor = v_deletor;
    kvp->_k_alloc = false;
    return (0);
}

CN_API void empty_kv_pair(struct generic_kv_pair_s *kvp)
{
    if (!kvp) {
        RAISE(ERR_INVALID_POINTER, "can't empty empty kv pair.");
        return;
    }

    if (kvp->_k_alloc && kvp->k._k)
        (void)free(kvp->k._k);
    (void)memset(&kvp->k, 0, sizeof(kvp->k));
    if (kvp->_v_deletor)
        (void)kvp->_v_deletor(kvp->v);
    kvp->v = NULL;
    kvp->_v_deletor = NULL;
    kvp->_k_alloc = false;
}

CN_API void delete_kv_pair(struct generic_kv_pair_s *kvp)
{
    if (!kvp) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty kv pair.");
        return;
    }

    (void)empty_kv_pair(kvp);
    (void)free(kvp);
}
