/*
 *   Copyright (C)2004-2008 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once

#include "Options.h"

class CFontLoader
{
public:
	/**
	 * Diese Funktion generiert die passende Schriftart. Daf�r wird die jeweilige Rasse im Parameter <code>playersRace
	 * </code>, die allgemeine Schriftgr��e (0: sehr klein, 1: klein, 2: normal, 3: gro�, 4: sehr gro�, 5: kolossal)
	 * mittels <code>size</code> und ein Zeiger auf ein Fontobjekt <code>font</code>, in welches die generierte Schrift
	 * gespeichert wird, �bergeben.
	 */
	static void CreateFont(BYTE playersRace, BYTE size, CFont* font);

	/**
	 * Diese Funktion generiert die passende Schriftart. Daf�r wird die jeweilige Rasse im Parameter <code>playersRace
	 * </code>, die allgemeine Schriftgr��e (0: sehr klein, 1: klein, 2: normal, 3: gro�, 4: sehr gro�, 5: kolossal)
	 * mittels <code>size</code>, die gew�nschte Schriftfarbe mittels <code>color</code> und ein Zeiger auf ein
	 * Fontobjekt <code>font</code>, in welches die generierte Schrift gespeichert wird, �bergeben. Der R�ckgabewert
	 * der Funktion ist die passende Schriftfarbe.
	 */
	static COLORREF CreateFont(BYTE playersRace, BYTE size, BYTE color, CFont* font);

	/**
	 * Diese Funktion gibt eine bestimmte Farbe f�r die Schrift einer Rasse zur�ck. Daf�r wird die jeweilige Rasse im
	 * Parameter <code>playersRace</code> und die gew�nschte Schriftfarbe im Paramter <code>color</code> �bergeben.
	 * Jede Rasse hat f�nfs verschiedene Farben f�r Schriften.
	 */
	static COLORREF GetFontColor(BYTE playersRace, BYTE color) {return m_Colors[playersRace-1][color];}

	static void CreateGDIFont(BYTE playersRace, BYTE size, CString &family, Gdiplus::REAL &fontSize);

	static void CFontLoader::GetGDIFontColor(BYTE playersRace, BYTE colorType, Gdiplus::Color &color);

private:
	/// Konstruktor
	CFontLoader(void);

	/// Destruktor
	~CFontLoader(void);

	/// Diese statische Variable beinhaltet die verschiedenen Schriftgr��en f�r die einzelnen Rassen.
	static BYTE m_byFontSizes[DOMINION][6];

	/// Diese statische Variable beinhaltet die verschiedenen Schriftgr��en f�r die einzelnen Rassen bei GDIPlus.
	static BYTE m_byGDIPlusFontSizes[DOMINION][6];

	/// Diese statische Variable beinhaltet die verschiedenen Schriftfarben der einzelnen Rassen.
	static COLORREF m_Colors[DOMINION][5];
};
