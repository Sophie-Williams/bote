/*
 *   Copyright (C)2004-2009 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once

class CIniLoader
{
public:
	/// Konstruktor
	CIniLoader(void);

	/// Destruktor
	virtual ~CIniLoader(void);

	/**
	 * Diese Funktion gibt den zum <code>key</code> geh�renden Eintrag aus der ini-Datei zur�ck.
	 */
	int GetValue(CString key);

	/**
	 * Diese Funktion gibt den zum <code>key</code> geh�renden Eintrag aus der ini-Datei zur�ck.
	 */
	float GetFloatValue(CString key);

	/**
	 * Diese Funktion gibt den zum <code>key</code> geh�renden Eintrag aus der ini-Datei zur�ck.
	 */
	CString GetStringValue(CString key);

	/**
	 * Diese Funktion schreibt den zum <code>key</code> geh�renden Eintrag <code>value</code> in die ini-Datei.
	 */
	BOOL WriteIniString(CString key, CString value, CString filename = "BotE.ini");
	
private:
	/**
	 * Diese Funktion liest das ini-File ein und speichert die Daten in der CStringToStringMap. Konnte die ini-
	 * Datei aus irgendeinem Grund nicht ge�ffnet werden liefert die Funktion <code>FALSE</code> zur�ck, ansonsten
	 * wird <code>TRUE</code> zur�ckgegeben.
	 */
	BOOL ReadIniFile(CString filename);
	
	/**
	 * Funktion erstellt die ini-Datei und schreibt einige Standartdaten in die Ini-Datei.
	 */
	void CreateIniFile(CString filename = "BotE.ini");

	/**
	 * Diese Funktion wandelt ein Wort, wie z.B. On oder True in einen Wahrheitswert um.
	 */
	int StringToInt(CString string);

	CMapStringToString m_Strings;		///< Speichert die eingelesenen Zeilen aus der ini Datei
};
