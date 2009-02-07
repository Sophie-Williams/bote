/*
 *   Copyright (C)2004-2008 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once
#include "afx.h"
#include "Options.h"

/**
 * Diese Klasse beinhaltet die genauen Zuweisungen auf die einzelnen Geheimdienstressorts und stellt Methoden
 * zur Manipulation und f�r den Zugriff dieser Informationen bereit.
 *
 * @author Sir Pustekuchen
 * @version 0.1
 */

class CMajorRace;
class CIntelAssignment : public CObject
{
	// ein CIntelligence Objekt bekommt vollen Zugriff auf die Attribute eines CIntelAssignment Objekts
	friend class CIntelligence;
public:
	DECLARE_SERIAL (CIntelAssignment)

	/// Konstruktor
	CIntelAssignment(void);
	
	/// Destruktor
	~CIntelAssignment(void);

	/// Serialisierungsfunktion
	void Serialize(CArchive &ar);

	// Zugriffsfunktionen
	/// Funktion gibt die globale prozentuale Zuteilung auf die innere Sicherheit zur�ck.
	BYTE GetInnerSecurityPercentage() const;

	/// Funktion gibt die globale prozentuale Zuteilung der Spionageabteilung zur�ck.
	/// @param race Rasse
	BYTE GetGlobalSpyPercentage(BYTE race) const {ASSERT(race); return m_byPercentage[race-1][0];}

	/// Funktion gibt die globale prozentuale Zuteilung der Sabotageabteilung zur�ck.
	BYTE GetGlobalSabotagePercentage(BYTE race) const {ASSERT(race); return m_byPercentage[race-1][1];}

	/// Funktion gibt die einzelnen Spionagezuteilungen f�r eine bestimmte Rasse zur�ck.
	/// @param race Rasse
	/// @param type Wirtschaft == 0, Wissenschaft == 1, Milit�r == 2, Diplomatie == 3, Depot == 4
	BYTE GetSpyPercentages(BYTE race, BYTE type) const;

	/// Funktion gibt die einzelnen Sabotagezuteilungen f�r eine bestimmte Rasse zur�ck.
	/// @param race Rasse
	/// @param type Wirtschaft == 0, Wissenschaft == 1, Milit�r == 2, Diplomatie == 3, Depot == 4
	BYTE GetSabotagePercentages(BYTE race, BYTE type) const;

	/// Funktion gibt das Feld der globalen Zuteilung zur�ck.
	/// @param race Rasse
	const BYTE* GetGlobalPercentage(BYTE race) const {ASSERT(race); return m_byPercentage[race-1];}

	/// Funktion �ndert die globale prozentuale Zuteilung der einzelnen Geheimdienstressorts. Dabei wird wenn
	/// n�tig die Zuteilung der anderen Ressorts ver�ndert.
	/// @param type Ressort (0 == Spionage, 1 == Sabotage, 2 == innere Sicherheit)
	/// @param perc neue prozentuale Zuteilung
	/// @param major unsere Majorrace
	/// @param race Rasse, welche von der �nderung betroffen sein soll
	/// @param ownRace eigene Rasse
	void SetGlobalPercentage(BYTE type, BYTE perc, CMajorRace* major, BYTE race, BYTE ownRace);

	/// Funktion �ndert die genauen Zuteilungen bei der Spionage. Also wieviel Prozent der Spionageabteilung
	/// gehen z.B. in die Wirtschaftsspionage. Nicht vergebene Prozentpunkte befinden sich automatisch im
	/// Spionagedepot f�r die jeweilige Rasse.
	/// @param type Wirtschaft == 0, Wissenschaft == 1, Milit�r == 2, Diplomatie == 3, Spionagedepot == 4
	/// @param perc neue prozentuale Zuteilung
	/// @param race Rasse, welche von der �nderung betroffen sein soll
	void SetSpyPercentage(BYTE type, BYTE perc, BYTE race);

	/// Funktion �ndert die genauen Zuteilungen bei der Sabotage. Also wieviel Prozent der Sabotageabteilung
	/// gehen z.B. in die Wirtschaftssabotage. Nicht vergebene Prozentpunkte befinden sich automatisch im
	/// Sabotagedepot f�r die jeweilige Rasse.
	/// @param type Wirtschaft == 0, Wissenschaft == 1, Milit�r == 2, Diplomatie == 3, Sabotagedepot == 4
	/// @param perc neue prozentuale Zuteilung
	/// @param race Rasse, welche von der �nderung betroffen sein soll
	void SetSabotagePercentage(BYTE type, BYTE perc, BYTE race);

	/// Resetfunktion f�r das CIntelAssignment-Objekt.
	void Reset();

private:
	// Attribute
	BYTE m_byPercentage[DOMINION][2];	///< prozentuale Zuteilung zwischen Spionage und Sabotage

	BYTE m_bySpyPercentage[DOMINION][4];///< prozentuale Zuteilung bei den einzelnen Spionagefeldern

	BYTE m_bySabPercentage[DOMINION][4];///< prozentuale Zuteilung bei den einzelnen Sabotagefeldern
};
