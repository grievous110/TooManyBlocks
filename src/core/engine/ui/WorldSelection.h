#ifndef WORLDSELECTION_H
#define WORLDSELECTION_H

#include "engine/ui/Ui.h"
#include <json/JsonParser.h>
#include <string>
#include <vector>

namespace UI {
	class WorldSelection : public Window {
    private:
        std::vector<Json::JsonValue> m_worldInfos;
		Json::JsonValue* m_selectedWorld;
		bool m_checkedDir;

		bool m_showCreateWorldDialog;
		bool m_showRenameWorldDialog;
		bool m_showDeleteWorldDialog;
		char m_newWorldName[64];
		uint32_t m_newWorldSeed;

		void randomSeed();

		void loadWorldInfoOnce();
	
		void createNewWorld(const std::string& name);

		void renameWorld(const std::string& name);

		void deleteWorld();
		
	public:
		WorldSelection() : m_selectedWorld(nullptr), m_showCreateWorldDialog(false), m_showRenameWorldDialog(false), m_showDeleteWorldDialog(false), m_checkedDir(false) {}

		virtual ~WorldSelection() = default;

		void render(ApplicationContext& context) override;
	};
}

#endif