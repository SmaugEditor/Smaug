#pragma once
#include "svar.h"

#include <vector>

class CSettingsLink;

class CSettingsRegister
{
public:
	void Register(CSettingsLink* link) { m_linkList.push_back(link); }
	void LoadSettings();
	void SaveSettings();
	std::vector<CSettingsLink*> m_linkList;
};

CSettingsRegister& GetSettingsRegister();

#define DEFINE_SETTINGS_MENU(displayName, table) static CSettingsLink s_##table##TableLink{&table, displayName};
// Maps up CSVarTables up to the CSettingsRegister
class CSettingsLink
{
public:
	CSettingsLink(CSVarTable* table, const char* displayName) { m_table = table; m_displayName = displayName; GetSettingsRegister().Register(this); }
	CSVarTable* m_table;
	const char* m_displayName;
};


class CSettingsMenu
{
public:
	
	void DrawMenu();
	void Enable() { m_shouldShow = true; }
	size_t m_selectedTabIndex = 0;
	bool m_shouldShow = false;
};

