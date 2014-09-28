/*
 *   Copyright (C)2004-2013 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de
 *
 */
//
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct RangeMap
{
	RangeMap() :
		range(NULL),
		w(0), h(0),
		x0(0), y0(0)
	{
	}

	unsigned char *range;		///< Array, das die Reichweitenmatrix zeilenweise enth�lt
	int w;						///< Anzahl der Spalten der Matrix
	int h;						///< Anzahl Zeilen der Matrix

	/// nullbasierter Index einer Spalte; die Matrix wird so ausgerichtet, dass diese Spalte �ber dem Sektor eines Au�enpostens steht
	int x0;
	/// nullbasierter Index einer Zeile; die Matrix wird so ausgerichtet, dass diese Zeile �ber dem Sektor eines Au�enpostens steht
	int y0;
};

class CRangeMaps
{

public:

	enum EXPANSION_SPEED
	{
		SLOW,
		CLASSIC,
		MEDIUM,
		FAST
	};

	/// Diese Funktion berechnet die Reichweitenkarte anhand der aktuellen Techstufe <code>propTech</code> und schreibt
	/// das Ergebnis in den Parameter <code>rangeMap</code>. Zus�tzlich werden Referenzen auf paar Hilfsvariablen
	/// �bergeben.
	static void CalcRangeMap(RangeMap& range_map, BYTE propTech, EXPANSION_SPEED speed);

private:

	/**
	 * Setzt die f�r nachfolgend hinzugef�gte Au�enposten zu verwendende RangeMap.
	 */
	static void SetRangeMap(RangeMap& range_map, unsigned char tmpRangeMap[], char w, char h, char x0, char y0);

};

