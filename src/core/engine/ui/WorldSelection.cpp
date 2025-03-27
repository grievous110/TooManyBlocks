#include "Application.h"
#include "engine/env/World.h"
#include "engine/GameInstance.h"
#include "engine/ui/fonts/FontUtil.h"
#include "Logger.h"
#include "util/Utility.h"
#include "WorldSelection.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <random>
#include <string>

namespace fs = std::filesystem;

namespace UI {
    void WorldSelection::randomSeed() {
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<uint32_t> distribution(0, UINT32_MAX);
        m_newWorldSeed = distribution(generator);
    }

    void WorldSelection::loadWorldInfoOnce() {
        if (!m_checkedDir) {
            m_worldInfos.clear();
            fs::path savedDir = getAppDataPath() / "saved";
            if (!fs::exists(savedDir)) {
                if (!fs::create_directories(savedDir)) {
                    throw std::runtime_error("Could not create directory: " + savedDir.string());                    
                }
            } else {
                for (const auto& entry : fs::directory_iterator(savedDir)) {
                    fs::path infoPath = entry.path() / "info.json";
                    if (fs::exists(infoPath)) {
                        std::ifstream file(infoPath.string(), std::ios::binary);
                        Json::JsonValue info = Json::parseJson(std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()));
                        m_worldInfos.push_back(info);
                        file.close();
                    }
                }
            }
            lgr::lout.debug("Saves directory: " + savedDir.string());

            m_selectedWorld = nullptr;
            m_checkedDir = true;
        }
    }
    
    void WorldSelection::createNewWorld(const std::string& name) {
        fs::path savedDir = getAppDataPath() / "saved" / name / "info.json";
        fs::create_directories(savedDir.parent_path());

        Json::JsonObject value;
        value["worldName"] = name;
        value["seed"] = std::to_string(m_newWorldSeed);
        std::ofstream file(savedDir.string());
        file << Json::toJsonString(value);
        file.close();
    }

    void WorldSelection::renameWorld(const std::string& name) {
        try {
            std::string oldName = (*m_selectedWorld)["worldName"].toString();
            (*m_selectedWorld)["worldName"] = name;
            fs::path savedDir = getAppDataPath() / "saved";
            fs::path oldWorldDir = savedDir / oldName;
            fs::path newWorldDir = savedDir / name;
            fs::rename(oldWorldDir, newWorldDir);
            std::ofstream file((newWorldDir / "info.json").string(), std::ios::binary);
            file << Json::toJsonString((*m_selectedWorld));
            file.close();
        } catch(const std::exception& e) {
            setError(e.what());
            lgr::lout.error(e.what());
        }
    }

    void WorldSelection::deleteWorld() {
        try {
            std::string name = (*m_selectedWorld)["worldName"].toString();
            fs::remove_all(getAppDataPath() / "saved" / name);
            m_selectedWorld = nullptr;
        } catch(const std::exception& e) {
            setError(e.what());
            lgr::lout.error(e.what());
        }
    }
    
    void WorldSelection::render(ApplicationContext& context) {
        loadWorldInfoOnce();
        ImGuiIO& io = ImGui::GetIO();
        
		ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;
        
		
        UI::Util::MakeNextWindowFullscreen();
        ImGui::Begin("World Selection", NULL, window_flags);
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 oldSpacing = style.ItemSpacing;
        style.ItemSpacing = ImVec2(25.0f, 20.0f);
        {
            ScopedFont font(context.fontPool->getFont(35));
            ImVec2 titleSize = ImGui::CalcTextSize("World Selection");
            
            ImGui::SetCursorPosX((io.DisplaySize.x - titleSize.x) * 0.5f); // Center horizontally
            ImGui::Text("World Selection");
        }
        {
            ScopedFont font(context.fontPool->getFont(25));
            ImVec2 available = ImGui::GetContentRegionAvail();
            ImVec2 childSize = ImVec2(available.x * 0.8f, available.y * 0.6f); // Adjust child siz
            ImGui::SetCursorPosX((available.x - childSize.x) * 0.5f);
            ImGui::BeginChild("ButtonList", childSize, true, ImGuiWindowFlags_HorizontalScrollbar);
            if (m_worldInfos.empty()) {
                ImGui::Text("No Worlds");
            } else {
                for (Json::JsonValue& worldInfo : m_worldInfos) {
                    std::string label = worldInfo["worldName"].toString() + "\n";
                    label += "Seed: " + worldInfo["seed"].toString() + "\n";
                    
                    bool isSelected = m_selectedWorld == &worldInfo;
                    float buttonHeight = 70.0f;
                    if (ImGui::Button(label.c_str(), ImVec2(400.0f, buttonHeight))) {
                        if (isSelected) {
                            m_selectedWorld = nullptr;
                        } else {
                            m_selectedWorld = &worldInfo;
                        }
                    }
                    if (isSelected) {
                        ImGui::SameLine();
                        ImDrawList* drawList = ImGui::GetWindowDrawList();
                        ImVec2 blockPos = ImGui::GetCursorScreenPos();
                        ImVec2 blockSize = ImVec2(10, buttonHeight);
                        drawList->AddRectFilled(blockPos, ImVec2(blockPos.x + blockSize.x, blockPos.y + blockSize.y), IM_COL32(0, 255, 0, 255));
                        ImGui::NewLine();
                    }
                }
            }
            ImGui::EndChild();
            
            if (hasError() && !ImGui::IsPopupOpen((const char*)0, ImGuiPopupFlags_AnyPopup)) {
                float errorWidth = ImGui::CalcTextSize(getError()).x;
                ImGui::SetCursorPosX((io.DisplaySize.x - errorWidth) * 0.5f);
                ImGui::TextColored(ImVec4(1, 0, 0, 1), getError());
            }
            
            ImGui::SetCursorPosX((io.DisplaySize.x - 725.0f) * 0.5f);
            
            if (m_selectedWorld == nullptr) ImGui::BeginDisabled();
            if (ImGui::Button("Play selected world", ImVec2(350.0f, 45.0f))) {
                try {
                    fs::path worldPath = getAppDataPath() / "saved" / (*m_selectedWorld)["worldName"].toString();
                    World* world = new World(worldPath);
                    context.instance->initializeWorld(world);
                    // Capture and hide the mouse cursor
                    glfwSetInputMode(context.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    navigateToWindow(context, "GameOverlay");
                } catch (const std::exception& e) {
                    context.instance->deinitWorld();
                    setError(e.what());
                    lgr::lout.error(e.what());
                }
            }
            if (m_selectedWorld == nullptr) ImGui::EndDisabled();
            
            ImGui::SameLine();
            if (ImGui::Button("Create new world", ImVec2(350.0f, 45.0f))) {
                std::memset(m_newWorldName, 0, sizeof(m_newWorldName));
                clearError();
                randomSeed();

                ImGui::OpenPopup("Create New World");
            }

            ImGui::SetCursorPosX((io.DisplaySize.x - 725.0f) * 0.5f);
            
            if (m_selectedWorld == nullptr) ImGui::BeginDisabled();
            if (ImGui::Button("Rename", ImVec2(162.5f, 45.0f))) {
                std::memset(m_newWorldName, 0, sizeof(m_newWorldName));
                clearError();

                ImGui::OpenPopup("Rename World");
            }
            if (m_selectedWorld == nullptr) ImGui::EndDisabled();

            ImGui::SameLine();
            
            if (m_selectedWorld == nullptr) ImGui::BeginDisabled();
            if (ImGui::Button("Delete", ImVec2(162.5f, 45.0f))) {
                clearError();

                ImGui::OpenPopup("Delete World");
            }
            if (m_selectedWorld == nullptr) ImGui::EndDisabled();

            ImGui::SameLine();
            
            if (ImGui::Button("Back", ImVec2(350.0f, 45.0f))) {
                navigateToWindow(context, "MainMenu");
            }

            if (ImGui::BeginPopupModal("Create New World", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
                UI::Util::CenterPopup();

                ImGui::Text("Enter World Name:");
                ImGui::InputText("##NameInput", m_newWorldName, sizeof(m_newWorldName));
                ImGui::Text("Enter World Seed:");
                ImGui::InputScalar("##SeedInput", ImGuiDataType_U32, &m_newWorldSeed);
                ImGui::SameLine();
                if (ImGui::Button("Random")) {
                    randomSeed();
                }
                if (hasError()) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), getError());
                }
                
                ImGui::Separator();
        
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
                if (ImGui::Button("OK", ImVec2(100.0f, 35.0f))) {
                    std::string worldName(m_newWorldName);
                    if (worldName.empty()) {
                        setError("Please enter a valid name!");
                    } else {
                        auto it = std::find_if(m_worldInfos.begin(), m_worldInfos.end(), [worldName](Json::JsonValue& value) {
                            return value["worldName"] == worldName;
                        });
                        if (it != m_worldInfos.end()) {
                            setError("World with that name already exists!");
                        } else {
                            createNewWorld(worldName);
                            m_checkedDir = false;                            

                            ImGui::CloseCurrentPopup();
                            clearError();
                        }    
                    }
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(100.0f, 35.0f))) {
                    ImGui::CloseCurrentPopup();
                    clearError();
                }
        
                ImGui::EndPopup();
            }

            if (ImGui::BeginPopupModal("Rename World", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
                UI::Util::CenterPopup();
                
                ImGui::Text("Enter New World Name:");
                ImGui::InputText("##NameInput", m_newWorldName, sizeof(m_newWorldName));
                if (hasError()) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), getError());
                }
                
                ImGui::Separator();
        
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
                if (ImGui::Button("OK", ImVec2(100.0f, 35.0f))) {
                    std::string worldName(m_newWorldName);
                    if (worldName.empty()) {
                        setError("Please enter a valid name!");
                    } else {
                        auto it = std::find_if(m_worldInfos.begin(), m_worldInfos.end(), [worldName](Json::JsonValue& value) {
                            return value["worldName"] == worldName;
                        });
                        if (it != m_worldInfos.end()) {
                            setError("World with that name already exists!");
                        } else {
                            renameWorld(worldName);
                            m_checkedDir = false;                            

                            ImGui::CloseCurrentPopup();
                            clearError();
                        }    
                    }
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(100.0f, 35.0f))) {
                    ImGui::CloseCurrentPopup();
                    clearError();
                }
        
                ImGui::EndPopup();
            }

            if (ImGui::BeginPopupModal("Delete World", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
                UI::Util::CenterPopup();

                ImGui::TextWrapped("Do you really want to delete world \"%s\"?", (*m_selectedWorld)["worldName"].toString().c_str());
                ImGui::Separator();
        
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                if (ImGui::Button("DELETE", ImVec2(100.0f, 35.0f))) {
                    deleteWorld();
                    m_checkedDir = false;

                    ImGui::CloseCurrentPopup();
                    clearError(); 
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(100.0f, 35.0f))) {
                    ImGui::CloseCurrentPopup();
                    clearError();
                }
        
                ImGui::EndPopup();
            }
        }
        style.ItemSpacing = oldSpacing;
        ImGui::End();
    }
}