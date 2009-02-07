/*
 *   Copyright (C)2004-2008 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once
#include "afx.h"
#include "CombatShip.h"

class CMajorRace;
class CCombat :	public CObject
{
public:
	/// Konstruktor
	CCombat(void);
	
	/// Destruktor
	~CCombat(void);

	// Zugriffsfunktionen
	BOOLEAN GetInvolvedRace(BYTE race) const {ASSERT(race >= HUMAN && race <= DOMINION); return m_bInvolvedRaces[race];}

	BOOLEAN GetReadyForCombat() const {return m_bReady;}

	/// Schiffskampfberechnungsfunktion
	/**
	* Diese Funktion verlangt beim Aufruf einen Zeiger auf ein Feld, welches Zeiger auf Schiffe beinhaltet
	* <code>ships<code>. Diese Schiffe werden dann am Kampf teilnehmen. Kommt es zu einem Kampf, so mu� 
	* diese Funktion zu allererst aufgerufen werden.
	*/
	void SetInvolvedShips(CArray<CShip*,CShip*>* ships, CMajorRace* majors);
	
	/**
	* Diese Funktion setzt die gew�hlte Schiffsformation der Rasse <code>race<code> fest.
	* Sie sollte vor dem Aufruf der Funktion <code>PreCombatCalculation()<code> aufgerufen werden.
	*/
	void ApplyShipFormation(int race);
	
	/**
	* Diese Funktion setzt die gew�hlte Taktik <code>tactic<code> der Rasse <code>race<code> fest.
	* Sie sollte vor dem Aufruf der Funktion <code>PreCombatCalculation()<code> aufgerufen werden.
	*/
	void ApplyShipTactic(int race, BYTE tactic);
	
	/**
	* Diese Funktion mu� vor der Funktion <code>CalculateCombat()<code> aufgerufen werden. Sie stellt alle
	* Berechnungen an, welche f�r den sp�teren Kampfverlauf n�tig sind. Wird diese Funktion nicht ordnungsgem��
	* durchgef�hrt, kann die Funktion <code>CalculateCombat()<code> nicht durchgef�hrt werden.
	*/
	void PreCombatCalculation();
	
	/** 
	* Diese Funktion ist das Herzst�ck der CCombat-Klasse. Sie f�hrt die ganzen Kampfberechnungen durch.
	*/
	void CalculateCombat(BYTE winner[7]);

	/**
	* Diese Funktion setzt alle Variablen des Combat-Objektes wieder auf ihre Ausgangswerte
	*/
	void Reset();

//public:
private:
	/// Das dynamische Feld in denen alle am Kampf beteiligten Schiffe mit allen
	/// dazugh�rigen Informationen abegelegt sind
	CArray<CCombatShip*, CCombatShip*> m_InvolvedShips;

	/// Das Feld mit den Referenzen auf die Schiffe, welche im Kampf beteiligt sind. Dieses Feld wird zur Iteration
	/// verwendet und auch manipuliert.
	CArray<CCombatShip*, CCombatShip*> m_CS;

	/// In diesem Array werdem alle Gegner des jeweiligen Imperiums initialisiert.
	CArray<CCombatShip*, CCombatShip*> m_Enemies[7];

	/// Das dynamische Feld in denen alle am Kampf abgefeuerten und noch vorhandenen Torpedos
	/// mit allen dazugh�rigen Informationen abegelegt sind
	CombatTorpedos m_CT;
	
	/// Die von den beteiligten Rassen gew�hlten Formationen
	BYTE m_iFormation[7];
	
	/// Die von den beteiligten Rassen gew�hlten Taktiken
	BYTE m_iTactic[7];

	/// Sind alle Vorbereitungen f�r eine Kampfberechnungen abgeschlossen
	BOOLEAN m_bReady;

	/// Diese Variable beinhaltet die aktuelle Zeit (Runde) im Kampf
	UINT m_iTime;

	/// Speichert ob in diesem Kampf irgendwer irgendwen attackiert hat.
	BOOLEAN m_bAttackedSomebody;

	/// Speichert die Nummer der beteiligten Rassen.
	BOOLEAN m_bInvolvedRaces[7];

	/// Speichert des Feld der Hauptrassen im Spiel.
	CMajorRace *m_MajorRaces;

	/**
	* Diese Funktion versucht dem i-ten Schiff im Feld <code>m_CS<code> ein Ziel zu geben. Wird dem Schiff ein Ziel
	* zugewiesen gibt die Funktion TRUE zur�ck, findet sich kein Ziel mehr gibt die Funktion FALSE zur�ck.
	*/	
	BOOLEAN SetTarget(int i);

	/**
	* Diese private Funktion �berpr�ft, ob das Schiff an der Stelle <code>i<code> im Feld <code>m_CS<code> noch am
	* Leben ist, also ob es noch eine positive H�lle besitzt. Falls dies nicht der Fall sein sollte, dann
	* unternimmt diese Funktion alle Arbeiten die anfallen, um dieses Schiff aus dem Feld zu entfernen.
	* D.h. m�gliche Ziele werden ver�ndert, Zeiger neu zugeweisen usw. Wenn das Schiff zerst�rt ist gibt diese
	* Funktion FALSE zur�ck, ansonsten TRUE.
	*/
	BOOLEAN CheckShipLife(int i);

	/**
	* Funktion �berpr�ft, ob die Rassen in einem Kampf sich gegeneinander aus diplomatischen Gr�nden
	* �berhaupt attackieren. Die Funktion gibt <code>TRUE</code> zur�ck, wenn sie sich angreifen k�nnen,
	* ansonsten gibt sie <code>FALSE</code> zur�ck.
	*/
	BOOLEAN CheckDiplomacyStatus(const CMajorRace* raceA, const CMajorRace* raceB);
};
