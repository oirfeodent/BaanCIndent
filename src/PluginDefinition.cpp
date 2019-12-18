//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"

//
// put the headers you need here
//
#include "SciLexer.h"
#include <shlwapi.h>
#include <memory>

const TCHAR sectionName[] = TEXT("Settings");
const TCHAR configFileName[] = TEXT("BaanCIndent.ini");

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

TCHAR iniFilePath[MAX_PATH]; 
TCHAR enableDisableKey[] = TEXT("Enable");
TCHAR keywordsKey[] = TEXT("Keywords");
TCHAR sectionsKey[] = TEXT("Sections");
TCHAR sqlKey[] = TEXT("SQL");

bool enableDisablePlugin;
bool enableDisableKeywords;
bool enableDisableSections;
bool enableDisableSql;
bool mainSectionStarted;
bool onCaseStarted;

std::vector<std::string> cmntFwdIndent = { "dllusage",
											"functionusage" };
std::vector<std::string> cmntBckIndent = { "enddllusage",
											"endfunctionusage" };
std::vector<std::string> keyFwdIndent = { "for",
											"if",
											"repeat",
											"while" };
std::vector<std::string> keyBckIndent = { "endfor",
											"endif",
											"endwhile",
											"until" };
std::vector<std::string> mainNoSubSec = { "after.form.read:",
											"after.program",
											"after.receive.data:",
											"after.update.db.commit:",
											"before.display.object:",
											"before.new.object:",
											"before.program:",
											"declaration:",
											"on.display.total.line:",
											"on.error:" };

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	::WritePrivateProfileString(sectionName, enableDisableKey, enableDisablePlugin ? TEXT("1") : TEXT("0"), iniFilePath);
	::WritePrivateProfileString(sectionName, keywordsKey, enableDisableKeywords ? TEXT("1") : TEXT("0"), iniFilePath);
	::WritePrivateProfileString(sectionName, sectionsKey, enableDisableSections ? TEXT("1") : TEXT("0"), iniFilePath);
	::WritePrivateProfileString(sectionName, sqlKey, enableDisableSql ? TEXT("1") : TEXT("0"), iniFilePath);
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
	//
	// Firstly we get the parameters from your plugin config file (if any)
	//

	// get path of plugin configuration
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)iniFilePath);

	// if config path doesn't exist, we create it
	if (PathFileExists(iniFilePath) == FALSE)
	{
		::CreateDirectory(iniFilePath, NULL);
	}

	// make your plugin config file full file path name
	::PathAppend(iniFilePath, configFileName);

	// get the parameter value from plugin config
	enableDisablePlugin = ::GetPrivateProfileInt(sectionName, enableDisableKey, 0, iniFilePath) != 0;
	enableDisableKeywords = ::GetPrivateProfileInt(sectionName, keywordsKey, 0, iniFilePath) != 0;
	enableDisableSections = ::GetPrivateProfileInt(sectionName, sectionsKey, 0, iniFilePath) != 0;
	enableDisableSql = ::GetPrivateProfileInt(sectionName, sqlKey, 0, iniFilePath) != 0;

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool checkOnInit                // optional. Make this menu item be checked visually
    //            );

	// Shortcut :
	// bind to the shortcut Alt-B
	ShortcutKey* enableDisablePluginShKey = new ShortcutKey;
	enableDisablePluginShKey->_isAlt = true;
	enableDisablePluginShKey->_isCtrl = false;
	enableDisablePluginShKey->_isShift = false;
	enableDisablePluginShKey->_key = 0x42; //VK_B

	setCommand(0, enableDisableKey, toggleEnableDisable, enableDisablePluginShKey, enableDisablePlugin);
	setCommand(1, TEXT("---"), NULL, NULL, false);
	setCommand(2, keywordsKey, toggleKeywords, nullptr, enableDisableKeywords);
	setCommand(3, sectionsKey, toggleSections, nullptr, enableDisableSections);
	setCommand(4, sqlKey, toggleSql, nullptr, enableDisableSql);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[0]._pShKey;
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool checkOnInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = checkOnInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void toggleEnableDisable()
{
	enableDisablePlugin = !enableDisablePlugin;

	HMENU hMenu = GetMenu(nppData._nppHandle);
	if (hMenu)
	{
		CheckMenuItem(hMenu, funcItem[0]._cmdID, MF_BYCOMMAND | (enableDisablePlugin ? MF_CHECKED : MF_UNCHECKED));
	}
}

void toggleKeywords()
{
	enableDisableKeywords = !enableDisableKeywords;

	HMENU hMenu = GetMenu(nppData._nppHandle);
	if (hMenu)
	{
		CheckMenuItem(hMenu, funcItem[2]._cmdID, MF_BYCOMMAND | (enableDisableKeywords ? MF_CHECKED : MF_UNCHECKED));
	}
}

void toggleSections()
{
	enableDisableSections = !enableDisableSections;

	HMENU hMenu = GetMenu(nppData._nppHandle);
	if (hMenu)
	{
		CheckMenuItem(hMenu, funcItem[3]._cmdID, MF_BYCOMMAND | (enableDisableSections ? MF_CHECKED : MF_UNCHECKED));
	}
}

void toggleSql()
{
	enableDisableSql = !enableDisableSql;

	HMENU hMenu = GetMenu(nppData._nppHandle);
	if (hMenu)
	{
		CheckMenuItem(hMenu, funcItem[4]._cmdID, MF_BYCOMMAND | (enableDisableSql ? MF_CHECKED : MF_UNCHECKED));
	}
}

void baanCIndent(bool contentUpdated)
{
	// Get the current scintilla
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&which));
	if (which == -1) return;
	HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	const LRESULT position = ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
	const LRESULT line_number = ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, position, 0);
	const LRESULT previousIndent = ::SendMessage(curScintilla, SCI_GETLINEINDENTATION, line_number - 1, 0);
	const LRESULT tabSize = ::SendMessage(curScintilla, SCI_GETTABWIDTH, 0, 0);
	const LRESULT last_line_start = ::SendMessage(curScintilla, SCI_POSITIONFROMLINE, line_number - 1, 0);
	const LRESULT last_line_end = ::SendMessage(curScintilla, SCI_GETLINEENDPOSITION, line_number - 1, 0);
	const LRESULT last_line_length = last_line_end - last_line_start;

	auto last_line = std::make_unique<char[]>(last_line_length + 1);

	// Start of Undo Block
	::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);

	::SendMessage(curScintilla, SCI_SETSEL, last_line_start, last_line_end);
	::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(last_line.get()));

	/* Find the start of code. */
	int i;
	for (i = 0; i < last_line_length; i++)
	{
		if (last_line[i] != ' ' && last_line[i] != '\t')
		{
			break;
		}
	}

	if (i < last_line_length)
	{
		const LRESULT word_start = ::SendMessage(curScintilla, SCI_WORDSTARTPOSITION, last_line_start + i, true);
		const LRESULT word_end = ::SendMessage(curScintilla, SCI_WORDENDPOSITION, last_line_start + i, true);
		const LRESULT word_style = SendMessage(curScintilla, SCI_GETSTYLEAT, last_line_start + i, 0);

		::SendMessage(curScintilla, SCI_SETSEL, word_start, word_end);
		char buf[MAX_PATH];
		::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(buf));

		for (int j = 0; buf[j] != '\0'; j++)
			buf[j] = static_cast<char>(tolower(buf[j]));

		switch (word_style)
		{
			case SCE_BAAN_COMMENTDOC:
				if (enableDisableKeywords)
				{
					if (in_array(buf, cmntFwdIndent))
					{
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent + tabSize);
					}
					else if (contentUpdated && in_array(buf, cmntBckIndent))
					{
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number - 1, previousIndent - tabSize);
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent - tabSize);
					}
				}
				break;
			case SCE_BAAN_WORD:
				// Keywords
				if (enableDisableKeywords)
				{
					if (in_array(buf, keyFwdIndent))
					{
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent + tabSize);
					}
					else if (contentUpdated && in_array(buf, keyBckIndent))
					{
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number - 1, previousIndent - tabSize);
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent - tabSize);
					}
					else if (strcmp(buf, "case") == 0)
					{
						if (contentUpdated)
						{
							if (onCaseStarted)
							{
								onCaseStarted = false;
								::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent + tabSize);
							}
							else
							{
								::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number - 1, previousIndent - tabSize);
								::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent);
							}
						}
						else
						{
							if (onCaseStarted)
							{
								onCaseStarted = false;
								::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent + tabSize);
							}
							else
							{
								::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent + tabSize);
							}
						}
					}
					else if (strcmp(buf, "on") == 0)
					{
						onCaseStarted = true;
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent + tabSize);
					}
					else if (strcmp(buf, "endcase") == 0 && contentUpdated)
					{
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number - 1, previousIndent - (tabSize * 2));
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent - (tabSize * 2));
					}
					else if (strcmp(buf, "default:") == 0)
					{
						if (contentUpdated)
						{
							::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number - 1, previousIndent - tabSize);
							::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent);
						}
						else
						{
							::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, previousIndent + tabSize);
						}
					}
				}
				break;
			case SCE_BAAN_WORD4:
				//Sub Sections
				if (enableDisableSections)
				{
					if (mainSectionStarted) {
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, tabSize);
						mainSectionStarted = false;
					}
					else
					{
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number - 1, 0);
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, tabSize);
					}
				}
				break;
			case SCE_BAAN_WORD5:
				//Main Sections
				if (enableDisableSections)
				{
					::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number - 1, 0);
					if (in_array(buf, mainNoSubSec))
					{
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, tabSize);
					}
					else
					{
						::SendMessage(curScintilla, SCI_SETLINEINDENTATION, line_number, 0);
						mainSectionStarted = true;
					}
				}
				break;

		}
	}
	//::SendMessage(curScintilla, SCI_SETSEL, save_position, save_position);
	::SendMessage(curScintilla, SCI_SETSEL, position, position);

	LRESULT indentPos2 = ::SendMessage(curScintilla, SCI_GETLINEINDENTPOSITION, line_number, 0);
	::SendMessage(curScintilla, SCI_SETSEL, indentPos2, indentPos2);
	// End of Undo Block.
	::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
}

bool in_array(const std::string& value, const std::vector<std::string>& array)
{
	return std::find(array.begin(), array.end(), value) != array.end();
}
