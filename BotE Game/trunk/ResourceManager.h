/*
 *   Copyright (C)2004-2009 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once
#include "afx.h"

class CResourceManager : public CObject
{
public:
	/// Kostruktor
	CResourceManager(void);

	/// Destruktor
	~CResourceManager(void);

	/// Funktion initiert die Klasse CResourceManager. Sie muss zu Beginn aufgerufen werden
	static void Init(void);

	/// Funktion gibt einen String zur�ck, der in einer StringTable steht.
	static CString GetString(CString key, BOOLEAN forceBigStarting = FALSE, CString subString1 = "", CString subString2 = "");

	/*
	/// Funktion gibt einen String aus der StrinTable zur�ck. Dieser String kann �hnlich der printf Funktion
	/// auch gleich formatiert werden.
	/// @param lpszKeyFormat Schl�ssel
	/// @param ... dynamische Anzahl mit einzuf�genden Strings
	/// @return Wert aus StringTable
	static CString Text(LPCTSTR lpszKeyFormat, ...);
	*/

private:
	static CMapStringToString m_StringTable;
};
