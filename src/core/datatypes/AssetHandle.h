#ifndef TOOMANYBLOCKS_ASSETHANDLE_H
#define TOOMANYBLOCKS_ASSETHANDLE_H

#include <atomic>
#include <memory>

template <typename T>
struct AssetHandle {
    std::atomic<bool> ready = false;
    std::shared_ptr<T> asset = nullptr;
};

#endif