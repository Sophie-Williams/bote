/*
 *   Copyright (C)2004-2009 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once
#include "Options.h"
#include <map>

using namespace std;

class CBotf2Doc;
class CSectorAI;
class CShipAI
{
public:
	/// Konstruktor
	CShipAI(CBotf2Doc* pDoc);
	
	/// Destruktor
	~CShipAI(void);

	/// Diese Funktion erteilt allen Schiffen aller computergesteuerten Rassen Befehle.
	void CalculateShipOrders(CSectorAI* SectorAI);

private:
	/// Funktion erteilt einen Terraformbefehl, sofern dies auch m�glich ist. �bergebn wird daf�r der index
	/// <code>index</code> des Schiffes aus dem Schiffsarray des Documents. Der R�ckgabewert der Funktion ist
	/// <code>TRUE</code>, wenn ein Terraformbefehl gegeben werden k�nnte.
	BOOLEAN DoTerraform(int index);

	/// Funktion erteilt einen Kolonisierungsbefehl, sofern dies auch m�glich ist. �bergeben wird daf�r der index
	/// <code>index</code> des Schiffes aus dem Schiffsarray des Dokuments. Der R�ckgabewert der Funktion ist
	/// <code>TRUE</code>, wenn ein Kolonisierungsbefehl gegeben werden k�nnte.
	BOOLEAN DoColonize(int index);

	/// Funktion schickt Kriegsschiffe zu einem m�glichen Offensivziel. Unter Umst�nden wird auch der Befehl zum
	/// Angriff automatisch erteilt. Auch kann es passieren, das diese Funktion die Kriegsschiffe zu einem
	/// gemeinsamen Sammelpunkt schickt. �bergebn wird daf�r der index <code>index</code> des Schiffes aus dem
	/// Schiffsarray des Documents. Der R�ckgabewert der Funktion ist <code>TRUE</code>, wenn ein Bewegungsbefehl
	/// gegeben werden k�nnte.
	BOOLEAN DoAttackMove(int index);

	/// Funktion schickt Kriegsschiffe zu einem m�glichen System, welches Bombardiert werden k�nnte. �bergeben wird
	/// daf�r der index <code>index</code> des Schiffes aus dem Schiffsarray des Documents. Der R�ckgabewert der
	/// Funktion ist <code>TRUE</code>, wenn ein Bewegungsbefehl gegeben werden konnte oder eine Bombardierung
	/// befohlen wurde oder das Schiff auf Verst�rkung zur Bombardierung im System wartet.
	BOOLEAN DoBombardSystem(int index);

	/// Funktion erteilt einen Tarnbefehl, wenn <code>camouflage</code> wahr ist. Ist <code>camouflage</code> falsch,
	/// wird ein Enttarnbefehl gegeben. �bergebn wird daf�r der index <code>index</code> des Schiffes aus dem
	/// Schiffsarray des Documents. Der R�ckgabewert der Funktion ist <code>TRUE</code>, wenn der Befehl gegeben wurde.
	BOOLEAN DoCamouflage(int index, BOOLEAN camouflage = TRUE);

	/// Funktion erteilt einen Au�enpostenbaubefehl, sofern dies auch m�glich ist. �bergeben wird daf�r der index
	/// <code>index</code> des Schiffes aus dem Schiffsarray des Dokuments. Der R�ckgabewert der Funktion ist
	/// <code>TRUE</code>, wenn ein Au�enpostenbaubefehl gegeben werden k�nnte.
	BOOLEAN DoStationBuild(int index);

	/// Funktion erstellt eine Flotte. Schiffe werden der Flotte nur hinzugef�gt, wenn diese bestimmte Voraussetzungen erf�llen.
	/// So muss der ungef�hre Schiffstyp �bereinstimmen (Combat <-> NonCombat) sowie die Geschwindigkeit des Schiffes.
	/// @param index Index des aktuellen Schiffes im Array.
	void DoMakeFleet(int index);
	
	/// Funkion berechnet einen m�glichen Angriffssektor, welcher sp�ter gesammelt angegriffen werden kann.
	void CalcAttackSector(void);

	/// Funktion berechnet einen m�glich besten Sektor f�r eine Bombardierung. Wurde solch ein Sektor ermittelt hat dieser
	/// die allerh�chste Priorit�t.
	void CalcBombardSector(void);

	/// Ein Zeiger auf das Document.
	CBotf2Doc* m_pDoc;

	/// Ein Zeiger auf das SectorAI-Objekt
	CSectorAI* m_pSectorAI;

	map<CString, CPoint> m_AttackSector;	///< der globale Angriffssektor der einzelnen Rassen

	map<CString, CPoint> m_BombardSector;	///< der globale Bombardierungssektor der einzelnen Rassen

	map<CString, int> m_iAverageMoral;		///< die durchschnittliche Moral in allen Systemen der einzelnen Rassen
};
