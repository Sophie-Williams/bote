/*
 *   Copyright (C)2004-2013 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de
 *
 */
/*
 *@file
 * properties of a map location with a sun system and with buildings
 */

#if !defined(AFX_SYSTEM_H__52476A8D_3AFC_4EFC_B456_155002572D31__INCLUDED_)
#define AFX_SYSTEM_H__52476A8D_3AFC_4EFC_B456_155002572D31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Worker.h"
#include "AssemblyList.h"
#include "SystemProd.h"
#include "Trade/TradeRoute.h"
#include "General/GlobalTypes.h"
#include "Manager.h"
#include "Galaxy/Sector.h"

// forward declaration
class CMajor;
class CMinor;
class CBotEDoc;
class CPlanet;
class CGlobalBuildings;

class CSystem : public CSector
{
public:

//////////////////////////////////////////////////////////////////////
// construction/destruction
//////////////////////////////////////////////////////////////////////

	// Standardkonstruktor
	CSystem(void);
	CSystem(int x, int y);

	//Kopierkontruktor (weil vector und braucht das)
	CSystem(const CSystem& other);

	// Destruktor
	virtual ~CSystem(void);

	CSystem& operator=(const CSystem&);

	// Resetfunktion f�r die Klasse CSystem.
	void ResetSystem(bool call_up);


//////////////////////////////////////////////////////////////////////
// serialization
//////////////////////////////////////////////////////////////////////

	// Serialisierungsfunktion
	void Serialize(CArchive &ar);

//////////////////////////////////////////////////////////////////////
// proper getting
//////////////////////////////////////////////////////////////////////

// Zugriffsfunktionen
	// zum Lesen der Membervariablen
	// Funktion gibt den Besitzer des Systems zur�ck, sollte eigentlich immer auch der Besitzer des Sektors sein.
	const CString& GetOwnerOfSystem() const {return m_sOwnerOfSystem;}

	// Funktion gibt die aktuelle Bev�lkerung des Systems zur�ck.
	double GetHabitants() const {return m_dHabitants;}

	// Funktionen geben jeweils einen Zeiger auf das Feld mit den Informationen �ber baubare Geb�ude,
	// baubare Updates oder baubaren Schiffen in dem System zur�ck.
	const CArray<short,short>* GetBuildableBuildings() const {return &m_BuildableBuildings;}
	const CArray<short,short>* GetBuildableUpdates() const {return &m_BuildableUpdates;}
	const CArray<short,short>* GetBuildableShips() const {return &m_BuildableShips;}
	const CArray<BYTE,BYTE>*   GetBuildableTroops() const {return &m_BuildableTroops;}

	// Funktion gibt die Anzahl oder die RunningNumber (ID) der Geb�ude zur�ck, welche Arbeiter ben�tigen.
	// Wir �bergeben daf�r als Parameter den Typ des Geb�udes (FARM, BAUHOF usw.) und einen Modus.
	// Ist der Modus NULL, dann bekommen wir die Anzahl zur�ck, ist der Modus EINS, dann die RunningNumber.
	USHORT GetNumberOfWorkbuildings(WORKER::Typ nWorker, int Modus) const;

	// Funktion gibt die Anzahl des Geb�udes mit der �bergebenen RunningNumber zur�ck.
	USHORT GetNumberOfBuilding(USHORT runningNumber) const;

	// Funktion gibt die Anzahl der aktiven Arbeiter zur�ck, deren Typ als Parameter an die Funktion �bergeben wurde.
	USHORT GetWorker(WORKER::Typ nWorker) const {return m_Workers.GetWorker(nWorker);}

	// Funktion gibt die aktuelle Moral im System zur�ck
	short GetMoral() const {return m_iMoral;}

	/// Funktion gibt den Prozentwert der Blockade des Systems zur�ck. Ist dieser R�ckgabewert gleich
	/// <code>NULL</code>, so existiert keine Blockade im System.
	BYTE GetBlockade() const {return m_byBlockade;}

	/// Funktion gibt Array mit Informationen ob eine Produktion deaktiviert ist zur�ck.
	/// @return Zeiger auf Array mit Infos ob Produktion deaktiviert ist oder nicht
	const bool* GetDisabledProductions() const { return m_bDisabledProductions; }

	// Funktionen geben die einzelnen Lagerinhalte des Systems zur�ck.
	long GetFoodStore() const {return m_Store.Food;}
	UINT GetTitanStore() const {return m_Store.Titan;}
	UINT GetDeuteriumStore() const {return m_Store.Deuterium;}
	UINT GetDuraniumStore() const {return m_Store.Duranium;}
	UINT GetCrystalStore() const {return m_Store.Crystal;}
	UINT GetIridiumStore() const {return m_Store.Iridium;}
	UINT GetDeritiumStore() const {return m_Store.Deritium;}

	// Funktionen geben die jeweiligen maximalen Lagerinhalte zur�ck
	static long GetFoodStoreMax() {return MAX_FOOD_STORE;}
	static UINT GetTitanStoreMax() {return MAX_RES_STORE;}
	static UINT GetDeuteriumStoreMax() {return MAX_RES_STORE;}
	static UINT GetDuraniumStoreMax() {return MAX_RES_STORE;}
	static UINT GetCrystalStoreMax() {return MAX_RES_STORE;}
	static UINT GetIridiumStoreMax() {return MAX_RES_STORE;}
	UINT GetDeritiumStoreMax() const;
	int GetXStoreMax(RESOURCES::TYPE x) const;
	int GetXStoreMax(WORKER::Typ x) const;

	// Funktion gibt den Lagerinhalt der Ressource zur�ck, die an die Funktion �bergeben wurde.
	UINT GetResourceStore(USHORT res) const;
	int GetResourceStore(WORKER::Typ type) const;

	// Funktion gibt die Anzahl der Geb�ude mit der �bergebenen RunningNumber wieder, welche n�chste Runde
	// abgerissen werden sollen
	USHORT GetBuildingDestroy(int RunningNumber) const;

private:
	int GetXBuildings(WORKER::Typ x) const;
public:

//////////////////////////////////////////////////////////////////////
// questionable getting
//////////////////////////////////////////////////////////////////////

	// Funktion gibt einen Zeiger auf den Lagerinhalt der Ressource zur�ck, die an die Funktion �bergeben wurde.
	const int* GetResourceStorages(USHORT res) const;
	int* GetResourceStorages(USHORT res);

	// Funktion gibt einen Zeiger auf die Bauliste des Systems zur�ck.
	const CAssemblyList* GetAssemblyList() const {
		return &m_AssemblyList;
	}
	CAssemblyList* GetAssemblyList() {
		return &m_AssemblyList;
	}

	const CSystemManager& Manager() const { return m_Manager; }
	CSystemManager& Manager() { return m_Manager; }

	// Funktion gibt einen Zeiger auf alle Produktionswerte und manche Boni des Systems zur�ck
	const CSystemProd* GetProduction() const {return &m_Production;}
	CSystemProd* GetProduction() {return &m_Production;}

	// Funktion gibt einen Zeiger auf das Feld aller Geb�ude im System zur�ck.
	const BuildingArray* GetAllBuildings() const {
		return &m_Buildings;
	}
	BuildingArray* GetAllBuildings() {
		return &m_Buildings;
	}

	// Funktion gibt einen Zeiger auf das Arbeiterobjekt der Systemklasse zur�ck.
	const CWorker* GetWorker() const {return &m_Workers;}
	CWorker* GetWorker() {return &m_Workers;}

	/// Funktion gibt einen Zeiger auf die Handelsrouten von diesem System aus zur�ck
	const CArray<CTradeRoute>* GetTradeRoutes() const {return &m_TradeRoutes;}
	CArray<CTradeRoute>* GetTradeRoutes() {return &m_TradeRoutes;}

	/// Funktion gibt einen Zeiger auf die Ressourcenrouten, welche in dieses System f�hren zur�ck.
	const CArray<CResourceRoute>* GetResourceRoutes() const {return &m_ResourceRoutes;}
	CArray<CResourceRoute>* GetResourceRoutes() {return &m_ResourceRoutes;}

	/// Funktion gibt einen Zeiger auf die stationierten Truppen in diesem System zur�ck.
	const CArray<CTroop>* GetTroops() const {return &m_Troops;}
	CArray<CTroop>* GetTroops() {return &m_Troops;}


//////////////////////////////////////////////////////////////////////
// setting
//////////////////////////////////////////////////////////////////////

	// zum Schreiben der Membervariabblen
	// Funktion setzt den neuen Besitzer des Systems. �bergeben wird der Besitzer.
	void SetOwnerOfSystem(const CString& sOwnerOfSystem);

	// Funktion setzt die Bev�lkerungsanzahl des Systems. �bergeben wird die Bev�lkerung aller Planeten des Sektors.
	// Gleichzeitig �berpr�ft die Funktion auch, ob man eine weitere Handelsroute aufgrund der Bev�lkerung bekommt, dann
	// gibt die Funktion ein <code>TRUE</code> zur�ck, ansonsten <code>FALSE</code>.
	BOOLEAN SetHabitants(double habitants);

	// Funktion sagt ob ein bestimmtes Geb�ude in dem System baubar ist oder nicht. Als Parameter werden daf�r
	// die RunningNumber des gew�nschten Geb�udes und der Wert �bergeben.
	void SetBuildableBuildings(int RunningNumber, BOOLEAN TrueOrFalse);

	// Funktion sagt ob ein bestimmtes Geb�udeupdate in dem System baubar ist oder nicht. Als Parameter werden daf�r
	// die RunningNumber des gew�nschten Geb�udes und der Wert �bergeben.
	void SetBuildableUpdates(int RunningNumber, BOOLEAN TrueOrFalse);

	/// Funktion setzt das Geb�ude aus dem Feld aller Geb�ude im System <code>m_Buildings</code> mit dem Index
	/// <code>index</code> online oder offline.
	/// @param index Feldeintrag in <code>m_Buildings</code>
	/// @param newStatus neuer Status des Geb�udes - <code>1</code> f�r online, ansonsten <code>NULL</code>
	void SetIsBuildingOnline(int index, BOOLEAN newStatus);

	enum SetWorkerMode { SET_WORKER_MODE_INCREMENT, SET_WORKER_MODE_DECREMENT, SET_WORKER_MODE_SET };
	// Komplette Zugriffsfunktion f�r das Arbeiterobjekt.
	//Bei Modus 0 wird der "WhatWorker" inkrementiert, bei Modus 2 wird
	// er dekrementiert und bei Modus 2 wird der "WhatWorker" auf den Wert von Value gesetzt.
	void SetWorker(WORKER::Typ nWhatWorker, SetWorkerMode Modus, int Value = -1);

	// Funktion setzt alle vorhandenen Arbeiter soweit wie m�glich in Geb�ude, die Arbeiter ben�tigen.
	void SetWorkersIntoBuildings();

	// Funktion addiert moralAdd zu m_iMoral dazu und mach gleichzeitig noch die �berpr�fen auf den richtigen Bereich.
	void SetMoral(short moralAdd) {if ((m_iMoral+moralAdd) >= 0) m_iMoral += moralAdd; if (m_iMoral > 200) m_iMoral = 200;}

	/// Funktion setzt den Prozentwert der Blockade fest. Ist dieser Wert gr��er als <code>NULL</code>, so besteht
	/// eine Blockade.
	void SetBlockade(BYTE blockadeValue) {m_byBlockade = min(blockadeValue, 100);}

	/// Funktion deaktiviert eine bestimmte Produktion.
	/// @param nType Produktionstyp (siehe FOOD_WORKER, INDUSTRY_WORKER usw.)
	void SetDisabledProduction(int nType) { m_bDisabledProductions[nType] = true; }

	// Funktion setzt das Nahrungslager des Systems auf den Parameterwert "food".
	void SetFoodStore(ULONG food) {m_Store.Food = food;}

	// Funktionen addieren den �bergebenen Paramterwert auf das jeweilige Ressourcenlager.
	void SetTitanStore(int titanAdd) {m_Store.Titan += titanAdd;}
	void SetDeuteriumStore(int deuteriumAdd) {m_Store.Deuterium += deuteriumAdd;}
	void SetDuraniumStore(int duraniumAdd) {m_Store.Duranium += duraniumAdd;}
	void SetCrystalStore(int crystalAdd) {m_Store.Crystal += crystalAdd;}
	void SetIridiumStore(int iridiumAdd) {m_Store.Iridium += iridiumAdd;}
	void SetDeritiumStore(int deritiumAdd) {m_Store.Deritium += deritiumAdd;}
	void SetStores(const GameResources& add);

	// Funktion addiert resAdd zur Ressource res.
	void SetResourceStore(USHORT res, int resAdd);

	// Diese Funktion setzt die abzurei�enden Geb�ude fest. Die zu �bergebenden Parameter sind die RunningNumber
	// des Geb�udes und einen Wert 1 f�r hinzuf�gen und 0 f�r wieder aus der Liste nehmen
	void SetBuildingDestroy(int RunningNumber, BOOLEAN add);

	/// Funktion setzt fest, ob die Autobaufunktion in dem System aktiviert ist.
	/// param is <code>TRUE</code> wenn eingeschalten, ansonsten <code>FALSE</code>
	void SetAutoBuild(BOOLEAN is) {m_bAutoBuild = is;}

	void FreeAllWorkers();

	/// Alle deaktivierten Produktionen zur�cksetzen
	void ClearDisabledProductions();

//////////////////////////////////////////////////////////////////////
// bool info
//////////////////////////////////////////////////////////////////////

	bool HasWorkerBuilding(WORKER::Typ type) const;

	bool HasStore(WORKER::Typ type) const;

	/// Funktion gibt zur�ck, ob die Autobaufunktion in dem System aktiviert ist.
	BOOLEAN GetAutoBuild() const {return m_bAutoBuild;}

	bool CanTakeOnline(const CBuildingInfo& info) const;

	bool SanityCheckWorkers();

private:
	bool SanityCheckWorkersInRange(WORKER::Typ type) const;
public:

//////////////////////////////////////////////////////////////////////
// calculation of member variables
//////////////////////////////////////////////////////////////////////

	// Funktion berechnet aus den Eigenschaften der stehenden Geb�ude alle Attribute der Systemklasse.
	void CalculateVariables();

	// Funktion berechnet die Lagerinhalte des Systems. Aufrufen bei Ende bzw. Beginn einer neuen Runde.
	// Gibt die Funktion TRUE zur�ck hat sich das System Aufgrund zu schlechter Moral vom Besitzer losgesagt.
	BOOLEAN CalculateStorages(CResearchInfo* researchInfo, int diliAdd);

	// Funktion berechnet die baubaren Schiffe in dem System.
	void CalculateBuildableShips();

	// Funktion berechnet die Anzahl aller Farmen, Bauh�fe usw., also alle Geb�ude die Arbeiter ben�tigen.
	// Sie mu� am Rundenanfang vor CalculateVariables() aufgerufen werden und sortiert gleichzeitig das
	// CArray m_Buildings nach der RunningNumber. // In der Doc-Klasse nach der Funktion DestroyBuildings()
	// und zu Beginn aufrufen!
	void CalculateNumberOfWorkbuildings(const BuildingInfoArray *buildingInfos);

	// Funktion berechnet die imperiumweite Moralproduktion, welche aus diesem System generiert wird.
	void CalculateEmpireWideMoralProd(const BuildingInfoArray *buildingInfos);

//////////////////////////////////////////////////////////////////////
// buildings
//////////////////////////////////////////////////////////////////////

private:
	// private Hilfsfunktionen (mal schauen ob wir die direkt in die cpp-Datei schreiben k�nnen)
	BOOLEAN CheckGeneralConditions(const CBuildingInfo* building, CGlobalBuildings* globals, CMajor* pMajor);
	BOOLEAN CheckFollower(const BuildingInfoArray* buildings, USHORT ID, BOOLEAN flag = 0, BOOLEAN equivalence = 0);
public:

	// Funktion berechnet die baubaren Geb�ude und Geb�udeupdates in dem System.
	void CalculateBuildableBuildings(CGlobalBuildings* globals);

	// Funktion f�gt ein neues Geb�ude zum System hinzu.
	void AddNewBuilding(CBuilding &building) { m_Buildings.Add(building); }

	// Funktion l�scht alle Geb�ude, die die �bergebene RunningNumber haben und gibt deren Anzahl zur�ck.
	// -> Danach mu� AddBuilding() mit dem Nachfolger gleich der Anzahl aufgerufen werden.
	int UpdateBuildings(int nRunningNumber, int nNeededEnergy);

	void Colonize(const CShips& ship, CMajor& major);

	// Funktion berechnet und baut die Startgeb�ude in einem System, nachdem wir einen Planeten
	// in diesem kolonisiert haben.
	void BuildBuildingsAfterColonization(const BuildingInfoArray* buildingInfo, USHORT ColonizationPoints);

	// Funktion baut die Geb�ude der Minorrace, wenn wir eine Mitgliedschaft mit dieser erreicht haben.
	void BuildBuildingsForMinorRace(BuildingInfoArray* buildingInfo, USHORT averageTechlevel, const CMinor* pMinor);

	// Funktion rei�t alle Geb�ude ab, die in der Variable m_BuildingDestroy stehen. Funktion wird in der Doc
	// bei NextRound() aufgerufen.
// --- wird noch �berarbeitet
	bool DestroyBuildings(void);

	// Funktion setzt das letzte Geb�ude, welches gebaut wurde online, sofern dies m�glich ist.
	int SetNewBuildingOnline(const BuildingInfoArray *buildingInfos);

	// Funktion �berpr�ft Geb�ude die Energie ben�tigen und schaltet diese gegebenfalls ab,
	// falls zuwenig Energie im System vorhanden ist. Diese Funktion aufrufen, bevor wir CalculateVariables() usw.
	// aufrufen, weil wir ja die b�sen Onlinegeb�ude vorher ausschalten wollen.
	bool CheckEnergyBuildings();

	// Funktion f�gt einen Baulistencheck durch
	BOOLEAN AssemblyListCheck(const BuildingInfoArray* buildingInfo, CGlobalBuildings* globals);

	/// Diese Funktion entfernt alle speziellen Geb�ude aus der Geb�udeliste. Diese Funktion sollte nach Eroberung des Systems
	/// aufgerufen werden. Danach sind keine Geb�ude mehr vorhanden, die nur x mal pro Imperium baubar gewesen oder die nur die Rasse
	/// selbst bauen darf.
	/// @param pvBuildingInfos Zeiger auf den Vektor mit allen Geb�udeinformationen
	void RemoveSpecialRaceBuildings(const BuildingInfoArray* pvBuildingInfos);


//////////////////////////////////////////////////////////////////////
// calculate something and return it
//////////////////////////////////////////////////////////////////////

	// Funktion berechnet die vorraussichtlich ben�tigte Anzahl an Runden,
	// bis ein beliebiges Projekt in diesem System fertig sein wird.
	//@param index_or_id assembly list entry index if already_in_list,
	//the building ID otherwise
	// @return Anzahl der ben�tigten Runden
	int NeededRoundsToBuild(int index_or_id, bool already_in_list, bool use_potential = false);

	//Function calculates, starting from the "base IPs", the actual industry points a system
	//generates, considering "never ready" buildings, UpdateBuildSpeed-Boni, BuildingBuildSpeed-Boni,
	//ShipYardEfficiency, BarrackEfficiency
	int CalcIPProd(const CArray<CBuildingInfo, CBuildingInfo>& BuildingInfo, const int list) const;

//////////////////////////////////////////////////////////////////////
// trade routes
//////////////////////////////////////////////////////////////////////

	// Funktion �berpr�ft, ob wie aufgrund der Bev�lkerung hier im System �berhaupt (noch) eine Handelsroute
	// anlegen k�nnen
	BOOLEAN CanAddTradeRoute(CResearchInfo* researchInfo) const;

	/// Funktion generiert eine neue Handelsroute. Wenn die Funktion <code>TRUE</code> zur�ckgibt, dann konnte die
	/// Handelsroute erfolgreich angelegt werden. Als Parameter wird dabei die Koordinate <code>dest</code>des
	/// Zielsektors �bergeben sowie ein Zeiger auf alle Systeme <code>systems</code> auf der Map.
	BOOLEAN AddTradeRoute(CPoint dest, std::vector<CSystem>& systems, CResearchInfo* researchInfo);

	// Funktion gibt das gesamte Credits zur�ck, was alle Handelsrouten aus diesem System generiert haben.
	USHORT CreditsFromTradeRoutes() const;

	// Funktion �berpr�ft alle Handelsrouten in dem System, ob sie Aufgrund der Bev�lkerung oder der Geb�ude noch
	// da sein d�rfen. Wurden Handelsrouten gel�scht, so gibt die Funktion die Anzahl der L�schungen zur�ck
	BYTE CheckTradeRoutes(CResearchInfo* researchInfo);
	// Calculates diplomacy-related effects of and to this system's trade routes.
	// @return number of deleted trade routes
	unsigned CheckTradeRoutesDiplomacy(CBotEDoc& pDoc);

//////////////////////////////////////////////////////////////////////
// resource routes
//////////////////////////////////////////////////////////////////////

	/// Funktion generiert eine neue Ressourcenroute. Wenn die Funktion <code>TRUE</code> zur�ckgibt, dann konnte die
	/// Ressourcenroute erfolgreich angelegt werden. Als Parameter wird dabei die Koordinate <code>dest</code> des
	/// Zielsektors �bergeben sowie die Art der betroffenen Ressource <code>res</code> und einen Zeiger auf alle
	/// Systeme <code>systems</code> auf der Map.
	BOOLEAN AddResourceRoute(CPoint dest, BYTE res, const std::vector<CSystem>& systems, CResearchInfo* researchInfo);

	/// Funktion �berpr�ft alle Ressourcenrouten in dem System, ob sie Aufgrund der Bev�lkerung oder der Geb�ude noch
	/// da sein d�rfen. Wurden Ressourcenrouten gel�scht, so gibt die Funktion die Anzahl der L�schungen zur�ck.
	BYTE CheckResourceRoutes(CResearchInfo* researchInfo);
	// Checks this system's resource routes for whether the target system is still part of the empire this system
	// belongs to.
	// @return number of deleted resource routes
	unsigned CheckResourceRoutesExistence(CBotEDoc& pDoc);

//////////////////////////////////////////////////////////////////////
// troops
//////////////////////////////////////////////////////////////////////

	// Diese Funktion berechnet die baubaren Truppen in diesem System
	void CalculateBuildableTroops(const CArray<CTroopInfo>* troopInfos, const CResearch *research);

	/// Wenn in diesem System Truppen stationiert sind, dann wird deren Moralwert mit einbezogen.
	/// Ist die Moral im System unter 100, so wird der Moralwert der Einheit dazuaddiert,
	//wenn er �ber 100 ist, dann wird
	/// der Moralwert abgezogen.
	void IncludeTroopMoralValue(CArray<CTroopInfo>* troopInfo);

	/// Diese Funktion f�gt eine neue Truppe <code>troop</code> dem System hinzu.
	void AddTroop(const CTroop* troop) {m_Troops.Add(*troop);}

	void TrainTroops();

//////////////////////////////////////////////////////////////////////
// other functions
//////////////////////////////////////////////////////////////////////

	void ExecuteManager(CMajor& owner, bool turn_change, bool energy = true);

//////////////////////////////////////////////////////////////////////
// member variables
//////////////////////////////////////////////////////////////////////

private:

	// Der Besitzer des Systems
	CString m_sOwnerOfSystem;

	// Einwohner in dem System
	double m_dHabitants;

	// Die Bauliste des Systems
	CAssemblyList m_AssemblyList;

	// Die Produktion des Systems
	CSystemProd m_Production;

	// Alle Geb�ude in dem System
	BuildingArray m_Buildings;

	// Variable beinhaltet welche Geb�ude in dem System baubar sind
	CArray<short,short> m_BuildableBuildings;

	// Variable beinhaltet welche Geb�udeupdates in dem System baubar sind
	CArray<short,short> m_BuildableUpdates;

	// Variable beinhaltet welche Schiffe in dem System baubar sind
	CArray<short,short> m_BuildableShips;

	// Variable beinhaltet welche Truppen in dem Sytem baubar sind
	CArray<BYTE,BYTE> m_BuildableTroops;

	// Variable beinhaltet welche Geb�ude(-stufe) in dem System immer baubar ist
	CArray<short,short> m_AllwaysBuildableBuildings;

	// Variable beinhaltet welche Geb�ude und Updates wir in dem System bauen k�nnen, bevor der Baulistencheck
	// gemacht wurde. Diese Variable mu� von au�erhalb nie betrachtet werden
	CArray<short,short> m_BuildableWithoutAssemblylistCheck;

	// Die Arbeiter in dem System
	CWorker m_Workers;

	// Die Moral der Bev�lkerung auf in dem System
	short m_iMoral;

	/// Besteht in dem System eine feindliche Blockade, so gibt diese Variable den Prozentwert der Blockade an.
	/// Ist dieser gr��er <code>NULL</code>, so besteht eine Blockade.
	BYTE m_byBlockade;

	// Deaktivierte Produktion
	bool m_bDisabledProductions[WORKER::IRIDIUM_WORKER + 1];

	// Die Lagerinhalt der einzelnen Rohstoffe und der Nahrung
	GameResources m_Store; //store for resources food, titan, deuterium, duranium, crystal, iridium, deritium

	// Variable zum Abrei�en von Geb�uden
	CArray<USHORT,USHORT> m_BuildingDestroy;

	// Anzahl der Geb�ude des jeweiligen Types
	BYTE m_iFoodBuildings;				// Anzahl der Nahrungsgeb�ude in dem System
	BYTE m_iIndustryBuildings;			// Anzahl der Industriegeb�ude in dem System
	BYTE m_iEnergyBuildings;			// Anzahl der Energiegeb�ude in dem System
	BYTE m_iSecurityBuildings;			// Anzahl der Geheimdienstgeb�ude in dem System
	BYTE m_iResearchBuildings;			// Anzahl der Forschungsgeb�ude in dem System
	BYTE m_iTitanMines;					// Anzahl der Titanminen in dem System
	BYTE m_iDeuteriumMines;				// Anzahl der Deuteriumminen in dem System
	BYTE m_iDuraniumMines;				// Anzahl der Duraniumminen in dem System
	BYTE m_iCrystalMines;				// Anzahl der Crystalminen in dem System
	BYTE m_iIridiumMines;				// Anzahl der Iridiumminen in dem System

	CArray<CTradeRoute> m_TradeRoutes;			///< Die Handelsrouten in dem System

	CArray<CResourceRoute> m_ResourceRoutes;	///< Die Ressourcenrouten in dem System

	// Die maximalen Handelsrouten allein durch die Bev�lkerung
	BYTE m_byMaxTradeRoutesFromHab;

	// Die vorhanden Truppen in dem System
	CArray<CTroop> m_Troops;

	/// Autofunktion aktiviert oder nicht
	BOOLEAN m_bAutoBuild;

	CSystemManager m_Manager;

};

#endif // !defined(AFX_SYSTEM_H__52476A8D_3AFC_4EFC_B456_155002572D31__INCLUDED_)
