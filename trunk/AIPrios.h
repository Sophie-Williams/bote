/*
 *   Copyright (C)2004-2008 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once
#include "Options.h"
#include "IntelAI.h"

class CBotf2Doc;
class CSectorAI;
/**
 * Diese Klasse beinhaltet zus�tzliche Priorit�ten f�r den Schiffsbau und Truppenbau, sowie den Geheimdienst.
 * Diese Priorit�ten werden dann bei der Ausf�hrung der System-KI mitbeachtet.
 */
class CAIPrios
{
public:
	/// Konstruktor
	CAIPrios(CBotf2Doc* pDoc);

	/// Destruktor
	~CAIPrios(void);

	// Zugriffsfunktionen
	/// Funtkion gibt die Priorit�ten ein Kolonieschiff zu bauen zur�ck.
	BYTE GetColoShipPrio(BYTE race) const {ASSERT(race); return m_byColoShipPrio[race-1];}

	/// Funtkion gibt die Priorit�ten ein Kolonieschiff zu bauen zur�ck.
	BYTE GetTransportShipPrio(BYTE race) const {ASSERT(race); return m_byTransportPrio[race-1];}

	/// Funtkion gibt die Priorit�ten ein Kriegsschiff zu bauen zur�ck.
	BYTE GetCombatShipPrio(BYTE race) const {ASSERT(race); return m_byCombatShipPrio[race-1];}

	/// Funktion gibt einen Zeiger auf das Geheimdienstobjekt f�r eine bestimmte Rasse zur�ck.
	/// @param race Rasse
	CIntelAI* GetIntelAI(void) {return &m_IntelAI;}

	// sonstige Funktionen
	/// Funktion sollte aufgerufen werden, wenn diese Priorit�t gew�hlt wurde. Denn dann verringert sich
	/// diese Priorit�t.
	void ChoosedColoShipPrio(BYTE race) {ASSERT(race); m_byColoShipPrio[race-1] /= 2;}

	/// Funktion sollte aufgerufen werden, wenn diese Priorit�t gew�hlt wurde. Denn dann verringert sich
	/// diese Priorit�t.
	void ChoosedTransportShipPrio(BYTE race) {ASSERT(race); m_byTransportPrio[race-1] /= 2;}

	/// Funktion sollte aufgerufen werden, wenn diese Priorit�t gew�hlt wurde. Denn dann verringert sich
	/// diese Priorit�t.
	void ChoosedCombatShipPrio(BYTE race) {ASSERT(race); m_byCombatShipPrio[race-1] /= 2;}

	/// Diese Funktion berechnet die Priorit�ten der einzelnen Majorrassen, wann sie ein Kolonieschiff in Auftrag
	/// geben sollen.
	void CalcShipPrios(CSectorAI* sectorAI);

	/// Funktion l�scht alle vorher berechneten Priorit�ten.
	void Clear(void);

private:
	// Attribute
	/// Beinhaltet die Priorit�t einer Rasse ein Kolonieschiff zu bauen.
	BYTE m_byColoShipPrio[DOMINION];

	/// Beinhaltet die Priorit�t Truppentransporter zu bauen.
	BYTE m_byTransportPrio[DOMINION];

	/// Beinhaltet die Priorit�t Kriegschiffe zu bauen.
	BYTE m_byCombatShipPrio[DOMINION];

	/// Beinhaltet Priorit�ten f�r den Geheimdienst und stellt Funktionen zur Steuerung der KI bereit
	CIntelAI m_IntelAI;

	/// Ein Zeiger auf das Document.
	CBotf2Doc* m_pDoc;
};

/*
Schiffsangriff:

Wann wird ein Zielsektor ermittelt?
-----------------------------------
Genau dann, wenn wir angegriffen werden, einen Angriff vorbereiten wollen (noch kein Krieg) oder im Krieg mit einer Rasse
sind und sofort angreifen wollen.


Wie wird ein Zielsektor ermittelt?
----------------------------------
Wir m�ssen zwischen Offensive und Defensive unterscheiden. Wollen wir angreifen haben wir genug Zeit die Schiffe zu
sammeln. M�ssen wir uns verteidigen, so m�ssen wir auch sammeln, sollten aber auch angreifen, wenn unsere Flotte
insgesamt schw�cher ist. Einen Zielsektor in der Offensive sollte eine gegnerische Flotte oder ein gegnerischer
Aussenposten oder auch ein gegnerisches System sein.


Wie wird die Flotte gesammelt?
------------------------------
Zuerst wird der Sektor gesucht, in dem wir die meisten/st�rksten Schiffe stationiert haben. All diese Schiffe werden
zu einer Flotte zusammengefasst und fliegen zum Zielsektor. Doch sie fliegen noch nicht direkt zum Zielsektor, sondern
halten rand()%3+1 Felder davor an. Um dies rauszubekommen werden einfach die letzten rand()%3+1 Flugpfadeintr�ge der
Flotte entfernt. Dann wird gewartet, bis die anderen Schiffsflotten auf diesem Feld eintreffen. Ich schlage mal vor,
dass ca. 80% - 90% der Flotte auf diesem Feld eingetroffen sein muss oder die Schiffsst�rke in diesem Feld schon st�rker
als die Schiffsst�rke im Zielsektor ist. Trifft eine der beiden Bedingungen ein wird der Kurs zum Zielsektor gesetzt.

*/