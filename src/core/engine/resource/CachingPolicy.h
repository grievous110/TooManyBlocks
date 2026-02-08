#ifndef TOOMANYBLOCKS_CACHINGPOLICY_H
#define TOOMANYBLOCKS_CACHINGPOLICY_H

enum class CachePolicy {
    Persist,
    GracePeriod,
    RefCounted,
    None
};

#endif
