// FullPartyRPG - Win32 application version of the terminal RPG.
#include "framework.h"
#include "FullPartyRPG.h"
#include "knight.h"
#include "ranger.h"
#include "wizard.h"
#include "bard.h"
#include "skeleton.h"
#include "game_config.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#define MAX_LOADSTRING 100
#define IDC_ACTOR 1001
#define IDC_ACTION 1002
#define IDC_TARGET 1003
#define IDC_TURN 1004
#define IDC_RESET 1005
#define IDC_LOG 1006
#define IDC_STATUS 1007
#define IDC_START 1008
#define IDC_CLASS1 1009
#define IDC_CLASS2 1010
#define IDC_CLASS3 1011
#define IDC_NAME1 1012
#define IDC_NAME2 1013
#define IDC_NAME3 1014
#define IDC_ACTION1 1020
#define IDC_ACTION2 1021
#define IDC_ACTION3 1022
#define IDC_TARGET1 1030
#define IDC_TARGET2 1031
#define IDC_TARGET3 1032
#define IDC_SONG1 1040
#define IDC_SONG2 1041
#define IDC_SONG3 1042
#define IDC_SPELL1 1050
#define IDC_SPELL2 1051
#define IDC_SPELL3 1052
#define IDC_BACK 1060
#define IDC_TREASURE1 1070
#define IDC_TREASURE2 1071
#define IDC_TREASURE3 1072
#define IDC_TREASURE4 1073
#define IDC_TREASURE5 1074
#define IDC_TREASURE_CHAR1 1080
#define IDC_TREASURE_CHAR2 1081
#define IDC_TREASURE_CHAR3 1082
#define IDC_SAVE 1090
#define IDC_LOAD 1091
#define IDM_SAVE_MENU 110
#define IDM_LOAD_MENU 111
#define IDM_NEWGAME_MENU 112
#define IDM_SAVE_SLOT1 120
#define IDM_SAVE_SLOT2 121
#define IDM_SAVE_SLOT3 122
#define IDM_LOAD_SLOT1 130
#define IDM_LOAD_SLOT2 131
#define IDM_LOAD_SLOT3 132
#define IDM_ERASE_SLOT1 140
#define IDM_ERASE_SLOT2 141
#define IDM_ERASE_SLOT3 142
#define IDC_DUNGEON_FIRST 2000

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING], szWindowClass[MAX_LOADSTRING];
HWND hActor, hAction, hTarget, hLog, hStatus, hReset;
HWND hClass[3], hName[3], hStart;
HWND hTurn, hCombatLabels[3];
HWND hActionButtons[3], hTargetButtons[3], hPrompt;
HWND hSongButtons[3];
HWND hSpellButtons[3];
HWND hBack;
HWND hTreasureButtons[5], hTreasureCharacterButtons[3];
HWND hSave, hLoad;
HMENU saveMenu, loadMenu, eraseMenu;
HWND hSetupLabels[7];
HWND hDungeonInfo;
HWND hDungeonTiles[49];
std::vector<std::unique_ptr<character>> party;
std::vector<std::unique_ptr<skeleton>> enemies;
std::mt19937 rng(std::random_device{}());
int currentActor = 0;
int pendingAction = 0;
int pendingSong = 1;
int pendingSpell = 1;
bool selectingPartyTarget = false;
bool turnManaGranted = false;
bool startTurnBonusesApplied = false;
bool selectingTreasureCharacter = false;
int selectedTreasure = 0;
enum class TrapType { Arrow, Animal, Magic, Riddle };
TrapType currentTrap = TrapType::Arrow;
bool selectingTrapCharacter = false;
bool inCombat = false;
std::array<int, 3> damageBonuses{};
std::array<int, 3> defenseBonuses{};
std::array<int, 3> healthBonuses{};
std::array<int, 3> abilityBonuses{};
std::array<int, 3> itemCounts{};
std::array<std::vector<std::string>, 3> inventory;
enum class RoomType { Empty, Treasure, Enemy, Trap, Start, End };
std::array<RoomType, 49> dungeon;
std::array<bool, 49> dungeonVisited{};
int dungeonRow = 3;
int dungeonColumn = 3;
int dungeonEndIndex = 0;
int forcedTarget = -1;
int teamDamageBonus = 0;
int songDefense = 0;

std::wstring widen(const std::string& value) { return { value.begin(), value.end() }; }

void appendLog(const std::string& text)
{
    int length = GetWindowTextLengthW(hLog);
    SendMessageW(hLog, EM_SETSEL, length, length);
    std::wstring line = widen(text) + L"\r\n";
    SendMessageW(hLog, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(line.c_str()));
    SendMessageW(hLog, EM_SCROLLCARET, 0, 0);
}

void refreshStatus()
{
    std::ostringstream out;
    out << "PARTY\r\n";
    for (size_t i = 0; i < party.size(); ++i) {
        out << i + 1 << ". " << party[i]->GetDisplayName() << " - HP: "
            << party[i]->GetHP() << "/" << party[i]->GetMaxHP();
        if (auto* knightMember = dynamic_cast<knight*>(party[i].get()))
            out << " | Spirit: " << knightMember->GetSpirit();
        else if (auto* wizardMember = dynamic_cast<wizard*>(party[i].get()))
            out << " | Mana: " << wizardMember->GetMana();
        else if (auto* bardMember = dynamic_cast<bard*>(party[i].get())) {
            std::string song = bardMember->GetCurrentSong();
            const std::string prefix = "Song of ";
            if (song.rfind(prefix, 0) == 0) song = song.substr(prefix.length());
            out << " | Song: " << song;
        }
        out << "\r\n   Items: " << itemCounts[i] << "/2";
        for (const auto& item : inventory[i]) {
            const std::string prefix = "Ring of ";
            std::string shortName = item.rfind(prefix, 0) == 0 ? item.substr(prefix.length()) : item;
            out << " (" << shortName << ")";
        }
        out << "\r\n";
    }
    if (inCombat) {
        out << "\r\nENEMIES\r\n";
        for (size_t i = 0; i < enemies.size(); ++i) {
            out << "Skeleton " << i + 1 << " - " << enemies[i]->GetHP() << " HP";
            if (enemies[i]->GetAttackReduction() > 0)
                out << " | vATKv: -" << enemies[i]->GetAttackReduction();
            if (enemies[i]->GetNextAttackBonus() > 0)
                out << " | vDEFv: -" << enemies[i]->GetNextAttackBonus();
            out << "\r\n";
        }
    }
    SetWindowTextW(hStatus, widen(out.str()).c_str());
}

template <typename Function>
std::string runWithInput(const std::string& input, Function action)
{
    std::istringstream inputStream(input);
    std::ostringstream outputStream;
    auto* oldIn = std::cin.rdbuf(inputStream.rdbuf());
    auto* oldOut = std::cout.rdbuf(outputStream.rdbuf());
    action();
    std::cin.rdbuf(oldIn);
    std::cout.rdbuf(oldOut);
    return outputStream.str();
}

std::string controlText(HWND control)
{
    int length = GetWindowTextLengthW(control);
    std::wstring value(length + 1, L'\0');
    GetWindowTextW(control, value.data(), length + 1);
    value.resize(length);
    std::string result;
    for (wchar_t character : value) result.push_back(static_cast<char>(character));
    return result;
}

void showCombat(bool visible)
{
    inCombat = visible;
    int command = visible ? SW_SHOW : SW_HIDE;
    if (visible) MoveWindow(hLog, 300, 140, 475, 395, TRUE);
    for (HWND control : hDungeonTiles) ShowWindow(control, SW_HIDE);
    ShowWindow(hDungeonInfo, SW_HIDE);
    for (HWND control : { hLog, hStatus }) ShowWindow(control, command);
    ShowWindow(hPrompt, command);
    for (HWND control : hActionButtons) ShowWindow(control, command);
    for (HWND control : hTargetButtons) ShowWindow(control, SW_HIDE);
    for (HWND control : hSongButtons) ShowWindow(control, SW_HIDE);
    for (HWND control : hSpellButtons) ShowWindow(control, SW_HIDE);
    for (HWND control : hTreasureButtons) ShowWindow(control, SW_HIDE);
    for (HWND control : hTreasureCharacterButtons) ShowWindow(control, SW_HIDE);
    ShowWindow(hBack, SW_HIDE);
    ShowWindow(hStart, visible ? SW_HIDE : SW_SHOW);
    for (HWND control : hClass) ShowWindow(control, visible ? SW_HIDE : SW_SHOW);
    for (HWND control : hName) ShowWindow(control, visible ? SW_HIDE : SW_SHOW);
    for (HWND control : hSetupLabels) ShowWindow(control, visible ? SW_HIDE : SW_SHOW);
}

std::wstring dungeonLabel(RoomType type)
{
    switch (type) {
    case RoomType::Empty: return L"";
    case RoomType::Treasure: return L"Treasure";
    case RoomType::Enemy: return L"Enemy";
    case RoomType::Trap: return L"Trap";
    case RoomType::Start: return L"Start";
    case RoomType::End: return L"End";
    }
    return L"?";
}

void updateDungeon()
{
    bool dungeonComplete = dungeonRow * 7 + dungeonColumn == dungeonEndIndex;
    for (int row = 0; row < 7; ++row) {
        for (int column = 0; column < 7; ++column) {
            int index = row * 7 + column;
            SetWindowTextW(hDungeonTiles[index], L"");
            EnableWindow(hDungeonTiles[index], !dungeonComplete);
            InvalidateRect(hDungeonTiles[index], nullptr, TRUE);
        }
    }
    std::wstring info = dungeonComplete
        ? L"Dungeon complete! You found the end."
        : L"Dungeon - move one room at a time (current room: "
            + dungeonLabel(dungeon[dungeonRow * 7 + dungeonColumn]) + L")";
    SetWindowTextW(hDungeonInfo, info.c_str());
}

void generateDungeon()
{
    dungeon.fill(RoomType::Empty);
    dungeonVisited.fill(false);
    dungeon[24] = RoomType::Start;
    std::vector<int> rooms = { 24 };
    std::vector<int> frontier;
    auto addFrontier = [&](int index) {
        if (index < 0 || index >= 49 || dungeon[index] != RoomType::Empty) return;
        if (std::find(frontier.begin(), frontier.end(), index) == frontier.end()) frontier.push_back(index);
    };
    auto addNeighbors = [&](int index) {
        int row = index / 7, column = index % 7;
        if (row > 0) addFrontier(index - 7);
        if (row < 6) addFrontier(index + 7);
        if (column > 0) addFrontier(index - 1);
        if (column < 6) addFrontier(index + 1);
    };
    addNeighbors(24);
    std::uniform_int_distribution<int> frontierRoll;
    while (rooms.size() < 24 && !frontier.empty()) {
        frontierRoll = std::uniform_int_distribution<int>(0, static_cast<int>(frontier.size()) - 1);
        int choice = frontierRoll(rng);
        int room = frontier[choice];
        frontier.erase(frontier.begin() + choice);
        if (dungeon[room] != RoomType::Empty) continue;
        dungeon[room] = RoomType::Treasure;
        rooms.push_back(room);
        addNeighbors(room);
    }
    // The final room grown into the connected dungeon becomes the END tile.
    dungeonEndIndex = rooms.back();
    dungeon[dungeonEndIndex] = RoomType::End;
    std::vector<int> roomIndexes;
    for (int room : rooms) if (room != 24 && room != dungeonEndIndex) roomIndexes.push_back(room);
    // Exactly six rooms are Treasure rooms; every other room is Enemy or Trap.
    for (int i = 0; i < 6; ++i) dungeon[roomIndexes[i]] = RoomType::Treasure;
    std::uniform_int_distribution<int> roomRoll(0, 1);
    for (size_t i = 6; i < roomIndexes.size(); ++i)
        dungeon[roomIndexes[i]] = static_cast<RoomType>(2 + roomRoll(rng));
    dungeonRow = 3;
    dungeonColumn = 3;
    dungeonVisited[24] = true;
}

void showDungeon(bool visible)
{
    if (visible) inCombat = false;
    int command = visible ? SW_SHOW : SW_HIDE;
    if (visible) MoveWindow(hLog, 300, 420, 475, 115, TRUE);
    for (HWND control : { hDungeonInfo, hStatus, hLog }) ShowWindow(control, command);
    for (HWND control : hDungeonTiles) ShowWindow(control, command);
    for (HWND control : hClass) ShowWindow(control, SW_HIDE);
    for (HWND control : hName) ShowWindow(control, SW_HIDE);
    for (HWND control : hSetupLabels) ShowWindow(control, SW_HIDE);
    ShowWindow(hStart, SW_HIDE);
    ShowWindow(hPrompt, SW_HIDE);
    for (HWND control : hActionButtons) ShowWindow(control, SW_HIDE);
    for (HWND control : hTargetButtons) ShowWindow(control, SW_HIDE);
    for (HWND control : hSongButtons) ShowWindow(control, SW_HIDE);
    for (HWND control : hSpellButtons) ShowWindow(control, SW_HIDE);
    for (HWND control : hTreasureButtons) ShowWindow(control, SW_HIDE);
    for (HWND control : hTreasureCharacterButtons) ShowWindow(control, SW_HIDE);
    ShowWindow(hBack, SW_HIDE);
    if (visible) {
        updateDungeon();
        refreshStatus();
    }
}

void updateActionButtons();

void enterDungeonTile(int index)
{
    if (dungeonRow * 7 + dungeonColumn == dungeonEndIndex) return;
    int row = index / 7;
    int column = index % 7;
    if (std::abs(row - dungeonRow) + std::abs(column - dungeonColumn) != 1) return;
    bool alreadyCleared = dungeonVisited[index];
    dungeonRow = row;
    dungeonColumn = column;
    dungeonVisited[index] = true;
    if (alreadyCleared) {
        appendLog("You re-entered a cleared room.");
        updateDungeon();
        return;
    }
    if (dungeon[index] == RoomType::End) {
        appendLog("You found the dungeon's end tile! Exploration is complete.");
        updateDungeon();
    }
    else if (dungeon[index] == RoomType::Treasure) {
        std::uniform_int_distribution<int> treasureRoll(0, 4);
        selectedTreasure = treasureRoll(rng);
        selectingTreasureCharacter = true;
        const wchar_t* itemNames[] = { L"Ring of Damage", L"Ring of Defense", L"Ring of Healing", L"Ring of Ability", L"Ring of Health" };
        const std::string itemMessages[] = { "Ring of Damage", "Ring of Defense", "Ring of Healing", "Ring of Ability", "Ring of Health" };
        SetWindowTextW(hPrompt, (std::wstring(L"Treasure found: ") + itemNames[selectedTreasure] + L". Choose a character:").c_str());
        for (HWND button : hTreasureButtons) ShowWindow(button, SW_HIDE);
        for (int i = 0; i < 3; ++i) {
            SetWindowTextW(hTreasureCharacterButtons[i], widen(party[i]->GetDisplayName()).c_str());
            ShowWindow(hTreasureCharacterButtons[i], SW_SHOW);
        }
        for (HWND control : hDungeonTiles) ShowWindow(control, SW_HIDE);
        ShowWindow(hDungeonInfo, SW_HIDE);
        ShowWindow(hPrompt, SW_SHOW);
        MoveWindow(hLog, 300, 140, 475, 395, TRUE);
        appendLog("You found " + itemMessages[selectedTreasure] + ". Choose a character to receive it.");
        return;
    }
    else if (dungeon[index] == RoomType::Enemy) {
        enemies.clear();
        enemies.push_back(std::make_unique<skeleton>());
        enemies.push_back(std::make_unique<skeleton>());
        std::vector<std::string> enemyNames = { "Skeleton 1", "Skeleton 2" };
        for (auto& member : party) member->SetEnemyNames(enemyNames);
        currentActor = 0;
        turnManaGranted = false;
        startTurnBonusesApplied = false;
        for (HWND button : hActionButtons) EnableWindow(button, TRUE);
        SetWindowTextW(hLog, L"You entered an Enemy room! Defeat the skeletons to continue exploring.\r\n");
        showCombat(true);
        updateActionButtons();
        refreshStatus();
        return;
    }
    else if (dungeon[index] == RoomType::Trap) {
        std::uniform_int_distribution<int> trapRoll(0, 3);
        currentTrap = static_cast<TrapType>(trapRoll(rng));
        selectingTrapCharacter = true;
        const wchar_t* trapNames[] = { L"Arrow Trap", L"Animal Trap", L"Magic Trap", L"Riddle Trap" };
        SetWindowTextW(hPrompt, (std::wstring(trapNames[static_cast<int>(currentTrap)]) + L"! Choose a character to trigger it:").c_str());
        for (int i = 0; i < 3; ++i) {
            SetWindowTextW(hTreasureCharacterButtons[i], widen(party[i]->GetDisplayName()).c_str());
            ShowWindow(hTreasureCharacterButtons[i], SW_SHOW);
            EnableWindow(hTreasureCharacterButtons[i], party[i]->GetHP() > 0);
        }
        for (HWND control : hDungeonTiles) ShowWindow(control, SW_HIDE);
        ShowWindow(hDungeonInfo, SW_HIDE);
        ShowWindow(hPrompt, SW_SHOW);
        MoveWindow(hLog, 300, 140, 475, 395, TRUE);
        appendLog("You entered a Trap room. Choose a character to trigger the trap.");
        return;
    }
    updateDungeon();
}

void chooseTreasure(int treasure)
{
    selectedTreasure = treasure;
    selectingTreasureCharacter = true;
    SetWindowTextW(hPrompt, L"Choose a character to receive the treasure:");
    for (int i = 0; i < 3; ++i) {
        SetWindowTextW(hTreasureCharacterButtons[i], widen(party[i]->GetDisplayName()).c_str());
        ShowWindow(hTreasureCharacterButtons[i], SW_SHOW);
    }
    for (HWND button : hTreasureButtons) ShowWindow(button, SW_HIDE);
}

int trapProficiency(int character)
{
    bool fighter = dynamic_cast<knight*>(party[character].get()) != nullptr;
    bool rangerMember = dynamic_cast<ranger*>(party[character].get()) != nullptr;
    bool wizardMember = dynamic_cast<wizard*>(party[character].get()) != nullptr;
    bool bardMember = dynamic_cast<bard*>(party[character].get()) != nullptr;
    if (currentTrap == TrapType::Arrow) return fighter ? 2 : (rangerMember ? 1 : 0);
    if (currentTrap == TrapType::Animal) return rangerMember ? 2 : (fighter ? 1 : 0);
    if (currentTrap == TrapType::Magic) return wizardMember ? 2 : (bardMember ? 1 : 0);
    return bardMember ? 2 : (wizardMember ? 1 : 0);
}

void triggerTrap(int character)
{
    const char* trapNames[] = { "Arrow Trap", "Animal Trap", "Magic Trap", "Riddle Trap" };
    int proficiency = trapProficiency(character);
    std::string name = party[character]->GetDisplayName();
    if (proficiency == 2) {
        appendLog(name + " avoided the " + trapNames[static_cast<int>(currentTrap)] + ".");
    } else {
        std::uniform_int_distribution<int> chance(1, 3);
        int triggerChance = chance(rng);
        bool triggered = proficiency == 1 ? triggerChance == 1 : triggerChance <= 2;
        if (!triggered) {
            appendLog(name + " avoids the " + trapNames[static_cast<int>(currentTrap)] + ".");
        } else {
            std::uniform_int_distribution<int> damageRoll(3, 5);
            int trapDamage = damageRoll(rng) - defenseBonuses[character];
            if (trapDamage < 0) trapDamage = 0;
            int actualDamage = trapDamage - party[character]->GetDamageReduction();
            if (actualDamage < 0) actualDamage = 0;
            party[character]->TakeDamage(trapDamage);
            appendLog(name + " triggers the " + trapNames[static_cast<int>(currentTrap)] + " and takes " + std::to_string(actualDamage) + " damage.");
        }
    }
    selectingTrapCharacter = false;
    for (HWND button : hTreasureCharacterButtons) ShowWindow(button, SW_HIDE);
    refreshStatus();
    showDungeon(true);
}

void giveTreasure(int character)
{
    if (itemCounts[character] >= 2) {
        appendLog(party[character]->GetDisplayName() + " already has 2 items. Choose another character.");
        return;
    }
    ++itemCounts[character];
    if (selectedTreasure == 0) ++damageBonuses[character];
    else if (selectedTreasure == 1) ++defenseBonuses[character];
    else if (selectedTreasure == 2) ++healthBonuses[character];
    else if (selectedTreasure == 3) ++abilityBonuses[character];
    else party[character]->IncreaseMaxHP(3);
    const char* itemNames[] = { "Ring of Damage", "Ring of Defense", "Ring of Healing", "Ring of Ability", "Ring of Health" };
    inventory[character].push_back(itemNames[selectedTreasure]);
    appendLog(std::string(itemNames[selectedTreasure]) + " was given to " + party[character]->GetDisplayName() + ".");
    refreshStatus();
    selectingTreasureCharacter = false;
    for (HWND button : hTreasureCharacterButtons) ShowWindow(button, SW_HIDE);
    showDungeon(true);
}

int classIdFor(const character* member)
{
    if (dynamic_cast<const knight*>(member)) return 1;
    if (dynamic_cast<const ranger*>(member)) return 2;
    if (dynamic_cast<const wizard*>(member)) return 3;
    return 4;
}

std::string characterName(const character* member)
{
    std::string display = member->GetDisplayName();
    size_t separator = display.find(' ');
    return separator == std::string::npos ? display : display.substr(separator + 1);
}

std::string currentTimeString()
{
    std::time_t now = std::time(nullptr);
    std::tm localTime{};
    localtime_s(&localTime, &now);
    std::ostringstream output;
    output << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return output.str();
}

std::string saveFileName(int slot)
{
    return "FullPartyRPG_save" + std::to_string(slot) + ".txt";
}

std::wstring slotLabel(int slot)
{
    std::ifstream file(saveFileName(slot));
    std::string header, savedTime;
    int version = 0;
    if (file && file >> header >> version && header == "FULL_PARTY_RPG_SAVE") {
        file >> std::ws;
        if (file.peek() == 'S') {
            std::string marker;
            file >> marker >> std::quoted(savedTime);
        }
    }
    std::string label = "Slot " + std::to_string(slot) + (savedTime.empty() ? " (Empty)" : " (" + savedTime + ")");
    return widen(label);
}

void updateSlotMenuLabels()
{
    if (!saveMenu || !loadMenu || !eraseMenu) return;
    for (int i = 0; i < 3; ++i) {
        int slot = i + 1;
        std::wstring label = slotLabel(slot);
        ModifyMenuW(saveMenu, IDM_SAVE_SLOT1 + i, MF_BYCOMMAND | MF_STRING, IDM_SAVE_SLOT1 + i, label.c_str());
        ModifyMenuW(loadMenu, IDM_LOAD_SLOT1 + i, MF_BYCOMMAND | MF_STRING, IDM_LOAD_SLOT1 + i, label.c_str());
        ModifyMenuW(eraseMenu, IDM_ERASE_SLOT1 + i, MF_BYCOMMAND | MF_STRING, IDM_ERASE_SLOT1 + i, label.c_str());
    }
    DrawMenuBar(GetParent(hStart));
}

void saveGame(int slot)
{
    std::ofstream file(saveFileName(slot));
    if (!file) { MessageBoxW(nullptr, L"Could not write the save file.", L"Save", MB_OK | MB_ICONERROR); return; }
    file << "FULL_PARTY_RPG_SAVE 1\n";
    file << "SAVED_TIME " << std::quoted(currentTimeString()) << '\n';
    file << dungeonRow << ' ' << dungeonColumn << ' ' << dungeonEndIndex << '\n';
    for (auto room : dungeon) file << static_cast<int>(room) << ' ';
    file << '\n';
    for (bool visited : dungeonVisited) file << visited << ' ';
    file << '\n';
    file << party.size() << '\n';
    for (const auto& member : party) {
        int special = 0;
        if (auto* knightMember = dynamic_cast<knight*>(member.get())) special = knightMember->GetSpirit();
        else if (auto* wizardMember = dynamic_cast<wizard*>(member.get())) special = wizardMember->GetMana();
        else if (auto* bardMember = dynamic_cast<bard*>(member.get())) special = bardMember->GetCurrentSong() == "Song of Defense" ? 2 : bardMember->GetCurrentSong() == "Song of Healing" ? 3 : 1;
        file << classIdFor(member.get()) << ' ' << std::quoted(characterName(member.get())) << ' '
             << member->GetHP() << ' ' << member->GetMaxHP() << ' ' << special << ' ' << itemCounts[&member - &party[0]] << '\n';
    }
    for (const auto& items : inventory) { file << items.size(); for (const auto& item : items) file << ' ' << std::quoted(item); file << '\n'; }
    for (auto value : damageBonuses) file << value << ' '; file << '\n';
    for (auto value : defenseBonuses) file << value << ' '; file << '\n';
    for (auto value : healthBonuses) file << value << ' '; file << '\n';
    for (auto value : abilityBonuses) file << value << ' '; file << '\n';
    file.close();
    updateSlotMenuLabels();
    MessageBoxW(nullptr, widen("Game saved to " + saveFileName(slot) + ".").c_str(), L"Save", MB_OK);
}

void loadGame(int slot)
{
    std::ifstream file(saveFileName(slot));
    std::string header;
    int version = 0;
    if (!file || !(file >> header >> version) || header != "FULL_PARTY_RPG_SAVE") {
        MessageBoxW(nullptr, L"No valid save file was found.", L"Load", MB_OK | MB_ICONWARNING); return;
    }
    file >> std::ws;
    if (file.peek() == 'S') {
        std::string marker, savedTime;
        file >> marker >> std::quoted(savedTime);
    }
    file >> dungeonRow >> dungeonColumn >> dungeonEndIndex;
    for (auto& room : dungeon) { int value; file >> value; room = static_cast<RoomType>(value); }
    for (bool& visited : dungeonVisited) file >> visited;
    size_t partySize; file >> partySize;
    party.clear();
    for (size_t i = 0; i < partySize && i < 3; ++i) {
        int classId, hp, maxHP, special, count; std::string name;
        file >> classId >> std::quoted(name) >> hp >> maxHP >> special >> count;
        if (classId == 1) party.push_back(std::make_unique<knight>());
        else if (classId == 2) party.push_back(std::make_unique<ranger>());
        else if (classId == 3) party.push_back(std::make_unique<wizard>());
        else party.push_back(std::make_unique<bard>());
        runWithInput(name + "\n", [&] { party.back()->SetName(); });
        party.back()->LoadHealth(hp, maxHP);
        itemCounts[i] = count;
        if (auto* knightMember = dynamic_cast<knight*>(party.back().get())) knightMember->SetSpirit(special);
        else if (auto* wizardMember = dynamic_cast<wizard*>(party.back().get())) wizardMember->SetMana(special);
        else if (auto* bardMember = dynamic_cast<bard*>(party.back().get())) bardMember->SetSong(special);
    }
    for (auto& items : inventory) { size_t count; file >> count; items.clear(); for (size_t i = 0; i < count; ++i) { std::string item; file >> std::quoted(item); items.push_back(item); } }
    for (auto& value : damageBonuses) file >> value;
    for (auto& value : defenseBonuses) file >> value;
    for (auto& value : healthBonuses) file >> value;
    for (auto& value : abilityBonuses) file >> value;
    std::vector<std::string> partyNames, enemyNames = { "Skeleton 1", "Skeleton 2" };
    for (auto& member : party) partyNames.push_back(member->GetDisplayName());
    for (auto& member : party) { member->SetPartyNames(partyNames); member->SetEnemyNames(enemyNames); }
    currentActor = 0; startTurnBonusesApplied = false; turnManaGranted = false;
    SetWindowTextW(hLog, L"Game loaded.\r\n");
    showDungeon(true);
    MessageBoxW(nullptr, widen("Game loaded from " + saveFileName(slot) + ".").c_str(), L"Load", MB_OK);
}

void eraseGame(int slot)
{
    if (slotLabel(slot).find(L"Empty") != std::wstring::npos) return;
    std::wstring question = L"Erase " + slotLabel(slot) + L"?";
    if (MessageBoxW(nullptr, question.c_str(), L"Erase Save", MB_YESNO | MB_ICONWARNING) == IDYES) {
        std::remove(saveFileName(slot).c_str());
        updateSlotMenuLabels();
    }
}

void updateActionButtons()
{
    if (party.empty()) return;
    if (!startTurnBonusesApplied) {
        if (healthBonuses[currentActor] > 0) {
            party[currentActor]->Heal(healthBonuses[currentActor]);
            appendLog(party[currentActor]->GetDisplayName() + " restores " + std::to_string(healthBonuses[currentActor]) + " HP from its Healing item.");
        }
        if (auto* knightMember = dynamic_cast<knight*>(party[currentActor].get()))
            knightMember->AddSpirit(abilityBonuses[currentActor]);
        if (auto* wizardMember = dynamic_cast<wizard*>(party[currentActor].get())) {
            wizardMember->StartTurn(abilityBonuses[currentActor]);
            turnManaGranted = true;
        }
        startTurnBonusesApplied = true;
        refreshStatus();
    }
    const wchar_t* labels[3];
    if (dynamic_cast<knight*>(party[currentActor].get())) labels[0] = L"Sword Attack", labels[1] = L"Provoke", labels[2] = L"Defend";
    else if (dynamic_cast<ranger*>(party[currentActor].get())) labels[0] = L"Bow Attack", labels[1] = L"Herbalism", labels[2] = L"Study Enemy";
    else if (dynamic_cast<wizard*>(party[currentActor].get())) labels[0] = L"Staff Attack", labels[1] = L"Focus Mana", labels[2] = L"Cast Spell";
    else labels[0] = L"Continue Song", labels[1] = L"Change Song", labels[2] = L"Dagger Attack";
    for (int i = 0; i < 3; ++i) { SetWindowTextW(hActionButtons[i], labels[i]); ShowWindow(hActionButtons[i], SW_SHOW); }
    ShowWindow(hBack, SW_HIDE);
    std::wstring prompt = L"Choose an action for Character " + std::to_wstring(currentActor + 1) + L" (" + widen(party[currentActor]->GetDisplayName()) + L")";
    SetWindowTextW(hPrompt, prompt.c_str());
}

bool actionNeedsTarget(int action)
{
    if (dynamic_cast<knight*>(party[currentActor].get())) return action == 1;
    if (dynamic_cast<ranger*>(party[currentActor].get())) return true;
    if (dynamic_cast<wizard*>(party[currentActor].get())) return action == 1 || (action == 3 && (pendingSpell == 1 || pendingSpell == 3));
    return action == 3;
}

void enemyTurn();

void applyPlayerAction(const std::vector<int>& result, int actor)
{
    if (result.size() != 3) return;
    int action = result[0], target = result[1], amount = result[2];
    std::string actorName = party[actor]->GetDisplayName();
    std::ostringstream text;
    if (action == PROVOKE || action == DEFEND) {
        party[actor]->SetDamageReduction(amount);
        if (action == PROVOKE) {
            forcedTarget = actor;
            text << actorName << " uses Provoke: skeletons will target the Knight, and damage is reduced by " << amount << " this turn.";
        } else {
            text << actorName << " uses Defend: incoming damage is reduced by " << amount << " this turn.";
        }
        appendLog(text.str());
    } else if ((action == ATTACK || action == RANGER_ATTACK || action == DARK_BLAST) && target >= 4 && target <= 5) {
        int enemy = target - 4;
        int damage = amount + teamDamageBonus + damageBonuses[actor];
        if (action == RANGER_ATTACK) enemies[enemy]->SetAttackReduction(1 + abilityBonuses[actor]);
        int actualDamage = enemies[enemy]->TakeDamage(damage);
        if (action == ATTACK && dynamic_cast<knight*>(party[actor].get()))
            text << actorName << " uses Sword Attack and deals " << actualDamage << " damage to Skeleton " << enemy + 1 << ".";
        else if (action == RANGER_ATTACK)
            text << actorName << " uses Bow Attack and deals " << actualDamage << " damage to Skeleton " << enemy + 1 << "; vATKv-" << 1 + abilityBonuses[actor] << " is applied to its next attack.";
        else if (action == ATTACK && dynamic_cast<wizard*>(party[actor].get()))
            text << actorName << " uses Staff Attack and deals " << actualDamage << " damage to Skeleton " << enemy + 1 << ".";
        else if (action == DARK_BLAST)
            text << actorName << " casts Darkblast and deals " << actualDamage << " damage to Skeleton " << enemy + 1 << ".";
        else
            text << actorName << " deals " << actualDamage << " damage to Skeleton " << enemy + 1 << ".";
        appendLog(text.str());
    } else if (action == JUMPING_SPARK) {
        appendLog(actorName + " casts Jumping Spark: both skeletons take " + std::to_string(amount) + " damage.");
        for (auto& enemy : enemies) enemy->TakeDamage(amount + teamDamageBonus + damageBonuses[actor]);
    } else if (action == CHILLING_AIR && target >= 4 && target <= 5) {
        enemies[target - 4]->SkipNextTurn();
        appendLog(actorName + " casts Chilling Air: Skeleton " + std::to_string(target - 3) + " skips its next turn.");
    } else if (action == HEAL && target >= 1 && target <= 3) {
        party[target - 1]->Heal(amount);
        appendLog(actorName + " uses Herbalism to heal " + party[target - 1]->GetDisplayName() + " for " + std::to_string(amount) + " HP.");
    } else if (action == STUDY && target >= 4 && target <= 5) {
        enemies[target - 4]->SetNextAttackBonus(amount);
        appendLog(actorName + " uses Study Enemy: vDEFv-1 is applied to Skeleton " + std::to_string(target - 3) + "; its next attack is more effective.");
    } else if (action == BARD_SONG) {
        int effectAmount = amount + abilityBonuses[actor];
        if (target == 1) {
            teamDamageBonus = effectAmount;
            appendLog(actorName + " plays Song of Attack: the party deals " + std::to_string(effectAmount) + " extra damage until the Bard's next turn.");
        }
        else if (target == 2) {
            songDefense = effectAmount;
            for (auto& member : party) member->SetDamageReduction(effectAmount);
            appendLog(actorName + " plays Song of Defense: the party takes " + std::to_string(effectAmount) + " less damage until the Bard's next turn.");
        }
        else {
            for (auto& member : party) member->Heal(effectAmount);
            appendLog(actorName + " plays Song of Healing: the party restores " + std::to_string(effectAmount) + " HP.");
        }
    } else if (action == FOCUS_MANA) {
        appendLog(actorName + " uses Focus Mana and gains extra mana for future spells.");
    }
}

void finishCharacterAction(int target)
{
    if (party.empty()) return;
    std::ostringstream input;
    if (dynamic_cast<knight*>(party[currentActor].get())) { input << pendingAction << "\n"; if (pendingAction == 1) input << target << "\n"; }
    else if (dynamic_cast<ranger*>(party[currentActor].get())) input << pendingAction << "\n" << target << "\n";
    else if (dynamic_cast<wizard*>(party[currentActor].get())) { input << pendingAction << "\n"; if (pendingAction == 1) input << target << "\n"; if (pendingAction == 3) { input << pendingSpell << "\n"; if (pendingSpell == 1 || pendingSpell == 3) input << target << "\n"; } }
    else { input << pendingAction << "\n"; if (pendingAction == 2) input << pendingSong << "\n"; if (pendingAction == 3) input << target << "\n"; }
    std::vector<int> actionResult;
    runWithInput(input.str(), [&] { actionResult = party[currentActor]->Action(); });
    if (dynamic_cast<wizard*>(party[currentActor].get()) && pendingAction == 3
        && !actionResult.empty() && actionResult[0] == NO_ACTION) {
        static_cast<wizard*>(party[currentActor].get())->UndoStartTurn(1 + abilityBonuses[currentActor]);
        appendLog("Not enough mana to cast that spell. The Wizard's turn starts over without gaining mana.");
        for (HWND button : hTargetButtons) ShowWindow(button, SW_HIDE);
        for (HWND button : hSpellButtons) ShowWindow(button, SW_HIDE);
        refreshStatus();
        updateActionButtons();
        return;
    }
    applyPlayerAction(actionResult, currentActor);
    refreshStatus();
    for (HWND button : hTargetButtons) ShowWindow(button, SW_HIDE);
    for (HWND button : hSongButtons) ShowWindow(button, SW_HIDE);
    for (HWND button : hSpellButtons) ShowWindow(button, SW_HIDE);
    ShowWindow(hBack, SW_HIDE);
    ++currentActor;
    turnManaGranted = false;
    startTurnBonusesApplied = false;
    if (currentActor == 3) {
        enemyTurn();
        currentActor = 0;
        refreshStatus();
    }
    bool enemiesDead = std::all_of(enemies.begin(), enemies.end(), [](const auto& e) { return e->GetHP() <= 0; });
    bool partyDead = std::all_of(party.begin(), party.end(), [](const auto& p) { return p->GetHP() <= 0; });
    if (enemiesDead) {
        appendLog("The Enemy room is cleared.");
        for (HWND button : hActionButtons) EnableWindow(button, FALSE);
        showDungeon(true);
        return;
    }
    if (partyDead) { appendLog("You lose! Click New Party to try again."); for (HWND button : hActionButtons) EnableWindow(button, FALSE); return; }
    updateActionButtons();
}

void chooseAction(int action)
{
    pendingAction = action;
    selectingPartyTarget = dynamic_cast<ranger*>(party[currentActor].get()) && action == 2;
    if (dynamic_cast<bard*>(party[currentActor].get())) {
        teamDamageBonus = 0;
        songDefense = 0;
        for (auto& member : party) member->ResetDamageReduction();
    }
    if (dynamic_cast<bard*>(party[currentActor].get()) && action == 2) {
        SetWindowTextW(hPrompt, L"Choose a new song:");
        SetWindowTextW(hSongButtons[0], L"Attack");
        SetWindowTextW(hSongButtons[1], L"Defense");
        SetWindowTextW(hSongButtons[2], L"Healing");
        for (HWND button : hSongButtons) ShowWindow(button, SW_SHOW);
        ShowWindow(hBack, SW_SHOW);
        for (HWND button : hActionButtons) ShowWindow(button, SW_HIDE);
        return;
    }
    if (dynamic_cast<wizard*>(party[currentActor].get()) && action == 3) {
        SetWindowTextW(hPrompt, L"Choose a spell:");
        SetWindowTextW(hSpellButtons[0], (std::to_wstring(GameConfig::WizardDarkblastCost) + L" - Darkblast").c_str());
        SetWindowTextW(hSpellButtons[1], (std::to_wstring(GameConfig::WizardJumpingSparkCost) + L" - Jumping Spark").c_str());
        SetWindowTextW(hSpellButtons[2], (std::to_wstring(GameConfig::WizardChillingAirCost) + L" - Chilling Air").c_str());
        auto* wizardMember = dynamic_cast<wizard*>(party[currentActor].get());
        const int spellCosts[] = { GameConfig::WizardDarkblastCost, GameConfig::WizardJumpingSparkCost, GameConfig::WizardChillingAirCost };
        for (int i = 0; i < 3; ++i) {
            ShowWindow(hSpellButtons[i], SW_SHOW);
            EnableWindow(hSpellButtons[i], wizardMember->GetMana() >= spellCosts[i]);
        }
        ShowWindow(hBack, SW_SHOW);
        for (HWND button : hActionButtons) ShowWindow(button, SW_HIDE);
        ShowWindow(hBack, SW_SHOW);
        return;
    }
    if (actionNeedsTarget(action)) {
        SetWindowTextW(hPrompt, L"Choose a target:");
        if (selectingPartyTarget) {
            for (int i = 0; i < 3; ++i) {
                SetWindowTextW(hTargetButtons[i], widen(party[i]->GetDisplayName()).c_str());
                ShowWindow(hTargetButtons[i], SW_SHOW);
                EnableWindow(hTargetButtons[i], party[i]->GetHP() > 0);
            }
        } else {
            for (int i = 0; i < 2; ++i) {
                SetWindowTextW(hTargetButtons[i], (i == 0) ? L"Skeleton 1" : L"Skeleton 2");
                ShowWindow(hTargetButtons[i], SW_SHOW);
                EnableWindow(hTargetButtons[i], enemies[i]->GetHP() > 0);
            }
            ShowWindow(hTargetButtons[2], SW_HIDE);
        }
        for (HWND button : hActionButtons) ShowWindow(button, SW_HIDE);
        ShowWindow(hBack, SW_SHOW);
    } else finishCharacterAction(0);
}

void chooseSpell(int spell)
{
    pendingSpell = spell;
    if (actionNeedsTarget(3)) {
        SetWindowTextW(hPrompt, L"Choose a target:");
        for (int i = 0; i < 2; ++i) {
            SetWindowTextW(hTargetButtons[i], i == 0 ? L"Skeleton 1" : L"Skeleton 2");
            ShowWindow(hTargetButtons[i], SW_SHOW);
            EnableWindow(hTargetButtons[i], enemies[i]->GetHP() > 0);
        }
        ShowWindow(hTargetButtons[2], SW_HIDE);
        for (HWND button : hSpellButtons) ShowWindow(button, SW_HIDE);
    } else {
        finishCharacterAction(0);
    }
}

void populateClassChoices()
{
    const LPCWSTR names[] = { L"Knight", L"Ranger", L"Wizard", L"Bard" };
    for (HWND control : hClass) {
        SendMessageW(control, CB_RESETCONTENT, 0, 0);
        for (int i = 0; i < 4; ++i) {
            int item = static_cast<int>(SendMessageW(control, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(names[i])));
            SendMessageW(control, CB_SETITEMDATA, item, i);
        }
        SendMessageW(control, CB_SETCURSEL, static_cast<WPARAM>(-1), 0);
    }
}

int selectedClass(HWND control)
{
    int selected = static_cast<int>(SendMessageW(control, CB_GETCURSEL, 0, 0));
    if (selected < 0) return -1;
    return static_cast<int>(SendMessageW(control, CB_GETITEMDATA, selected, 0));
}

void refreshClassChoices()
{
    int selected[3] = { selectedClass(hClass[0]), selectedClass(hClass[1]), selectedClass(hClass[2]) };
    const LPCWSTR names[] = { L"Knight", L"Ranger", L"Wizard", L"Bard" };
    for (int slot = 0; slot < 3; ++slot) {
        SendMessageW(hClass[slot], CB_RESETCONTENT, 0, 0);
        for (int classId = 0; classId < 4; ++classId) {
            bool usedElsewhere = false;
            for (int other = 0; other < 3; ++other)
                if (other != slot && selected[other] == classId) usedElsewhere = true;
            if (!usedElsewhere) {
                int item = static_cast<int>(SendMessageW(hClass[slot], CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(names[classId])));
                SendMessageW(hClass[slot], CB_SETITEMDATA, item, classId);
            }
        }
        for (int item = 0; item < static_cast<int>(SendMessageW(hClass[slot], CB_GETCOUNT, 0, 0)); ++item)
            if (static_cast<int>(SendMessageW(hClass[slot], CB_GETITEMDATA, item, 0)) == selected[slot])
                SendMessageW(hClass[slot], CB_SETCURSEL, item, 0);
    }
}

bool startGame()
{
    party.clear(); enemies.clear();
    std::vector<int> choices;
    for (HWND control : hClass) choices.push_back(selectedClass(control));
    for (int choice : choices) if (choice < 0) { MessageBoxW(nullptr, L"Choose a class for all three characters.", L"Party Setup", MB_OK | MB_ICONWARNING); return false; }
    if (choices[0] == choices[1] || choices[0] == choices[2] || choices[1] == choices[2]) {
        MessageBoxW(nullptr, L"Each character must have a different class.", L"Party Setup", MB_OK | MB_ICONWARNING); return false;
    }
    for (HWND control : hName) if (controlText(control).empty()) { MessageBoxW(nullptr, L"Enter a name for all three characters.", L"Party Setup", MB_OK | MB_ICONWARNING); return false; }

    for (int i = 0; i < 3; ++i) {
        if (choices[i] == 0) party.push_back(std::make_unique<knight>());
        else if (choices[i] == 1) party.push_back(std::make_unique<ranger>());
        else if (choices[i] == 2) party.push_back(std::make_unique<wizard>());
        else party.push_back(std::make_unique<bard>());
        std::string name = controlText(hName[i]);
        runWithInput(name + "\n", [&] { party.back()->SetName(); });
    }
    enemies.push_back(std::make_unique<skeleton>());
    enemies.push_back(std::make_unique<skeleton>());
    std::vector<std::string> partyNames;
    for (auto& member : party) partyNames.push_back(member->GetDisplayName());
    std::vector<std::string> enemyNames = { "Skeleton 1", "Skeleton 2" };
    for (auto& member : party) { member->SetPartyNames(partyNames); member->SetEnemyNames(enemyNames); }
    currentActor = 0;
    pendingAction = 0;
    turnManaGranted = false;
    startTurnBonusesApplied = false;
    damageBonuses.fill(0);
    defenseBonuses.fill(0);
    healthBonuses.fill(0);
    abilityBonuses.fill(0);
    itemCounts.fill(0);
    for (auto& items : inventory) items.clear();
    for (HWND button : hActionButtons) EnableWindow(button, TRUE);
    SetWindowTextW(hLog, L"Party ready! Explore the dungeon by moving to an adjacent room.\r\n");
    generateDungeon();
    showDungeon(true);
    refreshStatus();
    return true;
}

void enemyTurn()
{
    std::vector<int> living;
    for (int i = 0; i < static_cast<int>(party.size()); ++i) if (party[i]->GetHP() > 0) living.push_back(i);
    if (living.empty()) return;
    std::uniform_int_distribution<int> pick(0, static_cast<int>(living.size()) - 1);
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (enemies[i]->GetHP() <= 0) continue;
        if (enemies[i]->IsSkippingTurn()) {
            appendLog("Skeleton " + std::to_string(i + 1) + " skips its turn because of Chilling Air.");
            enemies[i]->ClearSkipTurn();
            continue;
        }
        int target = living[pick(rng)];
        if (forcedTarget >= 0 && forcedTarget < static_cast<int>(party.size()) && party[forcedTarget]->GetHP() > 0)
            target = forcedTarget;
        auto result = enemies[i]->Action(target + 1);
        int incomingDamage = result[2] - defenseBonuses[target];
        if (incomingDamage < 0) incomingDamage = 0;
        int damageAfterDefense = incomingDamage - party[target]->GetDamageReduction();
        if (damageAfterDefense < 0) damageAfterDefense = 0;
        party[target]->TakeDamage(incomingDamage);
        std::ostringstream text;
        text << "Skeleton " << i + 1 << " attacks " << party[target]->GetDisplayName() << " for " << damageAfterDefense << " damage.";
        appendLog(text.str());
    }
    forcedTarget = -1;
}

void takeTurn()
{
    int actor = static_cast<int>(SendMessageW(hActor, CB_GETCURSEL, 0, 0));
    int action = static_cast<int>(SendMessageW(hAction, CB_GETCURSEL, 0, 0)) + 1;
    int target = static_cast<int>(SendMessageW(hTarget, CB_GETCURSEL, 0, 0)) + 1;
    if (actor < 0 || actor >= static_cast<int>(party.size())) return;
    std::ostringstream input;
    if (dynamic_cast<knight*>(party[actor].get())) { input << action << "\n"; if (action == 1) input << target << "\n"; }
    else if (dynamic_cast<ranger*>(party[actor].get())) input << action << "\n" << target << "\n";
    else if (dynamic_cast<wizard*>(party[actor].get())) { input << action << "\n"; if (action == 1) input << target << "\n"; if (action == 3) input << "1\n" << target << "\n"; }
    else { input << action << "\n"; if (action == 2) input << "1\n"; if (action == 3) input << target << "\n"; }
    std::string result = runWithInput(input.str(), [&] { party[actor]->Action(); });
    if (!result.empty()) appendLog(result);
    enemyTurn(); refreshStatus();
    bool enemiesDead = std::all_of(enemies.begin(), enemies.end(), [](const auto& e) { return e->GetHP() <= 0; });
    bool partyDead = std::all_of(party.begin(), party.end(), [](const auto& p) { return p->GetHP() <= 0; });
    if (enemiesDead) appendLog("You win!"); else if (partyDead) appendLog("You lose! Click New Game to try again.");
}

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance); UNREFERENCED_PARAMETER(lpCmdLine);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FULLPARTYRPG, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    return static_cast<int>(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wc{}; wc.cbSize = sizeof(WNDCLASSEXW); wc.lpfnWndProc = WndProc; wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FULLPARTYRPG)); wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); wc.lpszMenuName = MAKEINTRESOURCEW(IDC_FULLPARTYRPG);
    wc.lpszClassName = szWindowClass; wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wc);
}

HWND makeControl(HWND parent, LPCWSTR type, LPCWSTR text, DWORD style, int id, int x, int y, int width, int height)
{
    return CreateWindowW(type, text, WS_CHILD | WS_VISIBLE | style, x, y, width, height, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), hInst, nullptr);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND window = CreateWindowW(szWindowClass, L"Full Party RPG", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 850, 620, nullptr, nullptr, hInstance, nullptr);
    if (!window) return FALSE;
    ShowWindow(window, nCmdShow); UpdateWindow(window); return TRUE;
}

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        {
        hSetupLabels[0] = makeControl(window, L"STATIC", L"Choose three unique classes and enter a name for each character:", 0, 0, 20, 20, 520, 24);
        for (int i = 0; i < 3; ++i) {
            int y = 65 + i * 65;
            std::wstring label = L"Character " + std::to_wstring(i + 1) + L":";
            hSetupLabels[1 + i] = makeControl(window, L"STATIC", label.c_str(), 0, 0, 20, y, 100, 24);
            hClass[i] = makeControl(window, L"COMBOBOX", L"", CBS_DROPDOWNLIST, IDC_CLASS1 + i, 125, y - 4, 180, 150);
            hName[i] = makeControl(window, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, IDC_NAME1 + i, 325, y - 4, 250, 24);
            hSetupLabels[4 + i] = makeControl(window, L"STATIC", L"Name", 0, 0, 585, y, 50, 24);
        }
        hStart = makeControl(window, L"BUTTON", L"Start Game", BS_PUSHBUTTON, IDC_START, 20, 275, 140, 34);
        HMENU fileMenu = GetSubMenu(GetMenu(window), 0);
        saveMenu = CreatePopupMenu();
        AppendMenuW(saveMenu, MF_STRING, IDM_SAVE_SLOT1, L"Slot 1");
        AppendMenuW(saveMenu, MF_STRING, IDM_SAVE_SLOT2, L"Slot 2");
        AppendMenuW(saveMenu, MF_STRING, IDM_SAVE_SLOT3, L"Slot 3");
        loadMenu = CreatePopupMenu();
        AppendMenuW(loadMenu, MF_STRING, IDM_LOAD_SLOT1, L"Slot 1");
        AppendMenuW(loadMenu, MF_STRING, IDM_LOAD_SLOT2, L"Slot 2");
        AppendMenuW(loadMenu, MF_STRING, IDM_LOAD_SLOT3, L"Slot 3");
        eraseMenu = CreatePopupMenu();
        AppendMenuW(eraseMenu, MF_STRING, IDM_ERASE_SLOT1, L"Slot 1");
        AppendMenuW(eraseMenu, MF_STRING, IDM_ERASE_SLOT2, L"Slot 2");
        AppendMenuW(eraseMenu, MF_STRING, IDM_ERASE_SLOT3, L"Slot 3");
        AppendMenuW(fileMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(saveMenu), L"&Save");
        AppendMenuW(fileMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(loadMenu), L"&Load");
        AppendMenuW(fileMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(eraseMenu), L"&Erase Save");
        AppendMenuW(fileMenu, MF_STRING, IDM_NEWGAME_MENU, L"&New Game");
        updateSlotMenuLabels();
        hPrompt = makeControl(window, L"STATIC", L"", 0, 0, 300, 16, 475, 28);
        hActionButtons[0] = makeControl(window, L"BUTTON", L"Action 1", BS_PUSHBUTTON, IDC_ACTION1, 300, 50, 140, 32);
        hActionButtons[1] = makeControl(window, L"BUTTON", L"Action 2", BS_PUSHBUTTON, IDC_ACTION2, 450, 50, 140, 32);
        hActionButtons[2] = makeControl(window, L"BUTTON", L"Action 3", BS_PUSHBUTTON, IDC_ACTION3, 600, 50, 140, 32);
        hTargetButtons[0] = makeControl(window, L"BUTTON", L"Skeleton 1", BS_PUSHBUTTON, IDC_TARGET1, 300, 50, 140, 32);
        hTargetButtons[1] = makeControl(window, L"BUTTON", L"Skeleton 2", BS_PUSHBUTTON, IDC_TARGET2, 450, 50, 140, 32);
        hTargetButtons[2] = makeControl(window, L"BUTTON", L"Character 3", BS_PUSHBUTTON, IDC_TARGET3, 600, 50, 140, 32);
        hSongButtons[0] = makeControl(window, L"BUTTON", L"Song of Attack", BS_PUSHBUTTON, IDC_SONG1, 300, 50, 145, 32);
        hSongButtons[1] = makeControl(window, L"BUTTON", L"Song of Defense", BS_PUSHBUTTON, IDC_SONG2, 455, 50, 145, 32);
        hSongButtons[2] = makeControl(window, L"BUTTON", L"Song of Healing", BS_PUSHBUTTON, IDC_SONG3, 610, 50, 145, 32);
        hSpellButtons[0] = makeControl(window, L"BUTTON", L"Darkblast", BS_PUSHBUTTON, IDC_SPELL1, 300, 50, 145, 32);
        hSpellButtons[1] = makeControl(window, L"BUTTON", L"Jumping Spark", BS_PUSHBUTTON, IDC_SPELL2, 455, 50, 145, 32);
        hSpellButtons[2] = makeControl(window, L"BUTTON", L"Chilling Air", BS_PUSHBUTTON, IDC_SPELL3, 610, 50, 145, 32);
        hBack = makeControl(window, L"BUTTON", L"<", BS_PUSHBUTTON, IDC_BACK, 760, 50, 35, 32);
        hTreasureButtons[0] = makeControl(window, L"BUTTON", L"Ring of Damage", BS_PUSHBUTTON, IDC_TREASURE1, 300, 50, 145, 32);
        hTreasureButtons[1] = makeControl(window, L"BUTTON", L"Ring of Defense", BS_PUSHBUTTON, IDC_TREASURE2, 455, 50, 145, 32);
        hTreasureButtons[2] = makeControl(window, L"BUTTON", L"Ring of Healing", BS_PUSHBUTTON, IDC_TREASURE3, 300, 90, 145, 32);
        hTreasureButtons[3] = makeControl(window, L"BUTTON", L"Ring of Ability", BS_PUSHBUTTON, IDC_TREASURE4, 455, 90, 145, 32);
        hTreasureButtons[4] = makeControl(window, L"BUTTON", L"Ring of Health", BS_PUSHBUTTON, IDC_TREASURE5, 610, 90, 145, 32);
        hTreasureCharacterButtons[0] = makeControl(window, L"BUTTON", L"Character 1", BS_PUSHBUTTON, IDC_TREASURE_CHAR1, 300, 50, 145, 32);
        hTreasureCharacterButtons[1] = makeControl(window, L"BUTTON", L"Character 2", BS_PUSHBUTTON, IDC_TREASURE_CHAR2, 455, 50, 145, 32);
        hTreasureCharacterButtons[2] = makeControl(window, L"BUTTON", L"Character 3", BS_PUSHBUTTON, IDC_TREASURE_CHAR3, 610, 50, 145, 32);
        hStatus = makeControl(window, L"EDIT", L"", ES_MULTILINE | ES_READONLY | WS_BORDER, IDC_STATUS, 20, 140, 260, 395);
        hLog = makeControl(window, L"EDIT", L"", ES_MULTILINE | ES_READONLY | WS_BORDER | WS_VSCROLL, IDC_LOG, 300, 140, 475, 395);
        hDungeonInfo = makeControl(window, L"STATIC", L"Dungeon", 0, 0, 300, 16, 495, 28);
        for (int row = 0; row < 7; ++row) {
            for (int column = 0; column < 7; ++column) {
                int index = row * 7 + column;
                hDungeonTiles[index] = makeControl(window, L"BUTTON", L"?", BS_OWNERDRAW,
                    IDC_DUNGEON_FIRST + index, 300 + column * 65, 55 + row * 50, 60, 45);
            }
        }
        populateClassChoices();
        showDungeon(false);
        showCombat(false);
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) >= IDC_DUNGEON_FIRST && LOWORD(wParam) < IDC_DUNGEON_FIRST + 49) enterDungeonTile(LOWORD(wParam) - IDC_DUNGEON_FIRST);
        else if (LOWORD(wParam) == IDC_TREASURE1) chooseTreasure(0);
        else if (LOWORD(wParam) == IDC_TREASURE2) chooseTreasure(1);
        else if (LOWORD(wParam) == IDC_TREASURE3) chooseTreasure(2);
        else if (LOWORD(wParam) == IDC_TREASURE4) chooseTreasure(3);
        else if (LOWORD(wParam) == IDC_TREASURE5) chooseTreasure(4);
        else if (LOWORD(wParam) == IDC_TREASURE_CHAR1) { if (selectingTrapCharacter) triggerTrap(0); else giveTreasure(0); }
        else if (LOWORD(wParam) == IDC_TREASURE_CHAR2) { if (selectingTrapCharacter) triggerTrap(1); else giveTreasure(1); }
        else if (LOWORD(wParam) == IDC_TREASURE_CHAR3) { if (selectingTrapCharacter) triggerTrap(2); else giveTreasure(2); }
        else if (LOWORD(wParam) == IDM_SAVE_SLOT1) saveGame(1);
        else if (LOWORD(wParam) == IDM_SAVE_SLOT2) saveGame(2);
        else if (LOWORD(wParam) == IDM_SAVE_SLOT3) saveGame(3);
        else if (LOWORD(wParam) == IDM_LOAD_SLOT1) loadGame(1);
        else if (LOWORD(wParam) == IDM_LOAD_SLOT2) loadGame(2);
        else if (LOWORD(wParam) == IDM_LOAD_SLOT3) loadGame(3);
        else if (LOWORD(wParam) == IDM_ERASE_SLOT1) eraseGame(1);
        else if (LOWORD(wParam) == IDM_ERASE_SLOT2) eraseGame(2);
        else if (LOWORD(wParam) == IDM_ERASE_SLOT3) eraseGame(3);
        else if (LOWORD(wParam) == IDC_ACTION1) chooseAction(1);
        else if (LOWORD(wParam) == IDC_ACTION2) chooseAction(2);
        else if (LOWORD(wParam) == IDC_ACTION3) chooseAction(3);
        else if (LOWORD(wParam) == IDC_TARGET1) finishCharacterAction(1);
        else if (LOWORD(wParam) == IDC_TARGET2) finishCharacterAction(2);
        else if (LOWORD(wParam) == IDC_TARGET3) finishCharacterAction(3);
        else if (LOWORD(wParam) == IDC_SONG1) { pendingSong = 1; finishCharacterAction(0); }
        else if (LOWORD(wParam) == IDC_SONG2) { pendingSong = 2; finishCharacterAction(0); }
        else if (LOWORD(wParam) == IDC_SONG3) { pendingSong = 3; finishCharacterAction(0); }
        else if (LOWORD(wParam) == IDC_SPELL1) chooseSpell(1);
        else if (LOWORD(wParam) == IDC_SPELL2) chooseSpell(2);
        else if (LOWORD(wParam) == IDC_SPELL3) chooseSpell(3);
        else if (LOWORD(wParam) == IDC_BACK) {
            pendingAction = 0;
            selectingPartyTarget = false;
            for (HWND button : hTargetButtons) ShowWindow(button, SW_HIDE);
            for (HWND button : hSongButtons) ShowWindow(button, SW_HIDE);
            for (HWND button : hSpellButtons) ShowWindow(button, SW_HIDE);
            updateActionButtons();
        }
        else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) >= IDC_CLASS1 && LOWORD(wParam) <= IDC_CLASS3) refreshClassChoices();
        else if (LOWORD(wParam) == IDC_TURN) takeTurn();
        else if (LOWORD(wParam) == IDC_START) startGame();
        else if (LOWORD(wParam) == IDC_RESET || LOWORD(wParam) == IDM_NEWGAME_MENU) {
            party.clear(); enemies.clear();
            showDungeon(false);
            for (int i = 0; i < 3; ++i) SetWindowTextW(hName[i], L"");
            populateClassChoices();
            showCombat(false);
        }
        break;
    case WM_DRAWITEM:
        {
            auto* draw = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            if (draw->CtlType == ODT_BUTTON && draw->CtlID >= IDC_DUNGEON_FIRST && draw->CtlID < IDC_DUNGEON_FIRST + 49) {
                int index = static_cast<int>(draw->CtlID - IDC_DUNGEON_FIRST);
                int row = index / 7;
                int column = index % 7;
                COLORREF fill = RGB(255, 255, 255);
                COLORREF text = RGB(0, 0, 0);
                if (row == dungeonRow && column == dungeonColumn) {
                    fill = RGB(255, 165, 0);
                    text = RGB(255, 255, 255);
                } else if (dungeon[index] != RoomType::Empty && dungeonVisited[index]) {
                    fill = RGB(65, 105, 225);
                    text = RGB(255, 255, 255);
                } else if (dungeon[index] != RoomType::Empty) {
                    fill = RGB(0, 0, 0);
                    text = RGB(255, 255, 255);
                }
                HBRUSH brush = CreateSolidBrush(fill);
                FillRect(draw->hDC, &draw->rcItem, brush);
                DeleteObject(brush);
                FrameRect(draw->hDC, &draw->rcItem, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
                SetBkMode(draw->hDC, TRANSPARENT);
                SetTextColor(draw->hDC, text);
                return TRUE;
            }
        }
        break;
    case WM_DESTROY: PostQuitMessage(0); break;
    default: return DefWindowProc(window, message, wParam, lParam);
    }
    return 0;
}
