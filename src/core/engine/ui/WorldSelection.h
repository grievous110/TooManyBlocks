#ifndef TOOMANYBLOCKS_WORLDSELECTION_H
#define TOOMANYBLOCKS_WORLDSELECTION_H

#include <json/JsonParser.h>

#include <mutex>
#include <string>

#include "engine/ui/Ui.h"

namespace UI {
    class WorldSelection : public Window {
    private:
        std::mutex m_mtx;
        Json::JsonArray m_worldInfos;
        Json::JsonValue* m_selectedWorld;
        bool m_shouldLoadInfo;

        char m_newWorldName[64];
        uint32_t m_newWorldSeed;

        void randomSeed();

        void loadWorldInfoOnce();

        void createNewWorld(const std::string& name);

        void renameWorld(const std::string& name);

        void deleteWorld();

    public:
        WorldSelection() : m_selectedWorld(nullptr), m_shouldLoadInfo(true) {}

        virtual ~WorldSelection() = default;

        void render(ApplicationContext& context) override;
    };
}  // namespace UI

#endif