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
#include "Starmap.h"

class CBotf2Doc;
class CShip;
class CSectorAI : public CObject
{
public:
	struct SectorToTerraform {
		BYTE pop;
		CPoint p;

		bool operator< (const SectorToTerraform& elem2) const { return pop < elem2.pop;}
		bool operator> (const SectorToTerraform& elem2) const { return pop > elem2.pop;}

		SectorToTerraform() {}
		SectorToTerraform(BYTE _pop, CPoint _p) : pop(_pop), p(_p) {}
	};

	/// Konstruktor
	CSectorAI(CBotf2Doc* pDoc);

	/// Destruktor
	~CSectorAI(void);

	/// Diese Funktion gibt das Gefahrenpotenzial in einem Sektor <code>sector</code> durch eine bestimmte
	/// Rasse <code>race</code> zur�ck.
	UINT GetDanger(BYTE race, CPoint sector) const {ASSERT(race); return m_iDangers[race-1][sector.x][sector.y];}

	/// Diese Funktion gibt das Gefahrenpotenzial in einem Sektor <code>sector</code> durch eine bestimmte
	/// Rasse <code>race</code> zur�ck. Dabei werden aber nur Kriegsschiffe beachtet.
	UINT GetDangerOnlyFromCombatShips(BYTE race, CPoint sector) const {ASSERT(race); return m_iCombatShipDangers[race-1][sector.x][sector.y];}

	/// Diese Funktion gibt das gesamte Gefahrenpotenzial aller Rassen in einem Sektor <code>sector</code> zur�ck.
	/// Das Gefahrenpotential der eigenen Rasse <code>ownRace</code> wird dabei aber nicht mit eingerechnet.
	UINT GetCompleteDanger(BYTE ownRace, CPoint sector) const;

	/// Diese Funktion gibt das gesamte Gefahrenpotenzial durch Kriegsschiffe einer Rasse <code>race</code> aus
	/// allen Sektoren zur�ck.
	UINT GetCompleteDanger(BYTE race) const {ASSERT(race); return m_iCompleteDanger[race-1];}

	/// Funktion gibt die Sektorkoordinate zur�ck, in welchem eine Rasse <code>race</code> die st�rkste Ansammlung
	/// von Kriegsschiffen hat.
	CPoint GetHighestShipDanger(BYTE race) const {ASSERT(race); return m_HighestShipDanger[race-1];}

	/// Funktion gibt die Sektorkoordinate zur�ck, in welchem eine Rasse <code>race</code> einen Au�enposten
	/// bauen sollte.
	BaseSector GetStationBuildSector(BYTE race) const {ASSERT(race); return m_StationBuild[race-1];}

	/// Diese Funktion gibt einen Zeiger auf das geordnete Feld der zum Terraforming am geeignetesten Systeme zur�ck.
	/// Daf�r muss nur die jeweilige Rasse <code>race</code> �bergeben werden.
	CArray<SectorToTerraform>* GetSectorsToTerraform(BYTE race) {ASSERT(race); return &m_SectorsToTerraform[race-1];}

	/// Diese Funktion gibt einen Zeiger auf das Feld mit den Sektoren der unbekannten Minors zur�ck. �bergeben muss
	/// daf�r nur die jeweilige Rasse <code>race</code> werden.
	CArray<CPoint>* GetMinorraceSectors(BYTE race) {ASSERT(race); return &m_MinorraceSectors[race-1];}

	/// Diese Funktion gibt einen Zeiger auf das Feld mit den Sektoren f�r m�gliche Offensivziele zur�ck. �bergeben muss
	/// daf�r nur die jeweilige Rasse <code>race</code> werden.
	CArray<CPoint>* GetOffensiveTargets(BYTE race) {ASSERT(race); return &m_OffensiveTargets[race-1];}

	/// Diese Funktion gibt einen Zeiger auf das Feld mit den Sektoren f�r m�gliche Bombardierungsziele zur�ck.
	/// �bergeben muss daf�r nur die jeweilige Rasse <code>race</code> werden.
	CArray<CPoint>* GetBombardTargets(BYTE race) {ASSERT(race); return &m_BombardTargets[race-1];}

	/// Funktion gibt die Anzahl an Kolonieschiffen der Rasse <code>race</code> zur�ck.
	short GetNumberOfColoships(BYTE race) const {ASSERT(race); return m_iColoShips[race-1];}

	/// Funktion gibt die Anzahl an Tranportschiffen der Rasse <code>race</code> zur�ck.
	short GetNumberOfTransportShips(BYTE race) const {ASSERT(race); return m_iTransportShips[race-1];}

	/// Diese Funktion tr�gt die ganzen Gefahrenpotenziale der Schiffe und Stationen in die Variable
	/// <code>m_iDangers</code> ein. Au�erdem wird hier auch gleich die Anzahl der verschiedenen Schiffe
	/// der einzelnen Rassen ermittelt.
	void CalculateDangers();

	/// Diese Funktion berechnet die ganzen Wichtigkeiten f�r die einzelnen Sektoren. Also wo lohnt es sich zum
	/// Terraformen, wo lernt man neue Minorraces kennen usw. Vorher sollte die Funktion <code>CalculateDangers()
	/// </code> aufgerufen werden.
	void CalcualteSectorPriorities();

private:
	/// Diese Funktion addiert die Offensiv- und Defensivst�rke eines Schiffes einer Rasse zum jeweiligen
	/// Sektor.
	void AddDanger(CShip* ship);

	/// Diese Funktion ermittelt die Sektoren, welche sich am ehesten zum Terraformen f�r eine bestimmte Rasse eignen.
	/// Die Eintr�ge werden dann im Array <code>m_SectorsToTerraform</code> gemacht.
	void CalculateTerraformSectors(int x, int y);

	/// Funktion berechnet die Sektoren, in denen eine einem Imperium unbekannte Minorrace lebt, zu deren Sektor
	/// aber theoretisch geflogen werden kann. Das Ergebnis wird im Array <code>m_MinorraceSectors</code> gespeichert.
	void CalculateMinorraceSectors(int x, int y);

	/// Diese Funktion berechnet alle m�glichen offensiven Ziele f�r eine bestimmte Rasse. Das Ergebnis wird im Array
	/// <code>m_OffensiveTargets</code> gespeichert.
	void CalculateOffensiveTargets(int x, int y);

	/// Diese Funktion berechnet Systeme, welche im Kriegsfall wom�glich angegriffen werden k�nnen. Das Ergebnis wird
	/// im Array <code>m_BombardTargets</code> gespeichert.
	void CalculateBombardTargets(BYTE race, int x, int y);

	/// Diese Funktion berechnet einen Sektor, welcher sich zum Bau eines Au�enpostens eignet.
	void CalculateStationTargets(BYTE race);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Variablen
	
	/// Ein Zeiger auf das Document.
	CBotf2Doc* m_pDoc;

	/// In dieser Variable stehen die addierten Offensiv- und Defensivwerte der einzelnen Rassen. Diese
	/// Werte spiegeln im Prinzip die St�rke der Schiffe wieder.
	UINT m_iDangers[UNKNOWN][STARMAP_SECTORS_HCOUNT][STARMAP_SECTORS_VCOUNT];

	/// In dieser Variable stehen die addierten Offensiv- und Defensivwerte der einzelnen Rassen.
	/// Doch befinden sich in dieser Variable nur die St�rken von Kriegsschiffen.
	UINT m_iCombatShipDangers[UNKNOWN][STARMAP_SECTORS_HCOUNT][STARMAP_SECTORS_VCOUNT];

	/// Variable beinhaltet die komplette St�rke aller Kriegsschiffe einer Rasse.
	UINT m_iCompleteDanger[DOMINION];

	/// Variable beinhaltet den Sektor, in dem die jeweilige Rasse die st�rkste Schiffsansammlung hat. Stationen
	/// werden dabei nicht mit beachtet.
	CPoint m_HighestShipDanger[DOMINION];

	/// Varibale beinhaltet den Sektor, in welchem die jewelige Rasse einen Au�enposten bauen sollte.
	BaseSector m_StationBuild[DOMINION];

	/// In diesem Feld werden die Sektoren f�r die jeweilige Rasse gespeichert, welche sich am meisten zum
	/// Terraformen lohnen.
	CArray<SectorToTerraform> m_SectorsToTerraform[DOMINION];

	/// In diesem Feld werden die Sektoren von Minorraces gespeichert, die die entsprechende Rasse noch nicht
	/// kennt, zu denen sie aber fliegen kann, um sie kennenzulernen.
	CArray<CPoint> m_MinorraceSectors[DOMINION];

	/// In diesem Feld stehen m�gliche Offensivziele einer bestimmten Rasse drin. Dabei kann es sich um eine
	/// gegnerische Flotte aber auch um einen Aussenposten handeln.
	CArray<CPoint> m_OffensiveTargets[DOMINION];

	/// In diesem Feld stehen m�gliche Systeme, welche im Kriegsfall bombardiert werden k�nnten.
	CArray<CPoint> m_BombardTargets[DOMINION];

	/// Variable beinhaltet die Anzahl an Kolonieschiffen einer bestimmten Rasse
	short m_iColoShips[DOMINION];

	/// Variable beinhaltet die Anzahl an Truppentransportern einer bestimmten Rasse
	short m_iTransportShips[DOMINION];
};
