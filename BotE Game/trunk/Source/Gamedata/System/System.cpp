/*
 *@file
 * properties of a map location with a sun system and with buildings
 */


#include "stdafx.h"
#include "System.h"
#include "BotEDoc.h"
#include "Races/RaceController.h"
#include "General/Loc.h"
#include "Ships/ships.h"
#include "ClientWorker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// construction/destruction
//////////////////////////////////////////////////////////////////////
CSystem::CSystem() :
	CSector()
{
	ResetSystem(false);
}

CSystem::CSystem(int x, int y) :
	CSector(x, y)
{
	ResetSystem(false);
}

CSystem::CSystem(const CSystem &other) :
	CSector(other),
	m_sOwnerOfSystem(other.m_sOwnerOfSystem),
	m_dHabitants(other.m_dHabitants),
	m_AssemblyList(other.m_AssemblyList),
	m_Production(other.m_Production),
	m_Workers(other.m_Workers),
	m_iMoral(other.m_iMoral),
	m_byBlockade(other.m_byBlockade),
	m_Store(other.m_Store),
	m_iFoodBuildings(other.m_iFoodBuildings),
	m_iIndustryBuildings(other.m_iIndustryBuildings),
	m_iEnergyBuildings(other.m_iEnergyBuildings),
	m_iSecurityBuildings(other.m_iSecurityBuildings),
	m_iResearchBuildings(other.m_iResearchBuildings),
	m_iTitanMines(other.m_iTitanMines),
	m_iDeuteriumMines(other.m_iDeuteriumMines),
	m_iDuraniumMines(other.m_iDuraniumMines),
	m_iCrystalMines(other.m_iCrystalMines),
	m_iIridiumMines(other.m_iIridiumMines),
	m_byMaxTradeRoutesFromHab(other.m_byMaxTradeRoutesFromHab),
	m_bAutoBuild(other.m_bAutoBuild),
	m_Manager(other.m_Manager)
{
	m_Buildings.Copy(other.m_Buildings);
	m_BuildableBuildings.Copy(other.m_BuildableBuildings);
	m_BuildableShips.Copy(other.m_BuildableShips);
	m_BuildableTroops.Copy(other.m_BuildableTroops);
	m_AllwaysBuildableBuildings.Copy(other.m_AllwaysBuildableBuildings);
	m_BuildableWithoutAssemblylistCheck.Copy(other.m_BuildableWithoutAssemblylistCheck);
	m_BuildingDestroy.Copy(other.m_BuildingDestroy);
	m_TradeRoutes.Copy(other.m_TradeRoutes);
	m_ResourceRoutes.Copy(other.m_ResourceRoutes);
	m_Troops.Copy(other.m_Troops);
	m_BuildableUpdates.Copy(other.m_BuildableUpdates);

	const unsigned size = sizeof(m_bDisabledProductions) / sizeof(m_bDisabledProductions[0]);
	for(unsigned i = 0; i < size; ++i)
		m_bDisabledProductions[i] = other.m_bDisabledProductions[i];

}

CSystem::~CSystem()
{
	ResetSystem(false);
}

CSystem& CSystem::operator=(const CSystem& other)
{
	m_sOwnerOfSystem = other.m_sOwnerOfSystem;
	m_dHabitants = other.m_dHabitants;
	m_AssemblyList = other.m_AssemblyList;
	m_Production = other.m_Production;
	m_Workers = other.m_Workers;
	m_iMoral = other.m_iMoral;
	m_byBlockade = other.m_byBlockade;
	m_Store = other.m_Store;
	m_iFoodBuildings = other.m_iFoodBuildings;
	m_iIndustryBuildings = other.m_iIndustryBuildings;
	m_iEnergyBuildings = other.m_iEnergyBuildings;
	m_iSecurityBuildings = other.m_iSecurityBuildings;
	m_iResearchBuildings = other.m_iResearchBuildings;
	m_iTitanMines = other.m_iTitanMines;
	m_iDeuteriumMines = other.m_iDeuteriumMines;
	m_iDuraniumMines = other.m_iDuraniumMines;
	m_iCrystalMines = other.m_iCrystalMines;
	m_iIridiumMines = other.m_iIridiumMines;
	m_byMaxTradeRoutesFromHab = other.m_byMaxTradeRoutesFromHab;
	m_bAutoBuild = other.m_bAutoBuild;
	m_Manager = other.m_Manager;

	m_Buildings.Copy(other.m_Buildings);
	m_BuildableBuildings.Copy(other.m_BuildableBuildings);
	m_BuildableUpdates.Copy(other.m_BuildableUpdates);
	m_BuildableShips.Copy(other.m_BuildableShips);
	m_BuildableTroops.Copy(other.m_BuildableTroops);
	m_AllwaysBuildableBuildings.Copy(other.m_AllwaysBuildableBuildings);
	m_BuildableWithoutAssemblylistCheck.Copy(other.m_BuildableWithoutAssemblylistCheck);
	m_BuildingDestroy.Copy(other.m_BuildingDestroy);
	m_TradeRoutes.Copy(other.m_TradeRoutes);
	m_ResourceRoutes.Copy(other.m_ResourceRoutes);
	m_Troops.Copy(other.m_Troops);

	const unsigned size = sizeof(m_bDisabledProductions) / sizeof(m_bDisabledProductions[0]);
	for(unsigned i = 0; i < size; ++i)
		m_bDisabledProductions[i] = other.m_bDisabledProductions[i];

	return (*this);
};

// Funktion setzt alle Werte wieder zur�ck auf standard.
void CSystem::ResetSystem(bool call_up)
{
	if(call_up)
		CSector::Reset(call_up);

	m_sOwnerOfSystem = "";			// Der Besitzer des Systems
	m_dHabitants = 0.0f;			// Einwohner in dem System
	m_iMoral = 100;					// Moral im System
	m_byBlockade = 0;				// Prozentwert der Blockade im System
	// Anzahl der Geb�ude des Types
	m_iFoodBuildings = 0;			// Anzahl der Nahrungsgeb�ude in dem System
	m_iIndustryBuildings = 0;		// Anzahl der Industriegeb�ude in dem System
	m_iEnergyBuildings = 0;			// Anzahl der Energiegeb�ude in dem System
	m_iSecurityBuildings = 0;		// Anzahl der Geheimdienstgeb�ude in dem System
	m_iResearchBuildings = 0;		// Anzahl der Forschungsgeb�ude in dem System
	m_iTitanMines = 0;				// Anzahl der Titanminen in dem System
	m_iDeuteriumMines = 0;			// Anzahl der Deuteriumminen in dem System
	m_iDuraniumMines = 0;			// Anzahl der Duraniumminen in dem System
	m_iIridiumMines = 0;			// Anzahl der Iridiumminen in dem System
	m_iCrystalMines = 0;			// Anzahl der Crystalminen in dem System
	// Lagerkapazit�ten
	m_Store.Reset();
	m_BuildingDestroy.RemoveAll();
	m_Production.Reset();
	m_AssemblyList.Reset();
	m_Buildings.RemoveAll();
	m_BuildableBuildings.RemoveAll();
	m_AllwaysBuildableBuildings.RemoveAll();
	m_BuildableUpdates.RemoveAll();
	m_BuildableWithoutAssemblylistCheck.RemoveAll();
	m_BuildableShips.RemoveAll();
	m_BuildableTroops.RemoveAll();
	m_TradeRoutes.RemoveAll();
	m_ResourceRoutes.RemoveAll();
	m_byMaxTradeRoutesFromHab = 0;
	m_Troops.RemoveAll();
	m_bAutoBuild = FALSE;
	m_Manager.Reset();

	// Deaktivierte Produktionen zur�cksetzen
	ClearDisabledProductions();
}

//////////////////////////////////////////////////////////////////////
// serialization
//////////////////////////////////////////////////////////////////////
void CSystem::Serialize(CArchive &ar)
{
	m_AssemblyList.Serialize(ar);
	m_Production.Serialize(ar);
	m_Workers.Serialize(ar);
	m_Store.Serialize(ar);
	m_Manager.Serialize(ar);
	// wenn gespeichert wird
	if (ar.IsStoring())
	{
		ar << m_sOwnerOfSystem;
		ar << m_dHabitants;
		ar << m_iMoral;
		ar << m_byBlockade;
		ar << m_iFoodBuildings;
		ar << m_iIndustryBuildings;
		ar << m_iEnergyBuildings;
		ar << m_iSecurityBuildings;
		ar << m_iResearchBuildings;
		ar << m_iTitanMines;
		ar << m_iDeuteriumMines;
		ar << m_iDuraniumMines;
		ar << m_iIridiumMines;
		ar << m_iCrystalMines;
		for (int i = WORKER::FOOD_WORKER; i <= WORKER::IRIDIUM_WORKER; i++)
			ar << m_bDisabledProductions[i];
		ar << m_Buildings.GetSize();
		for (int i = 0; i < m_Buildings.GetSize(); i++)
			m_Buildings.GetAt(i).Serialize(ar);
		m_BuildableBuildings.Serialize(ar);
		m_BuildableUpdates.Serialize(ar);
		m_BuildableShips.Serialize(ar);
		m_BuildableTroops.Serialize(ar);
		m_AllwaysBuildableBuildings.Serialize(ar);
		m_BuildableWithoutAssemblylistCheck.Serialize(ar);
		m_BuildingDestroy.Serialize(ar);
		ar << m_TradeRoutes.GetSize();
		for (int i = 0; i < m_TradeRoutes.GetSize(); i++)
			m_TradeRoutes.GetAt(i).Serialize(ar);
		ar << m_byMaxTradeRoutesFromHab;
		ar << m_ResourceRoutes.GetSize();
		for (int i = 0; i < m_ResourceRoutes.GetSize(); i++)
			m_ResourceRoutes.GetAt(i).Serialize(ar);
		ar << m_Troops.GetSize();
		for (int i = 0; i < m_Troops.GetSize(); i++)
			m_Troops.GetAt(i).Serialize(ar);
		ar << m_bAutoBuild;
	}
	// wenn geladen wird
	if (ar.IsLoading())
	{
		int number = 0;
		ar >> m_sOwnerOfSystem;
		ar >> m_dHabitants;
		ar >> m_iMoral;
		ar >> m_byBlockade;
		ar >> m_iFoodBuildings;
		ar >> m_iIndustryBuildings;
		ar >> m_iEnergyBuildings;
		ar >> m_iSecurityBuildings;
		ar >> m_iResearchBuildings;
		ar >> m_iTitanMines;
		ar >> m_iDeuteriumMines;
		ar >> m_iDuraniumMines;
		ar >> m_iIridiumMines;
		ar >> m_iCrystalMines;
		for (int i = WORKER::FOOD_WORKER; i <= WORKER::IRIDIUM_WORKER; i++)
			ar >> m_bDisabledProductions[i];
		ar >> number;
		m_Buildings.RemoveAll();
		m_Buildings.SetSize(number);
		for (int i = 0; i < number; i++)
			m_Buildings.GetAt(i).Serialize(ar);
		m_BuildableBuildings.RemoveAll();
		m_BuildableUpdates.RemoveAll();
		m_BuildableShips.RemoveAll();
		m_BuildableTroops.RemoveAll();
		m_AllwaysBuildableBuildings.RemoveAll();
		m_BuildableWithoutAssemblylistCheck.RemoveAll();
		m_BuildingDestroy.RemoveAll();

		m_BuildableBuildings.Serialize(ar);
		m_BuildableUpdates.Serialize(ar);
		m_BuildableShips.Serialize(ar);
		m_BuildableTroops.Serialize(ar);
		m_AllwaysBuildableBuildings.Serialize(ar);
		m_BuildableWithoutAssemblylistCheck.Serialize(ar);
		m_BuildingDestroy.Serialize(ar);
		ar >> number;
		m_TradeRoutes.RemoveAll();
		m_TradeRoutes.SetSize(number);
		for (int i = 0; i < number; i++)
			m_TradeRoutes.GetAt(i).Serialize(ar);
		ar >> m_byMaxTradeRoutesFromHab;
		ar >> number;
		m_ResourceRoutes.RemoveAll();
		m_ResourceRoutes.SetSize(number);
		for (int i = 0; i < number; i++)
			m_ResourceRoutes.GetAt(i).Serialize(ar);
		ar >> number;
		m_Troops.RemoveAll();
		m_Troops.SetSize(number);
		for (int i = 0; i < number; i++)
			m_Troops.GetAt(i).Serialize(ar);
		ar >> m_bAutoBuild;
	}
}

//////////////////////////////////////////////////////////////////////
// proper getting
//////////////////////////////////////////////////////////////////////

// Funktion gibt die Anzahl oder die RunningNumber (ID) der Geb�ude zur�ck, welche Arbeiter ben�tigen.
// Wir �bergeben daf�r als Parameter den Typ des Geb�udes (FARM, BAUHOF usw.) und einen Modus.
// Ist der Modus NULL, dann bekommen wir die Anzahl zur�ck, ist der Modus EINS, dann die RunningNumber.
USHORT CSystem::GetNumberOfWorkbuildings(WORKER::Typ nWorker, int Modus) const
{
	// "Modus" gibt an, ob wir die Anzahl der Geb�ude oder die aktuelle RunningNumber
	// des speziellen Geb�udes zur�ckgegeben wird
	// Modus == 0 -> Anzahl
	// Modus == 1 -> RunningNumber
	if (Modus == 0)
	{
		USHORT returnValue = 0;
		if (nWorker == WORKER::FOOD_WORKER) returnValue = m_iFoodBuildings;
		else if (nWorker == WORKER::INDUSTRY_WORKER) returnValue = m_iIndustryBuildings;
		else if (nWorker == WORKER::ENERGY_WORKER) returnValue = m_iEnergyBuildings;
		else if (nWorker == WORKER::SECURITY_WORKER) returnValue = m_iSecurityBuildings;
		else if (nWorker == WORKER::RESEARCH_WORKER) returnValue = m_iResearchBuildings;
		else if (nWorker == WORKER::TITAN_WORKER) returnValue = m_iTitanMines;
		else if (nWorker == WORKER::DEUTERIUM_WORKER) returnValue = m_iDeuteriumMines;
		else if (nWorker == WORKER::DURANIUM_WORKER) returnValue = m_iDuraniumMines;
		else if (nWorker == WORKER::CRYSTAL_WORKER) returnValue = m_iCrystalMines;
		else if (nWorker == WORKER::IRIDIUM_WORKER) returnValue = m_iIridiumMines;
		else if (nWorker == WORKER::ALL_WORKER) returnValue = m_Workers.GetWorker(WORKER::ALL_WORKER);
		return returnValue;
	}
	else if (Modus == 1)
	{
		for (int i = 0; i < m_Buildings.GetSize(); i++)
		{
			const CBuildingInfo* buildingInfo = &resources::BuildingInfo->GetAt(m_Buildings.GetAt(i).GetRunningNumber() - 1);

			if (buildingInfo->GetWorker())
			{
				USHORT runningNumber = 0;
				if (nWorker == WORKER::FOOD_WORKER)
				{
					if (buildingInfo->GetFoodProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
				else if (nWorker == WORKER::INDUSTRY_WORKER)
				{
					if (buildingInfo->GetIPProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
				else if (nWorker == WORKER::ENERGY_WORKER)
				{
					if (buildingInfo->GetEnergyProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
				else if (nWorker == WORKER::SECURITY_WORKER)
				{
					if (buildingInfo->GetSPProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
				else if (nWorker == WORKER::RESEARCH_WORKER)
				{
					if (buildingInfo->GetFPProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
				else if (nWorker == WORKER::TITAN_WORKER)
				{
					if (buildingInfo->GetTitanProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
				else if (nWorker == WORKER::DEUTERIUM_WORKER)
				{
					if (buildingInfo->GetDeuteriumProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
				else if (nWorker == WORKER::DURANIUM_WORKER)
				{
					if (buildingInfo->GetDuraniumProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
				else if (nWorker == WORKER::CRYSTAL_WORKER)
				{
					if (buildingInfo->GetCrystalProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
				else if (nWorker == WORKER::IRIDIUM_WORKER)
				{
					if (buildingInfo->GetIridiumProd())
					{
						runningNumber = m_Buildings.GetAt(i).GetRunningNumber();
						return runningNumber;
					}
				}
			}
		}
	}
	return 0;
}

// Funktion gibt die Anzahl des Geb�udes mit der �bergebenen RunningNumber zur�ck.
USHORT CSystem::GetNumberOfBuilding(USHORT runningNumber) const
{
	USHORT number = 0;
	for (int i = 0; i < m_Buildings.GetSize(); i++)
		if (runningNumber == m_Buildings.GetAt(i).GetRunningNumber())
			number++;
	return number;
}

UINT CSystem::GetDeritiumStoreMax() const
{
	short multi = 1;
	const CMajor* major = dynamic_cast<CMajor*>(resources::pDoc->GetRaceCtrl()->GetRace(m_sOwnerOfSystem));
	const CResearchInfo* info = major->GetEmpire()->GetResearch()->GetResearchInfo();
	if (info->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->
		GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
		multi = info->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetBonus(1);
	return MAX_DERITIUM_STORE * multi;
}

int CSystem::GetXStoreMax(RESOURCES::TYPE x) const
{
	switch(x)
	{
		case RESOURCES::TITAN: return GetTitanStoreMax();
		case RESOURCES::DEUTERIUM: return GetDeuteriumStoreMax();
		case RESOURCES::DURANIUM: return GetDuraniumStoreMax();
		case RESOURCES::CRYSTAL: return GetCrystalStoreMax();
		case RESOURCES::IRIDIUM: return GetIridiumStoreMax();
		case RESOURCES::DERITIUM: return GetDeritiumStoreMax();
		case RESOURCES::FOOD: return GetFoodStoreMax();
	}
	assert(false);
	return 0;
}

static RESOURCES::TYPE WorkerToResource(WORKER::Typ type)
{
	std::map<WORKER::Typ, RESOURCES::TYPE> transformer;
	transformer.insert(std::pair<WORKER::Typ, RESOURCES::TYPE>(WORKER::TITAN_WORKER, RESOURCES::TITAN));
	transformer.insert(std::pair<WORKER::Typ, RESOURCES::TYPE>(WORKER::DEUTERIUM_WORKER, RESOURCES::DEUTERIUM));
	transformer.insert(std::pair<WORKER::Typ, RESOURCES::TYPE>(WORKER::DURANIUM_WORKER, RESOURCES::DURANIUM));
	transformer.insert(std::pair<WORKER::Typ, RESOURCES::TYPE>(WORKER::CRYSTAL_WORKER, RESOURCES::CRYSTAL));
	transformer.insert(std::pair<WORKER::Typ, RESOURCES::TYPE>(WORKER::IRIDIUM_WORKER, RESOURCES::IRIDIUM));
	transformer.insert(std::pair<WORKER::Typ, RESOURCES::TYPE>(WORKER::FOOD_WORKER, RESOURCES::FOOD));
	const std::map<WORKER::Typ, RESOURCES::TYPE>::const_iterator it = transformer.find(type);
	assert(it != transformer.end());
	return it->second;
}

int CSystem::GetXStoreMax(WORKER::Typ x) const
{
	return GetXStoreMax(WorkerToResource(x));
}

// Funktion gibt den Lagerinhalt der Ressource zur�ck, die an die Funktion �bergeben wurde.
UINT CSystem::GetResourceStore(USHORT res) const
{
	switch (res)
	{
	case RESOURCES::TITAN: {return this->GetTitanStore();}
	case RESOURCES::DEUTERIUM: {return this->GetDeuteriumStore();}
	case RESOURCES::DURANIUM: {return this->GetDuraniumStore();}
	case RESOURCES::CRYSTAL: {return this->GetCrystalStore();}
	case RESOURCES::IRIDIUM: {return this->GetIridiumStore();}
	case RESOURCES::DERITIUM: {return this->GetDeritiumStore();}
	case RESOURCES::FOOD: { return this->GetFoodStore();}
	}
	return 0;
}

int CSystem::GetResourceStore(WORKER::Typ type) const
{
	assert(HasStore(type));
	return GetResourceStore(WorkerToResource(type));
}

// Funktion gibt die Anzahl der aktuell in dem Feld gespeicherten Geb�ude mit der RunningNumber == number zur�ck.
// Diese Geb�ude sollen am Ende der Runde abgerissen werden.
USHORT CSystem::GetBuildingDestroy(int RunningNumber) const
{
	USHORT number = 0;
	for (int i = 0; i < m_BuildingDestroy.GetSize(); i++)
		if (m_BuildingDestroy.GetAt(i) == RunningNumber)
			number++;
	return number;
}

int CSystem::GetXBuildings(WORKER::Typ x) const
{
	switch(x)
	{
		case WORKER::FOOD_WORKER: return m_iFoodBuildings;
		case WORKER::INDUSTRY_WORKER: return m_iIndustryBuildings;
		case WORKER::ENERGY_WORKER: return m_iEnergyBuildings;
		case WORKER::SECURITY_WORKER: return m_iSecurityBuildings;
		case WORKER::RESEARCH_WORKER: return m_iResearchBuildings;
		case WORKER::TITAN_WORKER: return m_iTitanMines;
		case WORKER::DEUTERIUM_WORKER: return m_iDeuteriumMines;
		case WORKER::DURANIUM_WORKER: return m_iDuraniumMines;
		case WORKER::CRYSTAL_WORKER: return m_iCrystalMines;
		case WORKER::IRIDIUM_WORKER: return m_iIridiumMines;
	}
	assert(false);
	return 0;
}

//////////////////////////////////////////////////////////////////////
// questionable getting
//////////////////////////////////////////////////////////////////////

// Funktion gibt einen Zeiger auf den Lagerinhalt der Ressource zur�ck, die an die Funktion �bergeben wurde.
const int* CSystem::GetResourceStorages(USHORT res) const
{
	switch (res)
	{
	case RESOURCES::TITAN: {return &m_Store.Titan;}
	case RESOURCES::DEUTERIUM: {return &m_Store.Deuterium;}
	case RESOURCES::DURANIUM: {return &m_Store.Duranium;}
	case RESOURCES::CRYSTAL: {return &m_Store.Crystal;}
	case RESOURCES::IRIDIUM: {return &m_Store.Iridium;}
	case RESOURCES::DERITIUM: {return &m_Store.Deritium;}
	}
	return 0;
}

int* CSystem::GetResourceStorages(USHORT res)
{
	switch (res)
	{
	case RESOURCES::TITAN: {return &m_Store.Titan;}
	case RESOURCES::DEUTERIUM: {return &m_Store.Deuterium;}
	case RESOURCES::DURANIUM: {return &m_Store.Duranium;}
	case RESOURCES::CRYSTAL: {return &m_Store.Crystal;}
	case RESOURCES::IRIDIUM: {return &m_Store.Iridium;}
	case RESOURCES::DERITIUM: {return &m_Store.Deritium;}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// setting
//////////////////////////////////////////////////////////////////////

// Funktion setzt den neuen Besitzer des Systems. �bergeben wird der Besitzer.
void CSystem::SetOwnerOfSystem(const CString& sOwnerOfSystem)
{
	// �ndert sich nichts, dann aus der Funktion springen
	if (m_sOwnerOfSystem == sOwnerOfSystem)
		return;

	// neuen Besitzer festlegen
	m_sOwnerOfSystem = sOwnerOfSystem;

	// alle alten Ressourcen- und Handelsrouten l�schen
	m_TradeRoutes.RemoveAll();
	m_ResourceRoutes.RemoveAll();

	// Alle Truppen entfernen
	m_Troops.RemoveAll();

	// Bauliste hart l�schen
	m_AssemblyList.Reset();
}

// Funktion setzt die Bev�lkerungsanzahl des Systems. �bergeben wird die Bev�lkerung aller Planeten des Sektors.
// Gleichzeitig �berpr�ft die Funktion auch, ob man eine weitere Handelsroute aufgrund der Bev�lkerung bekommt, dann
// gibt die Funktion ein <code>TRUE</code> zur�ck, ansonsten <code>FALSE</code>.
BOOLEAN CSystem::SetHabitants(double habitants)
{
	m_dHabitants = habitants;

	if (((USHORT)m_dHabitants / TRADEROUTEHAB) > m_byMaxTradeRoutesFromHab)
	{
		m_byMaxTradeRoutesFromHab++;
		if (m_sOwnerOfSystem != "")
			return TRUE;
	}
	// Wenn die Bev�lkerung wieder zu niedrig ist, dann die Varibale wieder runterz�hlen
	else if (((USHORT)m_dHabitants / TRADEROUTEHAB) < m_byMaxTradeRoutesFromHab)
		m_byMaxTradeRoutesFromHab--;
	return FALSE;
}

// Funktion sagt ob ein bestimmtes Geb�ude in dem System baubar ist oder nicht. Als Parameter werden daf�r
// die RunningNumber des gew�nschten Geb�udes und der Wert �bergeben.
void CSystem::SetBuildableBuildings(int RunningNumber, BOOLEAN TrueOrFalse)
{
	if (TrueOrFalse == FALSE)
	{
		for (int t = 0; t < m_BuildableBuildings.GetSize(); t++)
			if (m_BuildableBuildings.GetAt(t) == RunningNumber)
			{
				m_BuildableBuildings.RemoveAt(t);
				break;
			}
	}
	else
		m_BuildableBuildings.Add(RunningNumber);
}

// Funktion sagt ob ein bestimmtes Geb�udeupdate in dem System baubar ist oder nicht. Als Parameter werden daf�r
// die RunningNumber des gew�nschten Geb�udes und der Wert �bergeben.
void CSystem::SetBuildableUpdates(int RunningNumber, BOOLEAN TrueOrFalse)
{
	if (TrueOrFalse == FALSE)
	{
		for (int t = 0; t < m_BuildableUpdates.GetSize(); t++)
			if (m_BuildableUpdates.GetAt(t) == RunningNumber)
			{
				m_BuildableUpdates.RemoveAt(t);
				break;
			}
	}
	else
		m_BuildableUpdates.Add(RunningNumber);
}

// Setzt ein Geb�ude mit der richtigen RunningNumber online bzw. offline
void CSystem::SetIsBuildingOnline(int index, BOOLEAN newStatus)
{
	ASSERT(index < m_Buildings.GetSize());
	m_Buildings.ElementAt(index).SetIsBuildingOnline(newStatus);
}

// Komplette Zugriffsfunktion f�r das Arbeiterobjekt. Bei Modus 0 wird der "WhatWorker" inkrementiert, bei Modus 2 wird
// er dekrementiert und bei Modus 2 wird der "WhatWorker" auf den Wert von Value gesetzt.
void CSystem::SetWorker(WORKER::Typ nWhatWorker, SetWorkerMode Modus, int Value)
{
	if(Modus != SET_WORKER_MODE_SET)
		assert(Value == -1);
	else
		assert(Value >= 0);

	// Modus == 0 --> Inkrement
	// Modus == 1 --> Dekrement
	// Modus == 2 --> SetWorkers inkl. Value
	if (Modus == SET_WORKER_MODE_INCREMENT)
		m_Workers.InkrementWorker(nWhatWorker);
	else if (Modus == SET_WORKER_MODE_DECREMENT)
		m_Workers.DekrementWorker(nWhatWorker);
	else if (Modus == SET_WORKER_MODE_SET)
		m_Workers.SetWorker(nWhatWorker, Value);
}

// Funktion setzt alle vorhandenen Arbeiter soweit wie m�glich in Geb�ude, die Arbeiter ben�tigen.
void CSystem::SetWorkersIntoBuildings()
{
	m_Workers.SetWorker(WORKER::ALL_WORKER, (int)(m_dHabitants));
	m_Workers.CalculateFreeWorkers();

	while(m_Workers.GetWorker(WORKER::FREE_WORKER) > 0)
	{
		bool all_buildings_full = true;
		for (int i = WORKER::FOOD_WORKER; i <= WORKER::IRIDIUM_WORKER; i++)
		{
			WORKER::Typ nWorker = (WORKER::Typ)i;
			if (m_Workers.GetWorker(WORKER::FREE_WORKER) > 0 && GetNumberOfWorkbuildings(nWorker,0) > m_Workers.GetWorker(nWorker))
			{
				all_buildings_full = false;
				m_Workers.InkrementWorker(nWorker);
				m_Workers.DekrementWorker(WORKER::FREE_WORKER);
			}
		}
		if(all_buildings_full)
			break;
	}
}

void CSystem::SetStores(const GameResources& add)
{
	m_Store += add;
}

// Funktion addiert resAdd zum Lagerinhalt der jeweiligen Ressource.
void CSystem::SetResourceStore(USHORT res, int resAdd)
{
	// �berpr�fung das wir bei Lagerverkleinerung keine negativen Lager bekommen k�nnen
	if (resAdd < 0)
		if ((resAdd + (int)this->GetResourceStore(res)) < 0)
			resAdd = this->GetResourceStore(res) * (-1);
	// zum Lager hinzuf�gen/entfernen
	switch (res)
	{
	case RESOURCES::TITAN:		{m_Store.Titan += resAdd; break;}
	case RESOURCES::DEUTERIUM: {m_Store.Deuterium += resAdd; break;}
	case RESOURCES::DURANIUM:	{m_Store.Duranium += resAdd; break;}
	case RESOURCES::CRYSTAL:	{m_Store.Crystal += resAdd; break;}
	case RESOURCES::IRIDIUM:	{m_Store.Iridium += resAdd; break;}
	case RESOURCES::DERITIUM:	{m_Store.Deritium += resAdd; break;}
	}
}

// Diese Funktion setzt die abzurei�enden Geb�ude fest. Die zu �bergebenden Parameter sind die RunningNumber
// des Geb�udes und einen Wert 1 f�r hinzuf�gen und 0 f�r wieder aus der Liste nehmen
void CSystem::SetBuildingDestroy(int RunningNumber, BOOLEAN add)
{
	// Geb�ude zu Liste hinzuf�gen
	if (add == TRUE)
	{
		USHORT maxDestroy = this->GetNumberOfBuilding(RunningNumber);
		if (m_byBlockade > NULL)
			maxDestroy -= maxDestroy * m_byBlockade / 100;
		if (maxDestroy > this->GetBuildingDestroy(RunningNumber))
			m_BuildingDestroy.Add(RunningNumber);
	}
	else
	{
		for (int i = 0; i < m_BuildingDestroy.GetSize(); i++)
			if (m_BuildingDestroy.GetAt(i) == RunningNumber)
			{
				m_BuildingDestroy.RemoveAt(i);
				break;
			}
	}
}

void CSystem::FreeAllWorkers()
{
	m_Workers.FreeAll();
}

// Alle deaktivierten Produktionen zur�cksetzen
void CSystem::ClearDisabledProductions()
{
	memset(m_bDisabledProductions, false, sizeof(m_bDisabledProductions));
}

//////////////////////////////////////////////////////////////////////
// bool info
//////////////////////////////////////////////////////////////////////

bool CSystem::HasWorkerBuilding(WORKER::Typ type) const
{
	return GetXBuildings(type) > 0;
}

bool CSystem::HasStore(WORKER::Typ type) const
{
	return
		type == WORKER::TITAN_WORKER ||
		type == WORKER::DEUTERIUM_WORKER ||
		type == WORKER::DURANIUM_WORKER ||
		type == WORKER::CRYSTAL_WORKER ||
		type == WORKER::IRIDIUM_WORKER ||
		type == WORKER::FOOD_WORKER;
}

bool CSystem::CanTakeOnline(const CBuildingInfo& info) const
{
	if(m_Manager.Active())
		return info.GetNeededEnergy() <= m_Production.GetAvailableEnergy();
	return m_Production.m_iEnergyProd >= info.GetNeededEnergy();
}

bool CSystem::SanityCheckWorkers()
{
	m_Workers.CalculateFreeWorkers();
	const bool in_range = SanityCheckWorkersInRange(WORKER::FOOD_WORKER)
		&& SanityCheckWorkersInRange(WORKER::INDUSTRY_WORKER)
		&& SanityCheckWorkersInRange(WORKER::ENERGY_WORKER)
		&& SanityCheckWorkersInRange(WORKER::SECURITY_WORKER)
		&& SanityCheckWorkersInRange(WORKER::RESEARCH_WORKER)
		&& SanityCheckWorkersInRange(WORKER::TITAN_WORKER)
		&& SanityCheckWorkersInRange(WORKER::DEUTERIUM_WORKER)
		&& SanityCheckWorkersInRange(WORKER::DURANIUM_WORKER)
		&& SanityCheckWorkersInRange(WORKER::CRYSTAL_WORKER)
		&& SanityCheckWorkersInRange(WORKER::IRIDIUM_WORKER);
	if(!in_range)
		return false;

	return m_Workers.GetWorker(WORKER::ALL_WORKER) == m_Workers.GetWorker(WORKER::FREE_WORKER)
		+ m_Workers.GetWorker(WORKER::FOOD_WORKER)
		+ m_Workers.GetWorker(WORKER::INDUSTRY_WORKER)
		+ m_Workers.GetWorker(WORKER::ENERGY_WORKER)
		+ m_Workers.GetWorker(WORKER::SECURITY_WORKER)
		+ m_Workers.GetWorker(WORKER::RESEARCH_WORKER)
		+ m_Workers.GetWorker(WORKER::TITAN_WORKER)
		+ m_Workers.GetWorker(WORKER::DEUTERIUM_WORKER)
		+ m_Workers.GetWorker(WORKER::DURANIUM_WORKER)
		+ m_Workers.GetWorker(WORKER::CRYSTAL_WORKER)
		+ m_Workers.GetWorker(WORKER::IRIDIUM_WORKER);
}

bool CSystem::SanityCheckWorkersInRange(WORKER::Typ type) const
{
	const int workers = m_Workers.GetWorker(type);
	return 0 <= workers && workers <= GetNumberOfWorkbuildings(type, 0);
}

//////////////////////////////////////////////////////////////////////
// calculation of member variables
//////////////////////////////////////////////////////////////////////

namespace //helpers for CalculateVariables()
{
	void AddBonusToProd(int& prod, int bonus)
	{
		prod += bonus * prod / 100;
	}

	void AddPlanetBonusToBonus(short& base_bonus, int size)
	{
		base_bonus += size * 25;
	}
}

// Funktion berechnet aus den Eigenschaften der stehenden Geb�ude alle Attribute der System-Klasse.
void CSystem::CalculateVariables()
{
	const CMajor* pOwner = dynamic_cast<CMajor*>(resources::pDoc->GetRaceCtrl()->GetRace(m_sOwnerOfSystem));
	assert(pOwner);
	const CResearchInfo* const ResearchInfo = pOwner->GetEmpire()->GetResearch()->GetResearchInfo();
	const BuildingInfoArray* const buildingInfos = resources::BuildingInfo;

	const int NumberOfBuildings = m_Buildings.GetSize();
	// Alle werde wieder auf NULL setzen
	m_Production.Reset();
	// Die Anzahl der Arbeiter aus der aktuellen Bev�lkerung berechnen und auch an die Klasse CWorker �bergeben
	m_Workers.SetWorker(WORKER::ALL_WORKER,(int)(m_dHabitants));
	m_Workers.CheckWorkers();

	// Die Creditsprod. aus der Bev�lkerung berechnen und modifizieren durch jeweilige Rasseneigenschaft
	m_Production.m_iCreditsProd = (int)(m_dHabitants);
	m_Production.m_iCreditsProd = (int)(m_Production.m_iCreditsProd * pOwner->CreditsMulti());


	// Die Geb�ude online setzen, wenn das Objekt der Klasse CWorker das sagt
	// zuerst die Anzahl der Arbeiter auslesen und schauen ob die Arbeiter vielleicht gr��er sind als die
	// Anzahl der jeweiligen Geb�ude (z.B. durch Abri� aus letzter Runde) -> dann Arbeiter auf Geb�udeanzahl verringern
	assert(WORKER::FOOD_WORKER == 0 && WORKER::IRIDIUM_WORKER == 9);
	std::vector<int> workers;
	workers.reserve(10);
	for(int i = WORKER::FOOD_WORKER; i <= WORKER::IRIDIUM_WORKER; ++i)
	{
		WORKER::Typ type = static_cast<WORKER::Typ>(i);
		workers.push_back(m_Workers.Cap(type, GetXBuildings(type)));
	}

	// Wenn wir Handelsg�ter in der Bauliste stehen haben, dann Anzahl der Online-Fabs in Credits umrechnen
	if (m_AssemblyList.IsEmpty())
		m_Production.m_iCreditsProd += workers.at(static_cast<int>(WORKER::INDUSTRY_WORKER));

	// Die einzelnen Produktionen berechnen
	int ind_buildings_used = 0;//tracking variables for industry/energy potentials to not use more than ALL_WORKERS
	int energy_buildings_used = 0;
	for (int i = 0; i < NumberOfBuildings; i++)
	{
		const CBuildingInfo* buildingInfo = &buildingInfos->GetAt(m_Buildings.GetAt(i).GetRunningNumber() - 1);
		CBuilding& building = m_Buildings.GetAt(i);

		// Bei einer Blockade werden auch die Werften offline gesetzt
		if (m_byBlockade >= 100 && buildingInfo->GetShipYard() && buildingInfo->GetNeededEnergy() > 0)
			building.SetIsBuildingOnline(FALSE);

		// Geb�ude offline setzen
		if (buildingInfo->GetWorker() == TRUE)
		{
			building.SetIsBuildingOnline(FALSE);

			// Jetzt wieder wenn m�glich online setzen
			for(std::vector<int>::iterator it = workers.begin(); it != workers.end(); ++it)
				if(*it > 0 && buildingInfo->GetXProd(static_cast<WORKER::Typ>(it - workers.begin())) > 0)
				{
					--*it;
					building.SetIsBuildingOnline(TRUE);
				}
			if(buildingInfo->GetEnergyProd() > 0)
				++energy_buildings_used;
			if(buildingInfo->GetIPProd() > 0)
				++ind_buildings_used;
		}
		// Die einzelnen Produktionen berechnen (ohne Boni)
		m_Production.CalculateProduction(buildingInfo, building.GetIsBuildingOnline(),
			ind_buildings_used <= m_Workers.GetWorker(WORKER::ALL_WORKER),
			energy_buildings_used <= m_Workers.GetWorker(WORKER::ALL_WORKER));
	}

	// falls vorhanden, deaktiverte Produktionen auf 0 setzen
	m_Production.DisableProductions(m_bDisabledProductions);

	// Credits durch Handelsrouten berechnen
	m_Production.m_iCreditsProd += CreditsFromTradeRoutes();

	// Besitzt jemand ein Monopol auf eine Ressource, so verdoppelt sich seine Produktion
	const CString *const sMonopolOwner = CTrade::GetMonopolOwner();
	if (sMonopolOwner[RESOURCES::TITAN] == m_sOwnerOfSystem)
		m_Production.m_iTitanProd *= 2;
	if (sMonopolOwner[RESOURCES::DEUTERIUM] == m_sOwnerOfSystem)
		m_Production.m_iDeuteriumProd *= 2;
	if (sMonopolOwner[RESOURCES::DURANIUM] == m_sOwnerOfSystem)
		m_Production.m_iDuraniumProd *= 2;
	if (sMonopolOwner[RESOURCES::CRYSTAL] == m_sOwnerOfSystem)
		m_Production.m_iCrystalProd *= 2;
	if (sMonopolOwner[RESOURCES::IRIDIUM] == m_sOwnerOfSystem)
		m_Production.m_iIridiumProd *= 2;

	// Die Boni auf die einzelnen Produktionen berechnen
	short tmpFoodBoni		= pOwner->GetEmpire()->GetResearch()->GetBioTech() * TECHPRODBONUS;
	short tmpIndustryBoni	= pOwner->GetEmpire()->GetResearch()->GetConstructionTech() * TECHPRODBONUS;
	short tmpEnergyBoni		= pOwner->GetEmpire()->GetResearch()->GetEnergyTech() * TECHPRODBONUS;
	short tmpSecurityBoni	= 0;
	short tmpResearchBoni	= 0;
	short tmpTitanBoni		= 0;
	short tmpDeuteriumBoni	= 0;
	short tmpDuraniumBoni	= 0;
	short tmpCrystalBoni	= 0;
	short tmpIridiumBoni	= 0;
	short tmpDeritiumBoni	= 0;
	short tmpCreditsBoni	= 0;
	short tmpAllRessourcesBoni = 0;

	short neededEnergy = 0;
	for (int i = 0; i < NumberOfBuildings; i++)
	{
		const CBuildingInfo* buildingInfo = &buildingInfos->GetAt(m_Buildings.GetAt(i).GetRunningNumber() - 1);
		const CBuilding& building = m_Buildings.GetAt(i);
		// Hier die n�tige Energie von der produzierten abziehen,
		// geht aber nur hier, wenn wir keine Boni zur Energie reinmachen
		if (building.GetIsBuildingOnline() && buildingInfo->GetNeededEnergy() > 0)
			neededEnergy += buildingInfo->GetNeededEnergy();

		if (building.GetIsBuildingOnline())
		{
			// Es wird IMMER abgerundet, gemacht durch "floor"
			tmpFoodBoni			+= buildingInfo->GetFoodBoni();
			tmpIndustryBoni		+= buildingInfo->GetIndustryBoni();
			tmpEnergyBoni		+= buildingInfo->GetEnergyBoni();
			tmpSecurityBoni		+= buildingInfo->GetSecurityBoni();
			tmpResearchBoni		+= buildingInfo->GetResearchBoni();
			tmpAllRessourcesBoni = buildingInfo->GetAllRessourcesBoni();
			tmpTitanBoni		+= buildingInfo->GetTitanBoni() + tmpAllRessourcesBoni;
			tmpDeuteriumBoni	+= buildingInfo->GetDeuteriumBoni() + tmpAllRessourcesBoni;
			tmpDuraniumBoni		+= buildingInfo->GetDuraniumBoni() + tmpAllRessourcesBoni;
			tmpCrystalBoni		+= buildingInfo->GetCrystalBoni() + tmpAllRessourcesBoni;
			tmpIridiumBoni		+= buildingInfo->GetIridiumBoni() + tmpAllRessourcesBoni;
			tmpDeritiumBoni		+= buildingInfo->GetDeritiumBoni();
			tmpCreditsBoni		+= buildingInfo->GetCreditsBoni();
		}
	}

	// Jetzt werden noch eventuelle Boni durch die Planetenklassen dazugerechnet
	int deritiumProdMulti = 0;
	for(std::vector<CPlanet>::const_iterator it = m_Planets.begin(); it != m_Planets.end(); ++it)
	{
		if(!it->GetColonized() || it->GetCurrentHabitant() <= 0.0f)
			continue;
		const int size_plus_one = it->GetSize() + 1;
		const BOOLEAN* boni = it->GetBoni();
		// pro Planetengr��e gibt es 25% Bonus
		if (boni[RESOURCES::TITAN])
			AddPlanetBonusToBonus(tmpTitanBoni, size_plus_one);
		if (boni[RESOURCES::DEUTERIUM])
			AddPlanetBonusToBonus(tmpDeuteriumBoni, size_plus_one);
		if (boni[RESOURCES::DURANIUM])
			AddPlanetBonusToBonus(tmpDuraniumBoni, size_plus_one);
		if (boni[RESOURCES::CRYSTAL])
			AddPlanetBonusToBonus(tmpCrystalBoni, size_plus_one);
		if (boni[RESOURCES::IRIDIUM])
			AddPlanetBonusToBonus(tmpIridiumBoni, size_plus_one);
		if (boni[RESOURCES::FOOD])	// food
			AddPlanetBonusToBonus(tmpFoodBoni, size_plus_one);
		if (boni[7])	// energy
			AddPlanetBonusToBonus(tmpEnergyBoni, size_plus_one);
		// Menge des abgebauten Deritiums mit der Anzahl der kolonisierten Planeten mit Deritiumvorkommen
		// multiplizieren
		if (boni[RESOURCES::DERITIUM])
			deritiumProdMulti += 1;
	}
	m_Production.m_iDeritiumProd	*= deritiumProdMulti;

	AddBonusToProd(m_Production.m_iFoodProd, tmpFoodBoni);
	AddBonusToProd(m_Production.m_iIndustryProd, tmpIndustryBoni);
	AddBonusToProd(m_Production.m_iPotentialIndustryProd, tmpIndustryBoni);
	AddBonusToProd(m_Production.m_iEnergyProd, tmpEnergyBoni);
	AddBonusToProd(m_Production.m_iPotentialEnergyProd, tmpEnergyBoni);
	AddBonusToProd(m_Production.m_iSecurityProd, tmpSecurityBoni);
	AddBonusToProd(m_Production.m_iResearchProd, tmpResearchBoni);
	AddBonusToProd(m_Production.m_iTitanProd, tmpTitanBoni);
	AddBonusToProd(m_Production.m_iDeuteriumProd, tmpDeuteriumBoni);
	AddBonusToProd(m_Production.m_iDuraniumProd, tmpDuraniumBoni);
	AddBonusToProd(m_Production.m_iCrystalProd, tmpCrystalBoni);
	AddBonusToProd(m_Production.m_iIridiumProd, tmpIridiumBoni);
	AddBonusToProd(m_Production.m_iDeritiumProd, tmpDeritiumBoni);
	AddBonusToProd(m_Production.m_iCreditsProd, tmpCreditsBoni);

	// Wenn das System blockiert wird, dann verringern sich bestimmte Produktionswerte
	if (m_byBlockade > NULL)
	{
		const int bonus = -m_byBlockade;
		AddBonusToProd(m_Production.m_iIndustryProd, bonus);
		AddBonusToProd(m_Production.m_iPotentialIndustryProd, bonus);
		AddBonusToProd(m_Production.m_iSecurityProd, bonus);
		AddBonusToProd(m_Production.m_iResearchProd, bonus);
		AddBonusToProd(m_Production.m_iTitanProd, bonus);
		AddBonusToProd(m_Production.m_iDeuteriumProd, bonus);
		AddBonusToProd(m_Production.m_iDuraniumProd, bonus);
		AddBonusToProd(m_Production.m_iCrystalProd, bonus);
		AddBonusToProd(m_Production.m_iIridiumProd, bonus);
		AddBonusToProd(m_Production.m_iDeritiumProd, bonus);
		AddBonusToProd(m_Production.m_iCreditsProd, bonus);
		if (m_byBlockade >= 100)
			m_Production.m_bShipYard	= false;
	}

	// Hier noch Boni auf das Spezialzeug geben (z.B. Scanpower usw.)
	AddBonusToProd(m_Production.m_iScanPower, m_Production.m_iScanPowerBoni);
	AddBonusToProd(m_Production.m_iScanRange, m_Production.m_iScanRangeBoni);
	AddBonusToProd(m_Production.m_iGroundDefend, m_Production.m_iGroundDefendBoni);
	AddBonusToProd(m_Production.m_iShipDefend, m_Production.m_iShipDefendBoni);
	AddBonusToProd(m_Production.m_iShieldPower, m_Production.m_iShieldPowerBoni);

	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Hier die Boni durch die Uniqueforschung "Wirtschaft" -> 10% mehr Industrie
	if(const int bonus = ResearchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::ECONOMY, 1))
	{
		AddBonusToProd(m_Production.m_iIndustryProd, bonus);
		AddBonusToProd(m_Production.m_iPotentialIndustryProd, bonus);
	}
	else if(const int bonus = ResearchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::ECONOMY, 3))
		AddBonusToProd(m_Production.m_iCreditsProd, bonus);

	// Hier die Boni durch die Uniqueforschung "Produktion"
	if(const int bonus = ResearchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::PRODUCTION, 1))
		AddBonusToProd(m_Production.m_iFoodProd, bonus);
	else if (const int bonus = ResearchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::PRODUCTION, 2))
	{
		AddBonusToProd(m_Production.m_iTitanProd, bonus);
		AddBonusToProd(m_Production.m_iDeuteriumProd, bonus);
		AddBonusToProd(m_Production.m_iDuraniumProd, bonus);
		AddBonusToProd(m_Production.m_iCrystalProd, bonus);
		AddBonusToProd(m_Production.m_iIridiumProd, bonus);
	}
	// Wenn wir die Uniqueforschung "Produktion" gew�hlt haben, und dort mehr Energie haben wollen -> 20% mehr!
	else if (const int bonus = ResearchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::PRODUCTION, 3))
	{
		AddBonusToProd(m_Production.m_iEnergyProd, bonus);
		AddBonusToProd(m_Production.m_iPotentialEnergyProd, bonus);
	}

	// Hier die Boni durch die Uniqueforschung "Forschung und Sicherheit"
	if(const int bonus = ResearchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::DEVELOPMENT_AND_SECURITY, 1))
		AddBonusToProd(m_Production.m_iResearchProd, bonus);
	else if (const int bonus = ResearchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::DEVELOPMENT_AND_SECURITY, 2))
		AddBonusToProd(m_Production.m_iSecurityProd, bonus);
	else if (const int bonus = ResearchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::DEVELOPMENT_AND_SECURITY, 3))
	{
		AddBonusToProd(m_Production.m_iResearchProd, bonus);
		AddBonusToProd(m_Production.m_iSecurityProd, bonus);
	}

	// Maximalenergie, also hier noch ohne Abz�ge durch energiebed�rftige Geb�ude
	m_Production.m_iMaxEnergyProd = m_Production.m_iEnergyProd;
	// hier die gesamte Energie durch energiebed�rftige Geb�ude abziehen
	m_Production.m_iEnergyProd -= neededEnergy;

	// imperiumweite Moralprod mit aufrechnen
	m_Production.m_iMoralProd += m_Production.m_iMoralProdEmpireWide[m_sOwnerOfSystem];
	// Den Moralboni im System noch auf die einzelnen Produktionen anrechnen
	m_Production.IncludeSystemMoral(m_iMoral);
	// ben�tigte Nahrung durch Bev�lkerung von der Produktion abiehen
	if (!pOwner->HasSpecialAbility(SPECIAL_NEED_NO_FOOD))
		// ceil, wird auf Kommezahl berechnet, z.B brauchen wir f�r 14.5 Mrd. Leute 145 Nahrung und nicht 140 bzw. 150
		m_Production.m_iFoodProd -= (int)ceil(m_dHabitants*10);
	else
		m_Production.m_iFoodProd = m_Production.m_iMaxFoodProd;

	// Jetzt noch die freien Arbeiter berechnen
	m_Workers.CalculateFreeWorkers();
}

// Funktion berechnet die Lagerinhalte des Systems. Aufrufen bei Ende bzw. Beginn einer neuen Runde.
// Gibt die Funktion TRUE zur�ck hat sich das System Aufgrund zu schlechter Moral vom Besitzer losgesagt.
BOOLEAN CSystem::CalculateStorages(CResearchInfo* researchInfo, int diliAdd)
{
	// wie oben gesagt, noch �berarbeiten, wenn negatives Lager sein w�rdem dann darf das System nicht mehr wachsen
	// bzw. wird kleiner!!!
	m_Store.Food += m_Production.m_iFoodProd;
	m_Store.Titan += m_Production.m_iTitanProd;
	m_Store.Deuterium += m_Production.m_iDeuteriumProd;
	m_Store.Duranium += m_Production.m_iDuraniumProd;
	m_Store.Crystal += m_Production.m_iCrystalProd;
	m_Store.Iridium += m_Production.m_iIridiumProd;
	m_Store.Deritium += m_Production.m_iDeritiumProd + diliAdd;

	// Lagerobergrenzen
	m_Store.Cap();

	short multi = 1;
	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Hier die Boni durch die Uniqueforschung "Lager und Transport" -> doppeltes Deritiumlager
	if (researchInfo->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
		multi = researchInfo->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetBonus(1);
	if ((int)m_Store.Deritium > MAX_DERITIUM_STORE * multi)
		m_Store.Deritium = MAX_DERITIUM_STORE * multi;

	m_iMoral += (short)m_Production.m_iMoralProd;

	// Wenn wir eine Moral von -1 haben, senkt sich der Wert auf maximal noch 85
	if (m_iMoral < 85 && m_Production.m_iMoralProd == -1)
		m_iMoral = 85;
	// Wenn wir eine Moral von -2 haben, senkt sich der Wert auf maximal noch 70
	else if (m_iMoral < 70 && m_Production.m_iMoralProd == -2)
		m_iMoral = 70;

	// Moralanhebung bzw. -absenkung mal als "Lager" gesehen -> Moral, die ins Lager/System kommt
	// hier noch berechnen, das Moral von Stufen abh�ngt
	if (m_iMoral <= 30) m_iMoral -= 4;
	else if (m_iMoral <= 40) m_iMoral -= 3;
	else if (m_iMoral <= 50) m_iMoral -= 2;
	else if (m_iMoral <= 60) m_iMoral -= 1;
	else if (m_iMoral >= 190) m_iMoral -= 5;
	else if (m_iMoral >= 173) m_iMoral -= 4;
	else if (m_iMoral >= 152) m_iMoral -= 3;
	else if (m_iMoral >= 121) m_iMoral -= 2;
	else if (m_iMoral >= 110) m_iMoral -= 1;

	// ohne weiter Moralprod gehts wieder auf 100 zur�ck, wenn die Moral nicht gr��er 110 ist
	if (m_iMoral > 100 && m_iMoral <= 109 && m_Production.m_iMoralProd == 0) m_iMoral--;
	if (m_iMoral > 200) m_iMoral -= 6;
	if (m_iMoral < 0) m_iMoral = 0;
	if (m_iMoral > 200) m_iMoral = 200;

	// Jetzt �berpr�fen, ob das System sich bei einer Moral von unter 30 vom Imperium lossagt
	if (m_iMoral < 30)
	{
		// zuf�llig, nicht zwangsl�ufig sagt es sich los, erst bei einer Moral von 8
		USHORT random = rand()%(m_iMoral+1);
		if (random < 8)
		{
			m_sOwnerOfSystem = "";
			m_Production.m_iFoodProd = 10;
			m_Production.m_iIndustryProd = 5;
			m_iMoral = 65;
			if (m_Store.Food < 0)
				m_Store.Food = 0;
			return TRUE;		// Das System hat sich losgesagt
		}
	}
	return FALSE;
}

// Funktion berechnet die baubaren Schiffe in dem System.
void CSystem::CalculateBuildableShips()
{
	CBotEDoc* pDoc = resources::pDoc;
	m_BuildableShips.RemoveAll();
	m_BuildableShips.FreeExtra();
	// Hier jetzt schauen ob wir eine Werft haben und anhand der gr��e der Werft k�nnen wir bestimmte
	// Schiffstypen bauen
	CMajor* pMajor = dynamic_cast<CMajor*>(pDoc->GetRaceCtrl()->GetRace(m_sOwnerOfSystem));
	if (!pMajor)
		return;

	// exisitiert eine Werft?
	if (m_Production.GetShipYard())
	{
		// Array mit baubaren Minorraceschiffen f�llen
		int nMinorShipNumber = -1;
		if (GetMinorRace())
		{
			CMinor* pMinor = pDoc->GetRaceCtrl()->GetMinorRace(GetName());
			if (pMinor)
				nMinorShipNumber = pMinor->GetRaceShipNumber();
		}

		CResearch* pResearch = pMajor->GetEmpire()->GetResearch();
		CArray<USHORT> obsoleteClasses;

		for (int i = 0; i < pDoc->GetShipInfos()->GetSize(); i++)
		{
			CShipInfo* pShipInfo = &(pDoc->GetShipInfos()->GetAt(i));
			// Au�enposten und Sternenbasen k�nnen wir nat�rlich nicht hier bauen
			if ((pShipInfo->GetRace() == pMajor->GetRaceShipNumber() || pShipInfo->GetRace() == nMinorShipNumber)
				&& !pShipInfo->IsStation())
			{
				// Forschungsstufen checken
				BYTE researchLevels[6] =
				{
					pResearch->GetBioTech(),
					pResearch->GetEnergyTech(),
					pResearch->GetCompTech(),
					pResearch->GetPropulsionTech(),
					pResearch->GetConstructionTech(),
					pResearch->GetWeaponTech()
				};

				if (!pShipInfo->IsThisShipBuildableNow(researchLevels))
					continue;

				// Wenn durch dieses Schiff ein anderes Schiff veraltet ist (nur wenn es technologisch baubar ist)
				if (pShipInfo->GetObsoleteShipClass() != "")
				{
					for (int j = 0; j < pDoc->GetShipInfos()->GetSize(); j++)
						if (pDoc->GetShipInfos()->GetAt(j).GetShipClass() == pShipInfo->GetObsoleteShipClass())
						{
							obsoleteClasses.Add(pDoc->GetShipInfos()->GetAt(j).GetID());
							break;
						}
				}

				// Wenn das Schiff nur in einem bestimmten System gebaut werden kann, dann hier checken
				if (!pShipInfo->GetOnlyInSystem().IsEmpty())
				{
					if (pShipInfo->GetOnlyInSystem() != GetName())
						continue;
					// der Besitzer der Schiffsklasse wird auf den Besitzer des Schiffes gesetzt. Somit kann
					// eine Majorrace dann auch die Schiffe der Minorrace bauen
					else
						pShipInfo->SetRace(pMajor->GetRaceShipNumber());
				}
				else if (pShipInfo->GetRace() == nMinorShipNumber)
					continue;

				// gibt es eine Werft welche diese Schiffsgr��e bauen kann?
				if (pShipInfo->GetShipSize() > m_Production.GetMaxBuildableShipSize())
					continue;

				// wenn die Credits unter 0 gefallen sind k�nnen keine Kampfschiffe mehr gebaut werden
				if (pShipInfo->IsNonCombat() == false && pMajor->GetEmpire()->GetCredits() < 0)
					continue;

				// Schiff in die Liste der baubaren Schiffe aufnehmen
				m_BuildableShips.Add(pShipInfo->GetID());
			}
		}
		// Nochmal die jetzt baubare Schiffsliste durchgehen und schauen, ob manche Schiffe veraltet sind und somit
		// nicht mehr baubar sind
		for (int i = 0; i < obsoleteClasses.GetSize(); i++)
		{
			for (int j = 0; j < m_BuildableShips.GetSize(); j++)
				if (m_BuildableShips.GetAt(j) == obsoleteClasses.GetAt(i))
				{
					m_BuildableShips.RemoveAt(j);
					break;
				}
		}
	}
}

// Funktion berechnet die Anzahl aller Farmen, Bauh�fe usw., also alle Geb�ude die Arbeiter ben�tigen.
// Sie mu� am Rundenanfang vor CalculateVariables() aufgerufen werden und sortiert gleichzeitig das
// CArray m_Buildings nach der RunningNumber.
// In der Doc-Klasse nach der Funktion DestroyBuildings() und zu Beginn aufrufen!
void CSystem::CalculateNumberOfWorkbuildings(const BuildingInfoArray *buildingInfos)
{
	// Sortierung
	c_arraysort<CArray<CBuilding>,CBuilding>(m_Buildings,sort_asc);

	// Berechnung der Geb�ude
	m_iFoodBuildings = 0;
	m_iIndustryBuildings = 0;
	m_iEnergyBuildings = 0;
	m_iSecurityBuildings = 0;
	m_iResearchBuildings = 0;
	m_iTitanMines = 0;
	m_iDeuteriumMines = 0;
	m_iDuraniumMines = 0;
	m_iIridiumMines = 0;
	m_iCrystalMines = 0;
	for (int i = 0; i < m_Buildings.GetSize(); i++)
	{
		const CBuildingInfo* buildingInfo = &buildingInfos->GetAt(m_Buildings.GetAt(i).GetRunningNumber() - 1);

		if (buildingInfo->GetWorker() == TRUE)
		{
			if (buildingInfo->GetFoodProd() > 0)
				m_iFoodBuildings++;
			else if (buildingInfo->GetIPProd() > 0)
				m_iIndustryBuildings++;
			else if (buildingInfo->GetEnergyProd() > 0)
				m_iEnergyBuildings++;
			else if (buildingInfo->GetSPProd() > 0)
				m_iSecurityBuildings++;
			else if (buildingInfo->GetFPProd() > 0)
				m_iResearchBuildings++;
			else if (buildingInfo->GetTitanProd() > 0)
				m_iTitanMines++;
			else if (buildingInfo->GetDeuteriumProd() > 0)
				m_iDeuteriumMines++;
			else if (buildingInfo->GetDuraniumProd() > 0)
				m_iDuraniumMines++;
			else if (buildingInfo->GetCrystalProd() > 0)
				m_iCrystalMines++;
			else if (buildingInfo->GetIridiumProd() > 0)
				m_iIridiumMines++;
		}
	}
}

// Funktion berechnet die imperiumweite Moralproduktion, welche aus diesem System generiert wird.
void CSystem::CalculateEmpireWideMoralProd(const BuildingInfoArray *buildingInfos)
{
	for (int i = 0; i < m_Buildings.GetSize(); i++)
		if (m_Buildings.GetAt(i).GetIsBuildingOnline())
		{
			const CBuildingInfo *buildingInfo = &buildingInfos->GetAt(m_Buildings.GetAt(i).GetRunningNumber() - 1);
			m_Production.m_iMoralProdEmpireWide[m_sOwnerOfSystem] += buildingInfo->GetMoralProdEmpire();
		}
}

//////////////////////////////////////////////////////////////////////
// buildings
//////////////////////////////////////////////////////////////////////

BOOLEAN CSystem::CheckGeneralConditions(const CBuildingInfo* building, CGlobalBuildings* globals, CMajor* pMajor)
{
/*	Allgemeine Voraussetzungen
	--------------------------
	ben�tigte Anzahl eigener Systeme
	Nur baubar in System mit Name
	Nur wirklicher Besitzer des Geb�udes
	minimale Bev�lkerung im System
	mindst. X Geb�ude von ID im System
	max X mal pro System
	max X mal pro Imperium

	mindst. eins trifft zu von:
	Heimatsystem
	Eigene Kolonie
	Minorracesystem
	Erobertes System
*/
	// ben�tigte Anzahl eigener Systeme
	if (building->GetNeededSystems() > pMajor->GetEmpire()->CountSystems())
		return FALSE;
	// Nur baubar in System mit Name checken
	if (building->GetOnlyInSystemWithName() != "0" && building->GetOnlyInSystemWithName() != "")
		if (building->GetOnlyInSystemWithName() != GetName())
			return FALSE;
	// Nur wirklicher Besitzer des Geb�udes
	if (building->GetOnlyRace())
		if (pMajor->GetRaceBuildingNumber() != building->GetOwnerOfBuilding())
			return FALSE;
	// Minimale Bev�lkerungsanzahl checken
	if (building->GetMinHabitants() > 0)
		if (m_dHabitants < (double)building->GetMinHabitants())
			return FALSE;
	// mindst. X Geb�ude von ID im System
	if (building->GetMinInSystem().Number > 0)
	{
		USHORT number = 0;
		for (int i = 0; i < m_Buildings.GetSize(); i++)
		{
			if (m_Buildings.GetAt(i).GetRunningNumber() == building->GetMinInSystem().RunningNumber)
				number++;
			if (number >= building->GetMinInSystem().Number)
				break;
		}
		if (number < building->GetMinInSystem().Number)
			return FALSE;
	}
	// max. X Geb�ude von ID im System
	if (building->GetMaxInSystem().Number > 0)
	{
		USHORT number = 0;
		for (int i = 0; i < m_Buildings.GetSize(); i++)
		{
			if (m_Buildings.GetAt(i).GetRunningNumber() == building->GetMaxInSystem().RunningNumber)
				number++;
			if (number >= building->GetMaxInSystem().Number)
				return FALSE;
		}
	}
	// max. X Geb�ude von ID im Imperium
	if (building->GetMaxInEmpire() > 0)
	{
		int nCount = globals->GetCountGlobalBuilding(m_sOwnerOfSystem, building->GetRunningNumber());
		if (nCount >= building->GetMaxInEmpire())
			return FALSE;
	}
	// Checken ob zumindest eine der Voraussetzungen zutrifft: Heimatplanet, eigene Kolonie, Minorraceplanet oder Erobertes System
	// Zuerst Heimatplanet checken
	if (building->GetOnlyHomePlanet())
	{
		if (GetName() == pMajor->GetHomesystemName())
		{
			return TRUE;
		}
		if (building->GetOnlyOwnColony())
		{
			if (GetColonyOwner() == m_sOwnerOfSystem && GetName() != pMajor->GetHomesystemName())
				return TRUE;
		}
		if (building->GetOnlyMinorRace())
		{
			if (GetMinorRace() == TRUE)
				return TRUE;
		}
		if (building->GetOnlyTakenSystem())
		{
			if (GetTakenSector() == TRUE)
				return TRUE;
		}
		return FALSE;
	}
	// Zuerst eigene Kolonie checken
	if (building->GetOnlyOwnColony())
	{
		if (GetColonyOwner() == m_sOwnerOfSystem && GetName() != pMajor->GetHomesystemName())
		{
			return TRUE;
		}
		if (building->GetOnlyHomePlanet())
		{
			if (GetName() == pMajor->GetHomesystemName())
				return TRUE;
		}
		if (building->GetOnlyMinorRace())
		{
			if (GetMinorRace() == TRUE)
				return TRUE;
		}
		if (building->GetOnlyTakenSystem())
		{
			if (GetTakenSector() == TRUE)
				return TRUE;
		}
		return FALSE;
	}
	// Zuerst Minorraceplanet checken
	if (building->GetOnlyMinorRace())
	{
		if (GetMinorRace() == TRUE)
		{
			return TRUE;
		}
		if (building->GetOnlyHomePlanet())
		{
			if (GetName() == pMajor->GetHomesystemName())
				return TRUE;
		}
		if (building->GetOnlyOwnColony())
		{
			if (GetColonyOwner() == m_sOwnerOfSystem && GetName() != pMajor->GetHomesystemName())
				return TRUE;
		}
		if (building->GetOnlyTakenSystem())
		{
			if (GetTakenSector() == TRUE)
				return TRUE;
		}
		return FALSE;
	}
	// Zuerst erobertes System checken
	if (building->GetOnlyTakenSystem())
	{
		if (GetTakenSector() == TRUE)
		{
			return TRUE;
		}
		if (building->GetOnlyHomePlanet())
		{
			if (GetName() == pMajor->GetHomesystemName())
				return TRUE;
		}
		if (building->GetOnlyOwnColony())
		{
			if (GetColonyOwner() == m_sOwnerOfSystem && GetName() != pMajor->GetHomesystemName())
				return TRUE;
		}
		if (building->GetOnlyMinorRace())
		{
			if (GetMinorRace() == TRUE)
				return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

// Diese private Hilfsfunktion �berpr�ft, ob es einen Nachfolger zu unserem Geb�ude in der Liste der baubaren
// Geb�ude (-> flag == 0) oder als stehendes Geb�ude (-> flag == 1) im System schon steht. Die Funktion
// gibt TRUE zur�ck, wenn wir einen Nachfolger gefunden haben, ansonsten FALSE.
BOOLEAN CSystem::CheckFollower(const BuildingInfoArray* buildings, USHORT id, BOOLEAN flag, BOOLEAN equivalence)
{
	// Checken ob ein potentieller Nachfolger in der Liste der baubaren Geb�ude vorkommt
	if (flag == 0)
	{
		/*	zum Algorithmus:
			Ich durchsuche jede ID im Feld der baubaren Geb�ude nach potentiellen Vorg�ngern. Ist dann einer
			der Vorg�nger meine ID, dann k�nnen wir abbrechen.
		*/
		for (int i = 0; i < m_BuildableWithoutAssemblylistCheck.GetSize(); i++)
		{
			// Weil zu diesem Zeitpunkt auch Geb�udeupgrades (negative ID) in diesem Feld vorkommen k�nnen, darf ich
			// nur die echten Geb�ude betrachten
			if (m_BuildableWithoutAssemblylistCheck.GetAt(i) > 0)
			{
				// Vorg�ngerID eines Geb�udes aus der Liste der baubaren Geb�ude holen
				USHORT pre = buildings->GetAt(m_BuildableWithoutAssemblylistCheck.GetAt(i)-1).GetPredecessorID();
				// Haben wir jetzt schon den Vorg�nger gefunden (eher unwahrscheinlich)
				if (pre == id)
					return TRUE;
				// jetzt in die Schleife um sich immer neue ID�s zu holen
				do {
					// Vorg�ngerID des Vorg�ngers holen
					if (pre > 0)
					{
						USHORT oldPre = pre;
						pre = buildings->GetAt(pre-1).GetPredecessorID();
						if (oldPre == pre)
						{
							CString s;
							s.Format("ERROR with Buildings Predecessor ID:\nBuilding: %s\n\nID: %d\nit is the same like the ID of the building", buildings->GetAt(oldPre-1).GetBuildingName(), oldPre);
							AfxMessageBox(s);
							break;
						}
					}
				} while (pre != id && pre != 0);// mache das solange wir keinen Vorg�nger gefunden haben oder es keinen Vorg�nger gibt
				// Gibt es einen Vorg�nger
				if (pre == id)
					return TRUE;

				// So, sind wir jetzt noch dabei und haben in dem System auch Geb�ude von anderen Rassen stehen (also der
				// Parameter equivalence == TRUE), dannn m�ssen wir auch noch bei dem �quivalenzgeb�ude nach Vorg�ngern suchen
				if (equivalence == TRUE)
				{
					// Vorg�ngerID des entsprechenden �quivalenzgeb�udes holen
					USHORT race = buildings->GetAt(id-1).GetOwnerOfBuilding();
					//USHORT race = buildings->GetAt(m_BuildableWithoutAssemblylistCheck.GetAt(i)-1).GetOwnerOfBuilding();
					USHORT equi = buildings->GetAt(m_BuildableWithoutAssemblylistCheck.GetAt(i)-1).GetEquivalent(race);
/*					if (equi > 0)
					{
						pre = buildings->GetAt(equi-1).GetPredecessorID();	// pre ist Vorg�nger des �quivalenzgeb�udes
						// Jetzt wie oben die do - while Schleife durchlaufen
						do {
							// Vorg�ngerID des "Vorg�ngers holen
							if (pre > 0)
								pre = buildings->GetAt(pre-1).GetPredecessorID();
						} while (pre != equi && pre != 0);// mache das solange wir keinen Vorg�nger gefunden haben oder es keinen Vorg�nger gibt
						// Gibt es einen Vorg�nger
						if (pre == equi)
							return TRUE;
					}*/
					if (equi == id)
						return TRUE;
					pre = buildings->GetAt(m_BuildableWithoutAssemblylistCheck.GetAt(i)-1).GetPredecessorID();
					// Jetzt wir oben die do - while Schleife durchlaufen
					do {
						// Vorg�ngerID des "Vorg�ngers holen
						if (pre > 0)
						{
							USHORT oldPre = pre;
							equi = buildings->GetAt(pre-1).GetEquivalent(race);
							pre = buildings->GetAt(pre-1).GetPredecessorID();
							if (oldPre == pre)
							{
								CString s;
								s.Format("ERROR with Buildings Predecessor ID:\nBuilding: %s\n\nID: %d\nit is the same like the ID of the building", buildings->GetAt(oldPre-1).GetBuildingName(), oldPre);
								AfxMessageBox(s);
								break;
							}
						}
					} while (id != equi && pre != 0); // mache das solange wir keinen Vorg�nger gefunden haben oder es keinen Vorg�nger gibt
					// Gibt es einen Vorg�nger
					if (equi == id)
						return TRUE;
				}
			}
		}
	}
	// Checken ob ein potentieller Nachfolger schon im System steht. Normalerweise ist dieser schon in der Liste
	// der baubaren Geb�ude, aber z.B. Geb�ude die nur einmal pro System gebaut werden k�nnen sind nicht in dieser Liste.
	// z.B. wenn wir eine geupdatede Werft einer anderen Rasse in dem System stehen haben, darf ich nicht einen
	// potentiellen �quivalenten Vorg�nger dieser Werft unsererseits bauen.
	else
	{
		USHORT minID = 0;
		// Der Algorithmus ist wie der obere. Nur wird hier das Feld der stehenden Geb�ude im System durchlaufen
		for (int i = 0; i < m_Buildings.GetSize(); i++)
		{
			// Da das Geb�udefeld nach der RunningNumber geordnet ist, brauchen wir erst wieder checken
			// wenn wir ein Geb�ude mit einer neune RunningNumber haben
			if (m_Buildings.GetAt(i).GetRunningNumber() > minID)
				minID = m_Buildings.GetAt(i).GetRunningNumber();
			{
				// Vorg�ngerID eines Geb�udes aus der Liste der stehenden Geb�ude holen
				USHORT pre = buildings->GetAt(m_Buildings.GetAt(i).GetRunningNumber()-1).GetPredecessorID();
				// Haben wir jetzt schon den Vorg�nger gefunden (eher unwahrscheinlich)
				if (pre == id)
					return TRUE;
				// jetzt in die Schleife um sich immer neue ID�s zu holen
				do {
					// Vorg�ngerID des "Vorg�ngers holen
					if (pre > 0)
					{
						USHORT oldPre = pre;
						pre = buildings->GetAt(pre-1).GetPredecessorID();
						if (oldPre == pre)
						{
							CString s;
							s.Format("ERROR with Buildings Predecessor ID:\nBuilding: %s\n\nID: %d\nit is the same like the ID of the building", buildings->GetAt(oldPre-1).GetBuildingName(), oldPre);
							AfxMessageBox(s);
							break;
						}
					}
				} while (pre != id && pre != 0);// mache das solange wir keinen Vorg�nger gefunden haben oder es keinen Vorg�nger gibt
				// Gibt es einen Vorg�nger
				if (pre == id)
					return TRUE;

				// So, sind wir jetzt noch dabei und haben in dem System auch Geb�ude von anderen Rassen stehen (also der
				// Parameter equivalence == TRUE), dannn m�ssen wir auch noch bei dem �quivalenzgeb�ude nach Vorg�ngern suchen
				if (equivalence == TRUE)
				{
					// Vorg�ngerID des entsprechenden �quivalenzgeb�udes holen
					USHORT race = buildings->GetAt(id-1).GetOwnerOfBuilding();
					USHORT equi = buildings->GetAt(m_Buildings.GetAt(i).GetRunningNumber()-1).GetEquivalent(race);
					if (equi == id)
						return TRUE;
					pre = buildings->GetAt(m_Buildings.GetAt(i).GetRunningNumber()-1).GetPredecessorID();
					// Jetzt wir oben die do - while Schleife durchlaufen
					do {
						// Vorg�ngerID des "Vorg�ngers holen
						if (pre > 0)
						{
							USHORT oldPre = pre;
							equi = buildings->GetAt(pre-1).GetEquivalent(race);
							pre = buildings->GetAt(pre-1).GetPredecessorID();
							if (oldPre == pre)
							{
								CString s;
								s.Format("ERROR with Buildings Predecessor ID:\nBuilding: %s\n\nID: %d\nit is the same like the ID of the building", buildings->GetAt(oldPre-1).GetBuildingName(), oldPre);
								AfxMessageBox(s);
								break;
							}
						}
					} while (id != equi && pre != 0);// mache das solange wir keinen Vorg�nger gefunden haben oder es keinen Vorg�nger gibt
					// Gibt es einen Vorg�nger
					if (id == equi)
						return TRUE;
				}
			}
		}
	}
	return FALSE;
}

namespace //helpers for CalculateBuildableBuildings()
{
	BOOLEAN CheckTech(const CBuildingInfo* building, CResearch* research)
	{
		// n�tige Forschungsstufen checken
		if (research->GetBioTech() <  building->GetBioTech())
			return 0;
		if (research->GetEnergyTech() < building->GetEnergyTech())
			return 0;
		if (research->GetCompTech() < building->GetCompTech())
			return 0;
		if (research->GetPropulsionTech() < building->GetPropulsionTech())
			return 0;
		if (research->GetConstructionTech() < building->GetConstructionTech())
			return 0;
		if (research->GetWeaponTech() < building->GetWeaponTech())
			return 0;
		return 1;
	}

	BOOLEAN CheckPlanet(const CBuildingInfo* building, CSector* sector)
	{
		BOOLEAN Ok = FALSE;
		BOOLEAN deritium = FALSE;
		int number = sector->GetNumberOfPlanets();
		CPlanet planet;
		for (int i = 0; i < number && Ok == FALSE; i++)
		{
			planet = *sector->GetPlanet(i);
			if (planet.GetColonized())
			{
				//{M,O,L,P,H,Q,K,G,R,F,C,N,A,B,E,Y,I,J,S,T}
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_A) == 1 && planet.GetClass() == 'A' && Ok == FALSE)	// nach A suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_B) == 1 && planet.GetClass() == 'B' && Ok == FALSE)	// nach B suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_C) == 1 && planet.GetClass() == 'C' && Ok == FALSE)	// nach C suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_E) == 1 && planet.GetClass() == 'E' && Ok == FALSE)	// nach E suchen
					Ok = TRUE;	// D fehlt wieder, weil ja Asteroid
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_F) == 1 && planet.GetClass() == 'F' && Ok == FALSE)	// nach F suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_G) == 1 && planet.GetClass() == 'G' && Ok == FALSE)	// nach G suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_H) == 1 && planet.GetClass() == 'H' && Ok == FALSE)	// nach H suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_I) == 1 && planet.GetClass() == 'I' && Ok == FALSE)	// nach I suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_J) == 1 && planet.GetClass() == 'J' && Ok == FALSE)	// nach J suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_K) == 1 && planet.GetClass() == 'K' && Ok == FALSE)	// nach K suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_L) == 1 && planet.GetClass() == 'L' && Ok == FALSE)	// nach L suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_M) == 1 && planet.GetClass() == 'M' && Ok == FALSE)	// nach M suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_N) == 1 && planet.GetClass() == 'N' && Ok == FALSE)	// nach N suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_O) == 1 && planet.GetClass() == 'O' && Ok == FALSE)	// nach O suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_P) == 1 && planet.GetClass() == 'P' && Ok == FALSE)	// nach P suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_Q) == 1 && planet.GetClass() == 'Q' && Ok == FALSE)	// nach Q suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_R) == 1 && planet.GetClass() == 'R' && Ok == FALSE)	// nach R suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_S) == 1 && planet.GetClass() == 'S' && Ok == FALSE)	// nach S suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_T) == 1 && planet.GetClass() == 'T' && Ok == FALSE)	// nach T suchen
					Ok = TRUE;
				if (building->GetPlanetTypes(PLANET_CLASSES::PLANETCLASS_Y) == 1 && planet.GetClass() == 'Y' && Ok == FALSE)	// nach Y suchen
					Ok = TRUE;
			}
		}

		// Wenn das Geb�ude Deritium produziert, so muss auf einem kolonisierten Planeten auch Deritium vorhanden sein
		if (building->GetDeritiumProd() > 0)
		{
			for (int i = 0; i < sector->GetNumberOfPlanets(); i++)
				if (sector->GetPlanet(i)->GetColonized())
					deritium |= sector->GetPlanet(i)->GetBoni()[RESOURCES::DERITIUM];
			if (!deritium || !Ok)
				return 0;
		}
		if (Ok == FALSE)	// Haben also kein Planet im System gefunden, auf dem wir das Geb�ude bauen k�nnten
			return 0;
		else
			return 1;
	}
}

// Funktion berechnet die baubaren Geb�ude und Geb�udeupdates in dem System.
void CSystem::CalculateBuildableBuildings(CGlobalBuildings* globals)
{
	const BuildingInfoArray* buildingInfo = resources::BuildingInfo;
	CMajor* pMajor = dynamic_cast<CMajor*>(resources::pDoc->GetRaceCtrl()->GetRace(m_sOwnerOfSystem));

	CEmpire* empire = pMajor->GetEmpire();
	ASSERT(empire);

	m_BuildableWithoutAssemblylistCheck.RemoveAll();
	m_BuildableWithoutAssemblylistCheck.FreeExtra();
	m_BuildableBuildings.RemoveAll();
/*
	Algorithmus:

	1. schon stehende Geb�ude und Geb�ude in der Liste der immer baubaren Geb�ude
	   durchgehen und stehende Geb�ude gleichzeitig nach Voraussetzungen pr�fen
		1.1 - JA)   k�nnen dieses bauen und Punkt (0.) durchf�hren
		1.2 - NEIN) k�nnen es nicht bauen

	2. Geb�udeinfos durchgehen, nach allg. Voraussetzungen checken und pr�fen, dass
	   diese ID oder ein �quivalent NICHT schon in der Liste der baubaren Geb�ude sind
		2.1 - JA)   Wenn Geb�ude eine ID des Vorg�ngers besitzt
			2.1.1 - JA)   Schauen ob dieses Vorg�ngergeb�ude im System steht
				2.1.1.1 - JA)	Geb�udeupgrade in die Liste nehmen
				2.1.1.2 - NEIN) k�nnen es nicht bauen
			2.1.2 - NEIN) �berpr�fen ob ein Nachfolger des Geb�udes in der Liste der baubaren Geb�ude steht (*)
				2.1.2.1 - JA)	k�nnen es nicht bauen
				2.1.2.2 - NEIN) �berpr�fen ob ein Nachfolger des Geb�udes in dem System steht (*)
					2.1.2.2.1 - JA)	  k�nnen es nicht bauen
					2.1.2.2.1 - NEIN) k�nnen es bauen
		2.2 - NEIN) k�nnen es nicht bauen

	(*) -> nur wenn dieses Geb�ude auch updatebar ist!!! Ansonsten k�nnen wir es bauen

	3. zus�tzlichen Baulistencheck durchf�hren, d.h wenn es z.B. nur einmal pro Imperium
	   baubar ist, dann darf es sich auch in keiner Bauliste befinden. Auch Updates im System
	   d�rfen nicht schon in der Bauliste stehen.
		3.1 - JA)   Auftrag bleibt baubar
		3.2 - NEIN) Auftrag ist nicht mehr baubar

	0. mindst. baubare Geb�ude durchgehen und checken ob unsere ID in der Liste vorkommt
	0.1 - JA)   nichts weiter machen
	0.2 - NEIN) m�glichen Vorg�nger unseres Geb�udes in der Liste suchen. Gefunden?
		0.2.1 - JA)   Vorg�nger ID mit unserer ID ersetzen
		0.2.2 - NEIN) Unserer ID in die Liste einschreiben

	// gut w�re eine Variable, die alle ID�s speichert, die wir vor dem Baulistencheck bauen k�nnen.
	// Wenn wir dann einen Baulistencheck machen, bleibt diese Variable davon unber�hrt. Nur die Variablen,
	// in denen die nun baubaren Auftr�ge stehen werden ver�ndert. Wenn ich einen Auftrag in die Assemblylist
	// nehme, wird der Baulistencheck ausgef�hrt. Auch wenn ich diesen wieder aus der Bauliste nehme wird der
	// Baulistencheck ausgef�hrt. Nun k�nnte man noch einbauen, das dieser nicht gleich f�r jedes Geb�ude
	// gemacht wird, sondern nur bei denen, bei denen es notwendig ist.
*/
// ------------------------------------ 1 -------------------------------------------- //
	// schon stehende Geb�ude checken
	USHORT minID = 0;
	BOOLEAN equivalents = FALSE;	// steht in dem System ein Geb�ude einer anderen Rasse (eins was wir nicht h�tten bauen k�nnen)

	for (int i = 0; i < m_Buildings.GetSize(); i++)
	{
		// Wenn wir auf das Geb�ude in der Infoliste zugreifen wollen, dann steht dieses dort immer
		// an der Stelle ID-1, weil das Feld bei 0 beginnt und unsere ID's aber bei 1 anfangen!!!
		USHORT id = m_Buildings.GetAt(i).GetRunningNumber();
		BOOLEAN found = FALSE;
		// Da das Feld der Geb�ude im System aufsteigend nach der RunningNumber sortiert ist, brauchen wir immer erst
		// checken, wenn wir eine neue RunningNumber haben
		if (id > minID)
		{
			// steht hier ein Geb�ude einer anderen Rasse?
			if (buildingInfo->GetAt(id-1).GetOwnerOfBuilding() != pMajor->GetRaceBuildingNumber())
				equivalents = TRUE;
			minID = id;
			// Checken das das Geb�ude nicht schon in der Liste der baubaren Geb�ude vorkommt
			for (int k = 0; k < m_BuildableWithoutAssemblylistCheck.GetSize(); k++)
				if (m_BuildableWithoutAssemblylistCheck.GetAt(k) == id)
				{
					found = TRUE;
					break;
				}
			// Checken ob wir es durch die vorhanden Planeten bauen k�nnen
			if (found == FALSE && CheckPlanet(&buildingInfo->GetAt(id-1),this))
				// allg. Voraussetzungen des Geb�udes checken
				if (CheckGeneralConditions(&buildingInfo->GetAt(id-1),globals,pMajor))
				{
					// Wenn wir hier angekommen sind, dann k�nnten wir das Geb�ude bauen.
					// Ist es ein Geb�ude, welches wir manchmal mindst immer bauen k�nnen (alle Geb�ude, die Arbeiter
					// ben�tigen!!!), dann m�ssen wir noch Punkt (0.) durchf�hren.
					BOOLEAN id_in_list = FALSE;
					const CBuildingInfo* info = &buildingInfo->GetAt(m_Buildings.GetAt(i).GetRunningNumber() - 1);

					if (info->GetWorker())
					{
						for (int j = 0; j < m_AllwaysBuildableBuildings.GetSize(); j++)
							if (id == m_AllwaysBuildableBuildings.GetAt(j))
							{
								id_in_list = TRUE;	// gefunden -> k�nnen abbrechen
								break;
							}
						// Wir haben unseres Geb�ude nicht schon in der Liste der immer baubaren Geb�ude gefunden,
						// also mit Punkt (0.2) fortfahren!
						if (id_in_list == FALSE && buildingInfo->GetAt(id-1).GetPredecessorID() != 0)
						{
							BOOLEAN found_predecessor = FALSE;
							// Vorg�nger unseres Geb�udes finden (es werden noch nicht alle m�glichen Vorg�nger gesucht!!!)
							for (int j = 0; j < m_AllwaysBuildableBuildings.GetSize(); j++)
								if (m_AllwaysBuildableBuildings.GetAt(j) == buildingInfo->GetAt(id-1).GetPredecessorID())
								{
									m_AllwaysBuildableBuildings.SetAt(j, id);
									found_predecessor = TRUE;
									break;
								}
							// Haben wir keinen Vorg�nger gefunden, so f�gen wir dieses Geb�ude ein
							if (found_predecessor == FALSE)
								m_AllwaysBuildableBuildings.Add(id);
						}
					}
					// Nachdem wir Punkt (0.) durchgef�hrt haben f�gen wir unser jetziges Geb�ude noch in die Liste
					// der baubaren Geb�ude und Updates ein (ohne Baulistencheck)
					m_BuildableWithoutAssemblylistCheck.Add(id);
				}
		}
	}

	// Hier noch die Geb�ude aus der immer baubaren Geb�udeliste in die Liste der baubaren Geb�ude hinzuf�gen,
	// wenn sie nicht schon drin stehen
	USHORT size = m_BuildableWithoutAssemblylistCheck.GetSize();
	for (int i = 0; i < m_AllwaysBuildableBuildings.GetSize(); i++)
	{
		// Gibt es in der Liste der immer baubaren Geb�ude ein Geb�ude einer anderen Rasse?
		if (buildingInfo->GetAt(m_AllwaysBuildableBuildings.GetAt(i)-1).GetOwnerOfBuilding() != pMajor->GetRaceBuildingNumber())
			equivalents = TRUE;
		BOOLEAN found = FALSE;
		for (int j = 0; j < size; j++)
			if (m_AllwaysBuildableBuildings.GetAt(i) == m_BuildableWithoutAssemblylistCheck.GetAt(j))
			{
				found = TRUE;
				break;
			}
		if (found == FALSE)
			m_BuildableWithoutAssemblylistCheck.Add(m_AllwaysBuildableBuildings.GetAt(i));
	}

// ------------------------------------ 2 -------------------------------------------- //
	// alle anderen Geb�udeinfos durchgehen
	for (int i = 0; i < buildingInfo->GetSize(); i++)
	{
		// �berpr�fen, dass dieses Geb�ude nicht schon in der Liste der
		// baubaren Geb�ude und Updates vorhanden ist
		BOOLEAN found = FALSE;
		for (int j = 0; j < m_BuildableWithoutAssemblylistCheck.GetSize(); j++)
		{
			// schauen ob es in der Liste der baubaren Geb�ude ist
			if (m_BuildableWithoutAssemblylistCheck.GetAt(j) == buildingInfo->GetAt(i).GetRunningNumber())
			{
				found = TRUE;	// ist schon in der Liste der baubaren Geb�ude
				break;
			}
		}

		// Geb�ude ist ein Never-Ready-Geb�ude und produziert Moral, dann kann es erst gebaut werden, wenn
		// die Moral 2.5 x unter der Produktion des Geb�udes liegt
		if (buildingInfo->GetAt(i).GetNeverReady() && buildingInfo->GetAt(i).GetMoralProd() > 0)
			if (m_iMoral > 100 - buildingInfo->GetAt(i).GetMoralProd() * 2.5)
				continue;

		// Haben wir noch nicht dieses Geb�ude in der Liste gefunden, so k�nnten wir es vielleicht bauen, deswegen
		// jetzt die ganzen Voraussetzungen pr�fen
		if (found == FALSE && CheckTech(&buildingInfo->GetAt(i), empire->GetResearch()) == TRUE)
			// Checken ob wir es durch die vorhanden Planeten bauen k�nnen
			if (CheckPlanet(&buildingInfo->GetAt(i),this))
				// allg. Voraussetzungen des Geb�udes checken
				if (CheckGeneralConditions(&buildingInfo->GetAt(i),globals,pMajor))
				{
					// Wenn wir hier sind sind wir mittlerweile schon bei Punkt (2.1) angekommen
					// �berpr�fen ob das Geb�ude einen Vorg�nger besitzt
					if (buildingInfo->GetAt(i).GetPredecessorID() != 0)	// Punkt 2.1.1
					{
						BOOLEAN found_predecessor = FALSE;
						// Schauen ob ein Geb�ude mit dieser ID im System schon steht
						for (int k = 0; k < m_Buildings.GetSize(); k++)
							if (m_Buildings.GetAt(k).GetRunningNumber() == buildingInfo->GetAt(i).GetPredecessorID())
							{
								found_predecessor = TRUE;
								break;
							}
						// Haben wir einen Vorg�nger des Geb�udes im System stehen
						if (found_predecessor == TRUE)	// Punkt 2.1.1.1
						{
							// Geb�udeupgrade in die Liste nehmen
							m_BuildableWithoutAssemblylistCheck.Add(buildingInfo->GetAt(i).GetRunningNumber()*(-1));
						}
						// ansonsten haben wir keinen Vorg�nger des Geb�udes im System stehen // Punkt 2.1.1.2
					}
					// Geb�ude hat keinen Vorg�nger
					// es geh�rt zu einer Rasse oder kann von jeder Rasse gebaut werden -> Sprich 0
					else if (buildingInfo->GetAt(i).GetOwnerOfBuilding() == 0 || buildingInfo->GetAt(i).GetOwnerOfBuilding() == pMajor->GetRaceBuildingNumber())	// Punkt 2.1.2
					{
						// schauen das sich nicht schon ein �quivalent in der Liste befindet, aber nur wenn das Geb�ude eine andere Rasse bauen
						// kann oder wenn es jeder bauen kann
						if (equivalents == TRUE)
						{
							for (int j = 0; j < m_BuildableWithoutAssemblylistCheck.GetSize(); j++)
							{
								short tid = m_BuildableWithoutAssemblylistCheck.GetAt(j);
								// Nur wenn es sich hierbei nicht um ein Upgrade handelt!
								if (tid > 0)
									// Nur die �quivalenz abfragen, zu der das Geb�ude "tid" geh�rt
									if (buildingInfo->GetAt(i).GetEquivalent(buildingInfo->GetAt(tid-1).GetOwnerOfBuilding()) == tid)
									{
										found = TRUE;
										break;
									}
							}
							// Schauen, das sich nicht schon ein �quivalent bei den stehenden Geb�uden befindet
							if (found == FALSE)
							{
								minID = 0;
								for (int j = 0; j < m_Buildings.GetSize(); j++)
								{
									USHORT tid = m_Buildings.GetAt(j).GetRunningNumber();
									if (tid > minID)	// Weil geordnetes Feld
										if (buildingInfo->GetAt(i).GetEquivalent(buildingInfo->GetAt(tid-1).GetOwnerOfBuilding()) == tid)
										{
											found = TRUE;
											break;
										}
								}
							}
						}
						// Gibt es schon ein �quivalenzgeb�ude in dem System, so k�nnen wir die nicht bauen
						if (found == TRUE)
							continue;

						// Nachfolgerchecks brauchen wir nur machen, wenn das Geb�ude auch updatebar ist!
						if (buildingInfo->GetAt(i).GetUpgradeable())
						{
							// �berpr�fen ob ein Nachfolger des Geb�udes in der Liste der baubaren Geb�ude steht
							if (CheckFollower(buildingInfo,buildingInfo->GetAt(i).GetRunningNumber(),0,equivalents) == FALSE)
								// �berpr�fen ob ein Nachfolger des Geb�udes schon im System steht
								if (CheckFollower(buildingInfo,buildingInfo->GetAt(i).GetRunningNumber(),1,equivalents) == FALSE)
									m_BuildableWithoutAssemblylistCheck.Add(buildingInfo->GetAt(i).GetRunningNumber());
						}
						// Ansonsten k�nnen wir es bauen.
						else
							m_BuildableWithoutAssemblylistCheck.Add(buildingInfo->GetAt(i).GetRunningNumber());
					}
				}
	}

	// Baulistencheck noch durchf�hren und fertig sind wir
	AssemblyListCheck(buildingInfo,globals);
	// Sortierung in der Reihenfolge nach ihrer ID
	c_arraysort<CArray<short,short>,short>(m_BuildableBuildings,sort_asc);
}

// Funktion l�scht alle Geb�ude, die die �bergebene RunningNumber haben und gibt deren Anzahl zur�ck.
// -> Danach mu� AddBuilding() mit dem Nachfolger gleich der Anzahl aufgerufen werden.
int CSystem::UpdateBuildings(int nRunningNumber, int nNeededEnergy)
{
	int nCount = 0;
	for (int i = 0; i < m_Buildings.GetSize(); i++)
		if (m_Buildings.GetAt(i).GetRunningNumber() == nRunningNumber)
		{
			// Wenn das Geb�ude online war, dann Energie freigeben
			if (nNeededEnergy > 0 && m_Buildings.GetAt(i).GetIsBuildingOnline())
				m_Production.m_iEnergyProd += nNeededEnergy;

			// Geb�ude aus der Liste l�schen
			m_Buildings.RemoveAt(i--);
			nCount++;
		}

	return nCount;
}

void CSystem::Colonize(const CShips& ship, CMajor& major)
{
	const CString shipowner = ship.GetOwnerOfShip();
	CEmpire* empire = major.GetEmpire();
	// Geb�ude bauen, wenn wir das System zum ersten Mal kolonisieren,
	// sprich das System noch niemanden geh�rt
	if (GetOwnerOfSystem().IsEmpty())
	{
		// Sector- und Systemwerte �ndern
		SetOwned(TRUE);
		m_sOwnerOfSector = shipowner;
		m_sColonyOwner = shipowner;
		SetOwnerOfSystem(shipowner);
		// Geb�ude nach einer Kolonisierung bauen
		BuildBuildingsAfterColonization(resources::BuildingInfo,ship.GetColonizePoints());
		// Nachricht an das Imperium senden, das ein System neu kolonisiert wurde
		CString s = CLoc::GetString("FOUND_COLONY_MESSAGE",FALSE,GetName());
		CEmpireNews message;
		message.CreateNews(s,EMPIRE_NEWS_TYPE::SOMETHING,GetName(),m_KO);
		empire->AddMsg(message);

		// zus�tzliche Eventnachricht (Colonize a system #12) wegen der Moral an das Imperium
		message.CreateNews(major.GetMoralObserver()->AddEvent(12, major.GetRaceMoralNumber(), GetName()), EMPIRE_NEWS_TYPE::SOMETHING, "", m_KO);
		empire->AddMsg(message);
		if (major.IsHumanPlayer())
		{
			resources::pClientWorker->AddSoundMessage(SNDMGR_MSG_CLAIMSYSTEM, major , 0);
			resources::pClientWorker->SetToEmpireViewFor(major);
			CEventColonization* eventScreen = new CEventColonization(major.GetRaceID(), CLoc::GetString("COLOEVENT_HEADLINE", FALSE, GetName()), CLoc::GetString("COLOEVENT_TEXT_" + major.GetRaceID(), FALSE, GetName()));
			empire->GetEvents()->Add(eventScreen);
			s.Format("Added Colonization-Eventscreen for Race %s in System %s", major.GetRaceName(), GetName());
			MYTRACE("general")(MT::LEVEL_INFO, s);
		}
	}
	else
	{
		// Nachricht an das Imperium senden, das ein Planet kolonisiert wurde
		CString s = CLoc::GetString("NEW_PLANET_COLONIZED",FALSE,GetName());
		CEmpireNews message;
		message.CreateNews(s,EMPIRE_NEWS_TYPE::SOMETHING,GetName(),m_KO);
		empire->AddMsg(message);
		resources::pClientWorker->SetToEmpireViewFor(major);
	}
	SetHabitants(GetCurrentHabitants());

	CalculateNumberOfWorkbuildings(resources::BuildingInfo);
	CalculateVariables();
}

// Funktion berechnet und baut die Startgeb�ude in einem System, nachdem wir einen Planeten
// in diesem kolonisiert haben.
void CSystem::BuildBuildingsAfterColonization(const BuildingInfoArray *buildingInfo, USHORT colonizationPoints)
{
	CBotEDoc* pDoc = resources::pDoc;
	ASSERT(pDoc);

	CMajor* pMajor = dynamic_cast<CMajor*>(pDoc->GetRaceCtrl()->GetRace(GetOwnerOfSector()));
	ASSERT(pMajor);

	// alle Geb�ude die wir nach Systemeroberung nicht haben d�rfen werden aus der Liste der aktuellen Geb�ude entfernt
	RemoveSpecialRaceBuildings(buildingInfo);

	BYTE byRaceBuildingID = pMajor->GetRaceBuildingNumber();

	BYTE researchLevels[6] =
		{
			pMajor->GetEmpire()->GetResearch()->GetBioTech(),
			pMajor->GetEmpire()->GetResearch()->GetEnergyTech(),
			pMajor->GetEmpire()->GetResearch()->GetCompTech(),
			pMajor->GetEmpire()->GetResearch()->GetPropulsionTech(),
			pMajor->GetEmpire()->GetResearch()->GetConstructionTech(),
			pMajor->GetEmpire()->GetResearch()->GetWeaponTech()
		};

	// in exist[.] steht dann, ob wir einen Rohstoff abbauen k�nnen, wenn ja, dann k�nnen wir auch das Geb�ude bauen
	BOOLEAN exist[RESOURCES::DERITIUM + 1] = {0};
	GetAvailableResources(exist, true);

	USHORT start = 0;
	for (int i = 0; i < buildingInfo->GetSize(); i++)
		if (buildingInfo->GetAt(i).GetOwnerOfBuilding() == byRaceBuildingID)
		{
			start = i;
			break;
		}

	short counter[10] = {0};
	USHORT runningNumber[10] = {0,0,0,0,0,0,0,0,0,0};
	for (int i = start; i < buildingInfo->GetSize(); i++)
	{
		// Abbruchbedingung, wenn das Geb�ude eh nicht mehr zu den Geb�uden des Sektorbesitzers geh�ren.
		// Daf�r muss aber die Geb�udeliste geordnet nach den Besitzern vorliegen (das sie aktuell auch ist)
		if (buildingInfo->GetAt(i).GetOwnerOfBuilding() != byRaceBuildingID)
			break;
		// wenn das Geb�ude nicht gebaut werden kann, dann mit dem n�chsten weitermachen
		if (!buildingInfo->GetAt(i).IsBuildingBuildableNow(researchLevels))
			continue;

		if (buildingInfo->GetAt(i).GetFoodProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[0] < colonizationPoints)
		{
			counter[0]++;
			runningNumber[0] = buildingInfo->GetAt(i).GetRunningNumber();

		}
		else if (buildingInfo->GetAt(i).GetIPProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[1] < colonizationPoints)
		{
			counter[1]++;
			runningNumber[1] = buildingInfo->GetAt(i).GetRunningNumber();
		}
		else if (buildingInfo->GetAt(i).GetEnergyProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[2] < colonizationPoints)
		{
			counter[2]++;
			runningNumber[2] = buildingInfo->GetAt(i).GetRunningNumber();
		}
		else if (buildingInfo->GetAt(i).GetSPProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[3] < colonizationPoints)
		{
			counter[3]++;
			runningNumber[3] = buildingInfo->GetAt(i).GetRunningNumber();
		}
		else if (buildingInfo->GetAt(i).GetFPProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[4] < colonizationPoints)
		{
			counter[4]++;
			runningNumber[4] = buildingInfo->GetAt(i).GetRunningNumber();
		}
		else if (buildingInfo->GetAt(i).GetTitanProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[5] < colonizationPoints && exist[0] == TRUE)
		{
			counter[5]++;
			runningNumber[5] = buildingInfo->GetAt(i).GetRunningNumber();
		}
		else if (buildingInfo->GetAt(i).GetDeuteriumProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[6] < colonizationPoints && exist[1] == TRUE)
		{
			counter[6]++;
			runningNumber[6] = buildingInfo->GetAt(i).GetRunningNumber();
		}
		else if (buildingInfo->GetAt(i).GetDuraniumProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[7] < colonizationPoints && exist[2] == TRUE)
		{
			counter[7]++;
			runningNumber[7] = buildingInfo->GetAt(i).GetRunningNumber();
		}
		else if (buildingInfo->GetAt(i).GetCrystalProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[8] < colonizationPoints && exist[3] == TRUE)
		{
			counter[8]++;
			runningNumber[8] = buildingInfo->GetAt(i).GetRunningNumber();
		}
		else if (buildingInfo->GetAt(i).GetIridiumProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && counter[9] < colonizationPoints && exist[4] == TRUE)
		{
			counter[9]++;
			runningNumber[9] = buildingInfo->GetAt(i).GetRunningNumber();
		}
	}
	// wenn schon Geb�ude eines Typs stehen, dann d�rfen keine des gleichen Typ zus�tzlich gebaut werden. Z.B. wenn
	// schon Typ 4 Automatikfarmen stehen darf das Kolonieschiff nicht auch noch Primitive Farmen bauen.
	this->CalculateNumberOfWorkbuildings(buildingInfo);
	for (int build = WORKER::FOOD_WORKER; build <= WORKER::IRIDIUM_WORKER; build++)
	{
		WORKER::Typ nWorker = (WORKER::Typ)build;
		if (this->GetNumberOfWorkbuildings(nWorker, 0) > 0)
			runningNumber[build] = 0;
		if (runningNumber[build] != 0)
		{
			// Geb�ude anlegen, umso besser das Kolonieschiff ist, desto mehr Geb�ude stehen dann auf dem System
			for (int i = 0; i < colonizationPoints*2; i++)
			{
				CBuilding building(runningNumber[build]);
				building.SetIsBuildingOnline(buildingInfo->GetAt(runningNumber[build]-1).GetAllwaysOnline());
				m_Buildings.Add(building);
			}
			// Geb�ude mit Arbeitern besetzen
			this->SetWorker(nWorker,SET_WORKER_MODE_SET, colonizationPoints*2);
		}
	}
	this->CalculateNumberOfWorkbuildings(buildingInfo);

	// Wenn das System nach einer Bombardierung komplett ausgel�scht wurde und von uns wieder neu kolonisiert wurde,
	// so m�ssen die mindst. immer baubaren Geb�ude gel�scht werden. Dies wird hier jedesmal gemacht, wenn man
	// eine neue Kolonie gr�ndet
	m_AllwaysBuildableBuildings.RemoveAll();

}

// Funktion baut die Geb�ude der Minorrace, wenn wir eine Mitgliedschaft mit dieser erreicht haben.
void CSystem::BuildBuildingsForMinorRace(BuildingInfoArray* buildingInfo, USHORT averageTechlevel, const CMinor* pMinor)
{
	// alle Geb�ude die wir nach Systemeroberung nicht haben d�rfen werden aus der Liste der aktuellen Geb�ude entfernt
	RemoveSpecialRaceBuildings(buildingInfo);

	if (m_Buildings.GetSize() < 5)
	{
		// in exist[.] steht dann, ob wir einen Rohstoff abbauen k�nnen, wenn ja, dann k�nnen wir auch das Geb�ude bauen
		BOOLEAN exist[RESOURCES::DERITIUM + 1] = {0};
		GetAvailableResources(exist, true);

		// Schauen, welche Geb�udestufe ungef�hr in dem System steht
		// Jetzt nach der Fortschrittlichkeit der kleinen Rasse gehen
		short level = 0;
		switch (pMinor->GetTechnologicalProgress())
		{
		case 0:	// sehr r�ckst�ndig
			{
				USHORT temp = rand()%3+1;
				level = averageTechlevel-temp;
				break;
			}
		case 1:	// r�ckst�ndig
			{
				USHORT temp = rand()%2+1;
				level = averageTechlevel-temp;
				break;
			}
		case 2:	// normal fortschrittlich
			{
				USHORT temp = rand()%3;
				level = averageTechlevel+1-temp;
				break;
			}
		case 3:	// fortschrittlich
			{
				USHORT temp = rand()%2+1;
				level = averageTechlevel+temp;
				break;
			}
		case 4:	// sehr fortschrittlich
			{
				USHORT temp = rand()%3+1;
				level = averageTechlevel+temp;
				break;
			}
		}

		level = max(0, level);
		// Researchlevels f�r Minor zusammenbauen
		BYTE researchLevels[6] = {level, level, level, level, level, level};

		// die g�ltige Eigenschaft der Minor festlegen
		vector<RACE_PROPERTY::Typ> vProperty;
		if (pMinor->IsRaceProperty(RACE_PROPERTY::FINANCIAL))
			vProperty.push_back(RACE_PROPERTY::FINANCIAL);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::WARLIKE))
			vProperty.push_back(RACE_PROPERTY::WARLIKE);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::AGRARIAN))
			vProperty.push_back(RACE_PROPERTY::AGRARIAN);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::INDUSTRIAL))
			vProperty.push_back(RACE_PROPERTY::INDUSTRIAL);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::SECRET))
			vProperty.push_back(RACE_PROPERTY::SECRET);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::SCIENTIFIC))
			vProperty.push_back(RACE_PROPERTY::SCIENTIFIC);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::PRODUCER))
			vProperty.push_back(RACE_PROPERTY::PRODUCER);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::PACIFIST))
			vProperty.push_back(RACE_PROPERTY::PACIFIST);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::SNEAKY))
			vProperty.push_back(RACE_PROPERTY::SNEAKY);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::SOLOING))
			vProperty.push_back(RACE_PROPERTY::SOLOING);
		if (pMinor->IsRaceProperty(RACE_PROPERTY::HOSTILE))
			vProperty.push_back(RACE_PROPERTY::HOSTILE);

		if (vProperty.empty())
			vProperty.push_back(RACE_PROPERTY::NOTHING_SPECIAL);

		RACE_PROPERTY::Typ nProperty = vProperty[rand()%vProperty.size()];
		// Jetzt die Geb�ude bauen und auch das System nach der Art der MinorRace angleichen
		// z.B. bei einer landwirtschaftlichen Rasse viele Farmen
		// Die Rassenartzuweisung m�sste man vielleicht noch dynamisch machen. Bis jetzt wird immer
		// davon ausgegangen, dass z.B. die Major auf dem Cardassianerplatz bestimmte Eigenschaften vertritt.
		// Dies kann sich durch Modifikationen aber ver�ndern.
		// ALPHA5 -> fromRace m�sste noch fest in der Minorrace stehen, daher ist folgender Block vorerst entfernt
		BYTE fromRace = rand()%6 + 1; // liegt erstmal zwischen human und dominion

		USHORT runningNumber[10] = {0,0,0,0,0,0,0,0,0,0};
		USHORT nShipYard = 0;
		for (int i = 0; i < buildingInfo->GetSize(); i++)
		{
			if (fromRace == buildingInfo->GetAt(i).GetOwnerOfBuilding())
			{
				if (!buildingInfo->GetAt(i).IsBuildingBuildableNow(researchLevels))
					continue;

				// wenn noch gar keine Geb�ude standen, also die Rasse frisch vermitgliedert wurde und Raumschiffe bauen
				// kann, so bekommt sie die erste baubare Werft
				if (nShipYard == 0 && m_Buildings.GetSize() == 0 && pMinor->GetSpaceflightNation())
				{
					if (buildingInfo->GetAt(i).GetShipYard() && buildingInfo->GetAt(i).GetPredecessorID() == 0 && buildingInfo->GetAt(i).GetMaxInEmpire() == 0)
						nShipYard = buildingInfo->GetAt(i).GetRunningNumber();
				}

				if (buildingInfo->GetAt(i).GetFoodProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE)
				{
					runningNumber[0] = buildingInfo->GetAt(i).GetRunningNumber();
				}
				else if (buildingInfo->GetAt(i).GetIPProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE)
				{
					runningNumber[1] = buildingInfo->GetAt(i).GetRunningNumber();
				}
				else if (buildingInfo->GetAt(i).GetEnergyProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE)
				{
					runningNumber[2] = buildingInfo->GetAt(i).GetRunningNumber();
				}
				else if (buildingInfo->GetAt(i).GetSPProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE)
				{
					runningNumber[3] = buildingInfo->GetAt(i).GetRunningNumber();
				}
				else if (buildingInfo->GetAt(i).GetFPProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE)
				{
					runningNumber[4] = buildingInfo->GetAt(i).GetRunningNumber();
				}
				else if (buildingInfo->GetAt(i).GetTitanProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && exist[RESOURCES::TITAN] == TRUE)
				{
					runningNumber[5] = buildingInfo->GetAt(i).GetRunningNumber();
				}
				else if (buildingInfo->GetAt(i).GetDeuteriumProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && exist[RESOURCES::DEUTERIUM] == TRUE)
				{
					runningNumber[6] = buildingInfo->GetAt(i).GetRunningNumber();
				}
				else if (buildingInfo->GetAt(i).GetDuraniumProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && exist[RESOURCES::DURANIUM] == TRUE)
				{
					runningNumber[7] = buildingInfo->GetAt(i).GetRunningNumber();
				}
				else if (buildingInfo->GetAt(i).GetCrystalProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && exist[RESOURCES::CRYSTAL] == TRUE)
				{
					runningNumber[8] = buildingInfo->GetAt(i).GetRunningNumber();
				}
				else if (buildingInfo->GetAt(i).GetIridiumProd() > 0 && buildingInfo->GetAt(i).GetWorker() == TRUE && exist[RESOURCES::IRIDIUM] == TRUE)
				{
					runningNumber[9] = buildingInfo->GetAt(i).GetRunningNumber();
				}
			}
		}

		// Jetzt haben wir die ganzen RunningNumbers der baubaren Geb�ude, nun gucken welche Art von Rasse wir haben
		// und danach zuf�llig die Geb�ude bauen
		USHORT numberOfBuildings = 0;
		numberOfBuildings += (int)m_dHabitants+1 + rand()%5;
		int foodBuildings = 0;
		while (numberOfBuildings > 0)
		{
			USHORT build = rand()%10;

			USHORT temp;
			switch (nProperty)
			{
			// Wenn die Rasse eine landwirtschaftliche Rasse ist
			case RACE_PROPERTY::AGRARIAN:
				{
					if (build >= 5)
					{
						temp = rand()%3;
						if (temp == 0)
							build = 0;
					}
					else if (build == 4 || build == 2)
					{
						temp = rand()%2;
						if (temp == 0)
							build = 0;
					}
					else if (build == 3)
						build = 0;
					break;
				}
			case RACE_PROPERTY::INDUSTRIAL:
				{
					if (build >= 5)
					{
						temp = rand()%3;
						if (temp == 0)
							build = 1;
					}
					else if (build == 4 || build == 2)
					{
						temp = rand()%2;
						if (temp == 0)
							build = 1;
					}
					else if (build == 3)
					{
						temp = rand()%2;
						if (temp == 0)
							build = 0;
						else
							build = 1;
					}
					break;
				}
			case RACE_PROPERTY::SECRET:
				{
					if (build >= 5)
					{
						temp = rand()%3;
						if (temp == 0)
							build = 3;
					}
					else if (build == 4)
					{
						temp = rand()%4;
						if (temp == 0)
							build = 3;
					}
					else if (build == 2)
					{
						temp = rand()%4;
						if (temp == 0)
							build = 0;
						else if (temp == 1)
							build = 3;
					}
					break;
				}
			case RACE_PROPERTY::SCIENTIFIC:
				{
					if (build >= 5)
					{
						temp = rand()%3;
						if (temp == 0)
							build = 4;
					}
					else if (build == 1)
					{
						temp = rand()%6;
						if (temp == 0)
							build = 4;
					}
					else if (build == 2)
					{
						temp = rand()%4;
						if (temp == 0)
							build = 4;
					}
					else if (build == 3)
					{
						temp = rand()%2;
						if (temp == 0)
							build = 0;
						else
							build = 4;
					}
					break;
				}
			case RACE_PROPERTY::PRODUCER:
				{
					if (build < 5 && build > 1)
					{
						temp = rand()%5;
						if (temp == 0)
							build = 5;
						else if (temp == 1)
							build = 6;
						else if (temp == 2)
							build = 7;
						else if (temp == 3)
							build = 8;
						else if (temp == 4)
							build = 9;
					}
					break;
				}
			case RACE_PROPERTY::SNEAKY:
				break;
			case RACE_PROPERTY::SOLOING:
				break;
			case RACE_PROPERTY::NOTHING_SPECIAL:
				break;
				// Pazifisten, kriegerische Rassen sowie Finanzrassen bekommen nur weniger Geheimdienst drauf
			default:
				{
					if (build == 2 || build == 3)
					{
						if (rand()%2 == NULL)
							build = 1;
					}
				}
			}

			if (runningNumber[build] != 0)
			{
				if (build == 0)
					foodBuildings++;
				CBuilding building(runningNumber[build]);
				building.SetIsBuildingOnline(buildingInfo->GetAt(runningNumber[build] - 1).GetAllwaysOnline());
				m_Buildings.Add(building);
				numberOfBuildings--;
			}
		}
		// wenn zuwenig Nahrungsgeb�ude vorhanden sind, so werden hier noch ein paar mehr hinzugef�gt
		while (runningNumber[0] != 0 && foodBuildings < m_dHabitants/4)
		{
			CBuilding building(runningNumber[0]);
			building.SetIsBuildingOnline(buildingInfo->GetAt(runningNumber[0] - 1).GetAllwaysOnline());
			m_Buildings.Add(building);
			foodBuildings++;
		}

		// Nun Lager f�llen, sp�ter vlt. noch �ndern nach der Rassenart (bei landwirtschaftl. mehr Nahrung im Lager)
		// Ressourcen sind abh�ngig von der Anzahl der jeweiligen Geb�ude und dem technologischen Fortschritt der
		// Minorrace
		this->CalculateNumberOfWorkbuildings(buildingInfo);
		this->SetFoodStore(this->GetFoodStore() + rand()%(this->GetNumberOfWorkbuildings(WORKER::FOOD_WORKER,0) * (pMinor->GetTechnologicalProgress() + 1) * 100 + 1));
		for (int res = RESOURCES::TITAN; res <= RESOURCES::IRIDIUM; res++)
		{
			WORKER::Typ nWorker = (WORKER::Typ)(res + 5);
			int resAdd = rand()%(this->GetNumberOfWorkbuildings(nWorker, 0) * (pMinor->GetTechnologicalProgress() + 1) * 100 + 1);
			this->SetResourceStore(res, resAdd);
		}

		// wenn m�glich Werft bauen
		if (nShipYard != 0)
		{
			CBuilding building(nShipYard);
			building.SetIsBuildingOnline(buildingInfo->GetAt(nShipYard - 1).GetAllwaysOnline());
			m_Buildings.Add(building);
			SetNewBuildingOnline(buildingInfo);
		}
	}
}

// Funktion rei�t alle Geb�ude ab, die in der Variable m_BuildingDestroy stehen. Funktion wird in der Doc
// bei NextRound() aufgerufen.
bool CSystem::DestroyBuildings(void)
{
	bool destroy = !m_BuildingDestroy.IsEmpty();
	for (int i = 0; i < m_BuildingDestroy.GetSize(); i++)
		// Schleife von hinten durchlaufen, weil Arbeiter die Geb�ude von "vorn" besetzten
		// und ich nicht gerade die online-Geb�ude abrei�en will, sondern die wahrscheinlich offline
		for (int j = m_Buildings.GetUpperBound(); j >= 0; j--)
			if (m_Buildings.GetAt(j).GetRunningNumber() == m_BuildingDestroy.GetAt(i))
			{
				m_Buildings.RemoveAt(j);
				m_BuildingDestroy.RemoveAt(i--);
				break;
			}
	if (destroy)
		m_BuildingDestroy.RemoveAll();
	return destroy;
}

// Funktion setzt das letzte Geb�ude, welches gebaut wurde online, sofern dies m�glich ist.
int CSystem::SetNewBuildingOnline(const BuildingInfoArray *buildingInfos)
{
	// 0: dann konnte das Geb�ude in Betrieb genommen werden
	// 1: dann fehlen Arbeiter
	// 2: dann fehlt Energie im System

	unsigned short nLastBuilding = m_Buildings.GetUpperBound();
	// Ist das Geb�ude schon online
	if (m_Buildings.GetAt(nLastBuilding).GetIsBuildingOnline())
		return 0;

	const CBuildingInfo *pBuildingInfo = &buildingInfos->GetAt(m_Buildings.GetAt(nLastBuilding).GetRunningNumber() - 1);

	// Geb�ude braucht Arbeiter, aber keine sind frei
	if (pBuildingInfo->GetWorker() == TRUE && m_Workers.GetWorker(WORKER::FREE_WORKER) == 0)
		return 1;

	// Geb�ude braucht Energie, aber nicht gen�gend vorhanden
	if (pBuildingInfo->GetNeededEnergy() > m_Production.m_iEnergyProd)
		return 2;

	// Arbeiter reduzieren
	if (pBuildingInfo->GetWorker() == TRUE)
	{
		if (pBuildingInfo->GetFoodProd() > 0)
			m_Workers.InkrementWorker(WORKER::FOOD_WORKER);
		else if (pBuildingInfo->GetIPProd() > 0)
			m_Workers.InkrementWorker(WORKER::INDUSTRY_WORKER);
		else if (pBuildingInfo->GetEnergyProd() > 0)
			m_Workers.InkrementWorker(WORKER::ENERGY_WORKER);
		else if (pBuildingInfo->GetSPProd() > 0)
			m_Workers.InkrementWorker(WORKER::SECURITY_WORKER);
		else if (pBuildingInfo->GetFPProd() > 0)
			m_Workers.InkrementWorker(WORKER::RESEARCH_WORKER);
		else if (pBuildingInfo->GetTitanProd() > 0)
			m_Workers.InkrementWorker(WORKER::TITAN_WORKER);
		else if (pBuildingInfo->GetDeuteriumProd() > 0)
			m_Workers.InkrementWorker(WORKER::DEUTERIUM_WORKER);
		else if (pBuildingInfo->GetDuraniumProd() > 0)
			m_Workers.InkrementWorker(WORKER::DURANIUM_WORKER);
		else if (pBuildingInfo->GetCrystalProd() > 0)
			m_Workers.InkrementWorker(WORKER::CRYSTAL_WORKER);
		else if (pBuildingInfo->GetIridiumProd() > 0)
			m_Workers.InkrementWorker(WORKER::IRIDIUM_WORKER);
		else
			ASSERT(FALSE);
	}

	// Energie von der Produktion abziehen
	m_Production.m_iEnergyProd -= pBuildingInfo->GetNeededEnergy();

	// Geb�ude online setzen
	m_Buildings.ElementAt(nLastBuilding).SetIsBuildingOnline(TRUE);
	return 0;
}

// Funktion �berpr�ft Geb�ude die Energie ben�tigen und schaltet diese gegebenfalls ab,
// falls zuwenig Energie im System vorhanden ist.
// Diese Funktion aufrufen, bevor wir CalculateVariables() usw. aufrufen, weil wir ja die b�sen Onlinegeb�ude vorher
// ausschalten wollen.
bool CSystem::CheckEnergyBuildings()
{
	const BuildingInfoArray* const pvBuildingInfos = resources::BuildingInfo;
	bool bRet = false;
	for (int i = 0; i < m_Buildings.GetSize(); i++)
	{
		// ist gen�gend Energie vorhanden, so muss nichts �berpr�ft werden
		if (m_Production.m_iEnergyProd >= 0)
			return bRet;

		// anderenfalls m�ssen die Geb�ude nacheinander solange abgeschaltet werden, bis wieder gen�gend Energie vorhanden ist
		const CBuildingInfo* pBuildingInfo = &pvBuildingInfos->GetAt(m_Buildings.GetAt(i).GetRunningNumber() - 1);
		if (m_Production.m_iEnergyProd < 0 && pBuildingInfo->GetNeededEnergy() > 0 && m_Buildings[i].GetIsBuildingOnline())
		{
			// Geb�ude abschalten
			m_Buildings[i].SetIsBuildingOnline(FALSE);
			// vorhande Energie um den Differenzbetrag erh�hen
			m_Production.m_iEnergyProd += pBuildingInfo->GetNeededEnergy();
			bRet = true;
		}
	}
	// Wenn bRet == false, dann ist genug Energie vorhanden, ansonsten wurde mindst. ein Geb�ude abgeschaltet
	return bRet;
}

BOOLEAN CSystem::AssemblyListCheck(const BuildingInfoArray* buildingInfo, CGlobalBuildings* globals)
{
	// zu allererst die Liste der baubaren Geb�ude und Updates mit der Liste der in dieser Runde baubaren Geb�ude
	// vor dem AssemblyListCheck f�llen
	m_BuildableBuildings.RemoveAll();
	m_BuildableUpdates.RemoveAll();
	m_BuildableBuildings.FreeExtra();
	m_BuildableUpdates.FreeExtra();
	for (int i = 0; i < m_BuildableWithoutAssemblylistCheck.GetSize(); i++)
	{
		// Unterscheidung zwischen Upgrades und normalen Geb�uden. Auch bei den Upgrades muss geschaut werden, dass dieses Geb�ude
		// noch nicht zu oft vorkommt
		int entry = m_BuildableWithoutAssemblylistCheck.GetAt(i);
		if (entry < 0)
			entry *= (-1);

		// wenn dieses Geb�ude nur X mal pro Imperium vorkommen darf, dann darf dieses Geb�ude nicht mehr als X mal
		// als globales Geb�ude vorkommen
		bool bFound = false;
		if (buildingInfo->GetAt(entry - 1).GetMaxInEmpire() > 0)
		{
			short ID = buildingInfo->GetAt(entry - 1).GetRunningNumber();
			short nCount = globals->GetCountGlobalBuilding(m_sOwnerOfSystem, ID);
			if (nCount >= buildingInfo->GetAt(entry - 1).GetMaxInEmpire())
				bFound = true;
		}
		// wir haben das Geb�ude nicht in der Liste der globalen Geb�ude gefunden
		if (!bFound)
		{
			if (m_BuildableWithoutAssemblylistCheck.GetAt(i) > 0)
				m_BuildableBuildings.Add(entry);
			else
				m_BuildableUpdates.Add(entry);
		}
	}
	// nach Updates in der Bauliste suchen. Wenn wir ein Update in der Bauliste gefunden haben, so
	// m�ssen wir dieses und all deren Vorg�nger aus der Liste der baubaren Geb�ude entfernen
	for (int i = 0; i < ALE; i++)
	{
		if (m_AssemblyList.GetAssemblyListEntry(i) < 0)
		{
			// Vorg�nger des Updates aus der Liste entfernen
			for (int t = 0; t < m_BuildableBuildings.GetSize(); t++)
			{
				USHORT pre = buildingInfo->GetAt(m_AssemblyList.GetAssemblyListEntry(i)*(-1)-1).GetPredecessorID();
				if (pre == m_BuildableBuildings.GetAt(t))
				{
					m_BuildableBuildings.RemoveAt(t--);
					// Haben wir einen Vorg�nger entfernt, dann auch alle Updates des Vorg�ngers, die in der Liste
					// der Updates sind entfernen

					// HIER FIXEN: Auch wenn ein Vorg�nger nicht mehr baubar war m�ssen wir das Update aus der Bauliste
					// entfernen!!!
					/*
					for (int s = 0; s < m_BuildableUpdates.GetSize(); s++)
					{
						USHORT pre2 = buildingInfo->GetAt(m_BuildableUpdates.GetAt(s)-1).GetPredecessorID();
						if (pre2 == pre)
							m_BuildableUpdates.RemoveAt(s--);
					}*/
				}
				for (int s = 0; s < m_BuildableUpdates.GetSize(); s++)
				{
					USHORT pre2 = buildingInfo->GetAt(m_BuildableUpdates.GetAt(s)-1).GetPredecessorID();
					if (pre2 == pre)
						m_BuildableUpdates.RemoveAt(s--);
				}
			}
		}
		else if (m_AssemblyList.GetAssemblyListEntry(i) < 10000 && m_AssemblyList.GetAssemblyListEntry(i) != 0)
		{
			// hier die maximal baubaren Geb�ude pro Imperium checken
			// Die Geb�ude befinden sich auch schon in der GlobalBuildings Variable, wenn sie schon in der
			// Bauliste sind
			USHORT nID = m_AssemblyList.GetAssemblyListEntry(i);
			if (buildingInfo->GetAt(nID - 1).GetMaxInEmpire() > 0)
			{
				int nCount = globals->GetCountGlobalBuilding(m_sOwnerOfSystem, nID);
				// Haben wir dieses Geb�ude schon "oft genug" stehen und in der Bauliste der Systeme, dann aus der
				// Bauliste nehmen
				if (nCount >= buildingInfo->GetAt(nID - 1).GetMaxInEmpire())
					for (int t = 0; t < m_BuildableBuildings.GetSize(); t++)
						if (nID == m_BuildableBuildings.GetAt(t))
						{
							m_BuildableBuildings.RemoveAt(t--);
							break;
						}
			}
			// hier die maximal baubaren Geb�ude pro System checken
			if (buildingInfo->GetAt(m_AssemblyList.GetAssemblyListEntry(i)-1).GetMaxInSystem().Number > 0)
			{
				USHORT n = 0;
				for (int j = 0; j < m_Buildings.GetSize(); j++)
				{
					if (m_Buildings.GetAt(j).GetRunningNumber() == buildingInfo->GetAt(m_AssemblyList.GetAssemblyListEntry(i)-1).GetMaxInSystem().RunningNumber)
						n++;
					if (n >= buildingInfo->GetAt(m_AssemblyList.GetAssemblyListEntry(i)-1).GetMaxInSystem().Number)
						break;
				}
				// Bauliste durchgehen und wenn dieses Gebe�ude in dieser vorkommt, dann auch mitz�hlen
				for (int j = 0; j < ALE; j++)
					if (m_AssemblyList.GetAssemblyListEntry(i) == m_AssemblyList.GetAssemblyListEntry(j))
						if (m_AssemblyList.GetAssemblyListEntry(j) > 0 && m_AssemblyList.GetAssemblyListEntry(j) < 10000)
							if (m_AssemblyList.GetAssemblyListEntry(j) == buildingInfo->GetAt(m_AssemblyList.GetAssemblyListEntry(j)-1).GetMaxInSystem().RunningNumber)
								n++;
				// Haben wir dieses Geb�ude schon "oft genug" stehen und in der Bauliste des Systems, dann aus der
				// Bauliste nehmen
				if (n >= buildingInfo->GetAt(m_AssemblyList.GetAssemblyListEntry(i)-1).GetMaxInSystem().Number)
					for (int t = 0; t < m_BuildableBuildings.GetSize(); t++)
						if (m_AssemblyList.GetAssemblyListEntry(i) == m_BuildableBuildings.GetAt(t))
						{
							m_BuildableBuildings.RemoveAt(t--);
							break;
						}
			}
		}
	}
	return FALSE;
}

// Diese Funktion entfernt alle speziellen Geb�ude aus der Geb�udeliste. Diese Funktion sollte nach Eroberung des Systems
// aufgerufen werden. Danach sind keine Geb�ude mehr vorhanden, die nur x mal pro Imperium baubar gewesen oder die nur die Rasse
// selbst bauen darf.
// @param pvBuildingInfos Zeiger auf den Vektor mit allen Geb�udeinformationen
void CSystem::RemoveSpecialRaceBuildings(const BuildingInfoArray* pvBuildingInfos)
{
	for (int i = 0; i < m_Buildings.GetSize(); i++)
	{
		const CBuildingInfo *pInfo = &pvBuildingInfos->GetAt(m_Buildings[i].GetRunningNumber() - 1);
		if (pInfo->GetMaxInEmpire() > 0 || pInfo->GetOnlyRace() || pInfo->GetShipYard() || pInfo->GetBarrack())
			m_Buildings.RemoveAt(i--);
	}
}

//////////////////////////////////////////////////////////////////////
// calculate something and return it
//////////////////////////////////////////////////////////////////////

class TurnsCalc //helpers for NeededRoundsToBuild()
{
private:

	static float SpeedFactor(short speed)
	{
		return 1.0f + speed / 100.0f;
	}

	static float EffFactor(short efficiency)
	{
		if(efficiency == 0)
			return 1;
		return efficiency / 100.0f;
	}

public:
	static int Turns(float needed, float prod, short speed, short eff = -1)
	{
		float eff_factor = 1.0f;
		if(eff != -1)
			eff_factor = EffFactor(eff);
		float turns = needed /(prod * SpeedFactor(speed) * eff_factor);
		return static_cast<int>(ceil(turns));
	}

	static void CalculateNeededResources(CSystem& system, int index,
		const CBuildingInfo* pBuildingInfo,
		const CShipInfo* pShipInfo, const CTroopInfo* pTroopInfo)
	{
		const CMajor* pMajor = dynamic_cast<CMajor*>(
			resources::pDoc->GetRaceCtrl()->GetRace(system.GetOwnerOfSystem()));
		system.GetAssemblyList()->CalculateNeededRessources(
			pBuildingInfo,
			pShipInfo, pTroopInfo, system.GetAllBuildings(), index,
			pMajor->GetEmpire()->GetResearch()->GetResearchInfo());
	}
};

int CSystem::NeededRoundsToBuild(int index_or_id, bool already_in_list, bool use_potential)
{
	const float prod = use_potential ? m_Production.m_iPotentialIndustryProd : m_Production.GetIndustryProd();
	if(prod <= 0)
	{
		assert(false);
		return INT_MAX;
	}

	float needed = 0;
	if(already_in_list)
	{
		//must be an assembly list index
		assert(0 <= index_or_id && index_or_id < ALE);
		needed = m_AssemblyList.GetNeededIndustryInAssemblyList(index_or_id);
		//till the following call it's perhaps an index, afterwards always an ID
		index_or_id = m_AssemblyList.GetAssemblyListEntry(index_or_id);
	}
	if (index_or_id < 0) // Bei Upgrades
	{
		if(!already_in_list)
		{
			TurnsCalc::CalculateNeededResources(*this, index_or_id,
				&resources::pDoc->GetBuildingInfo(abs(index_or_id)), NULL, NULL);
			needed = m_AssemblyList.GetNeededIndustryForBuild();
		}
		return TurnsCalc::Turns(needed, prod, m_Production.GetUpdateBuildSpeed());
	}
	else if (index_or_id < 10000)	// Bei Geb�uden
	{
		if(!already_in_list)
		{
			TurnsCalc::CalculateNeededResources(*this, index_or_id,
				&resources::pDoc->GetBuildingInfo(index_or_id), NULL, NULL);
			needed = m_AssemblyList.GetNeededIndustryForBuild();
		}
		if(resources::pDoc->GetBuildingInfo(index_or_id).GetNeverReady())
			return already_in_list ? needed : 0;
		return TurnsCalc::Turns(needed, prod, m_Production.GetBuildingBuildSpeed());
	}
	else if (index_or_id < 20000)	// Bei Schiffen Wertfeffiziens mitbeachten
	{
		if(!already_in_list)
		{
			TurnsCalc::CalculateNeededResources(*this, index_or_id, NULL,
				&resources::pDoc->m_ShipInfoArray.GetAt(index_or_id - 10000), NULL);
			needed = m_AssemblyList.GetNeededIndustryForBuild();
		}
		short eff = m_Production.GetShipYardEfficiency();
		return TurnsCalc::Turns(needed, prod, m_Production.GetShipBuildSpeed(), eff);
	}
	// Bei Truppen die Kaserneneffiziens beachten
	assert(m_Production.GetBarrackEfficiency() > 0);
	if(!already_in_list)
	{
		TurnsCalc::CalculateNeededResources(*this, index_or_id, NULL, NULL,
			&resources::pDoc->m_TroopInfo.GetAt(index_or_id - 20000));
		needed = m_AssemblyList.GetNeededIndustryForBuild();
	}
	short eff = m_Production.GetBarrackEfficiency();
	return TurnsCalc::Turns(needed, prod, m_Production.GetTroopBuildSpeed(), eff);
}

int CSystem::CalcIPProd(const CArray<CBuildingInfo, CBuildingInfo>& BuildingInfo, const int list) const {
	int IPProd = m_Production.GetIndustryProd();
	// Wenn ein Auftrag in der Bauliste ist, der niemals fertig ist z.B. Kriegsrecht, dann dies
	// einzeln beachten
	if (list > 0 && list < 10000 && BuildingInfo[list-1].GetNeverReady() == TRUE)
	{
		// dann z�hlen die industriellen Baukosten des Auftrags als Anzahl Runden, wie lange
		// es in der Liste stehen bleibt und den Bonus gibt
		// Also wird immer nur ein IP abgezogen, au�er wenn die Moral gr��er gleich 80 ist,
		// dann wird der Auftrag gleich abgebrochen
		if (m_iMoral >= 85)
			IPProd = static_cast<int>(m_AssemblyList.GetNeededIndustryForBuild());
		else
			IPProd = 1;
	}
	// Wenn ein Update in der Liste ist die m�gliche UpdateBuildSpeed-Boni beachten
	else if (list < 0)
		IPProd = (int)floor((float)IPProd * (100 + m_Production.GetUpdateBuildSpeed()) / 100);
	// Wenn ein Geb�ude in der Liste ist die m�gliche BuildingBuildSpeed-Boni beachten
	else if (list < 10000)
		IPProd = (int)floor((float)IPProd * (100 + m_Production.GetBuildingBuildSpeed()) / 100);
	// Wenn es ein Schiff ist, dann die Effiziens der Werft und m�glichen ShipBuildSpeed-Boni beachten
	else if (list < 20000)
		IPProd = (int)floor((float)IPProd * m_Production.GetShipYardEfficiency() / 100
					* (100 + m_Production.GetShipBuildSpeed()) / 100);
	// Wenn es eine Truppe ist, dann Effizienz der Werft und m�glichen TroopBuildSpeed-Boni beachten
	else
		IPProd = (int)floor((float)IPProd * m_Production.GetBarrackEfficiency() / 100
					* (100 + m_Production.GetTroopBuildSpeed()) / 100);

	return IPProd;
}

//////////////////////////////////////////////////////////////////////
// trade routes, resource routes
//////////////////////////////////////////////////////////////////////

// Funktion �berpr�ft, ob wie aufgrund der Bev�lkerung hier im System �berhaupt (noch) eine Handelsroute
// anlegen k�nnen
BOOLEAN CSystem::CanAddTradeRoute(CResearchInfo* researchInfo)  const
{
	// aller 20Mrd. Bev�lkerung bekommt man eine neue Handelsroute
	USHORT currentTradeRoutes = m_TradeRoutes.GetSize();
	short addResRoute = 1;
	USHORT maxTradeRoutes = (USHORT)(m_dHabitants / TRADEROUTEHAB) + m_Production.GetAddedTradeRoutes();
	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Hier die Boni durch die Uniqueforschung "Lager und Transport" -> eine Ressourcenroute mehr
	if(const int bonus = researchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT, 3))
		addResRoute += bonus;
	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Bei dieser Spezialforschung gibt es wom�glich auch eine Ressourcenroute mehr
	// Hier die Boni durch die Uniqueforschung "Handel" -> mindestens eine Handelsroute
	if(const int bonus = researchInfo->IsResearchedThenGetBonus(RESEARCH_COMPLEX::TRADE, 3))
	{
		if (maxTradeRoutes == NULL)
		{
			addResRoute += bonus;
			maxTradeRoutes += bonus;
		}
	}
	if (m_ResourceRoutes.GetSize() > addResRoute)
		currentTradeRoutes += m_ResourceRoutes.GetSize() - addResRoute;
	return (currentTradeRoutes < maxTradeRoutes);
}

// Funktion generiert eine neue Handelsroute. Wenn die Funktion <code>TRUE</code> zur�ckgibt, dann konnte die
// Handelsroute erfolgreich angelegt werden. Als Parameter wird dabei die Koordinate <code>dest</code>des
// Zielsektors �bergeben sowie ein Zeiger auf alle Systeme <code>systems</code> auf der Map..
BOOLEAN CSystem::AddTradeRoute(CPoint dest, std::vector<CSystem>& systems, CResearchInfo* researchInfo)
{
	BOOLEAN canAddTradeRoute = CanAddTradeRoute(researchInfo);
	// als erstes �berpr�fen, das noch keine Handelsroute aus einem System, welches den gleichen Besitzer wie
	// dieses System hat, zu diesem Ziel f�hrt. Man kann immer nur eine Handelsroute zu einem bestimmten
	// System haben.
	if (canAddTradeRoute)
		for(std::vector<CSystem>::const_iterator it = systems.begin(); it != systems.end(); ++it)
			if(it->GetOwnerOfSystem() == m_sOwnerOfSystem)
				if(&*it != this)
					for (int i = 0; i < it->GetTradeRoutes()->GetSize(); i++)
						if(it->GetTradeRoutes()->GetAt(i).GetDestKO() == dest)
							return FALSE;

	// nachschauen, das nicht schon eine Handelsroute mit dem Ziel vorhanden ist, wenn dies der Fall ist, dann wird
	// diese Handelsroute gel�scht
	for (int i = 0; i < m_TradeRoutes.GetSize(); i++)
		if (m_TradeRoutes.GetAt(i).GetDestKO() == dest)
		{
			// Wenn wir eine Route aufheben wollen, so die Dauer der Handelsroute negativ setzen. Wenn die Handelsroute
			// eh nur noch weniger als 5 Runden dauert, dann wird die K�ndigungsdauer auf diese k�rzere Zeit eingestellt.
			if (m_TradeRoutes.GetAt(i).GetDuration() > 0)
			{
				// weniger als 5 Runden
				if (m_TradeRoutes.GetAt(i).GetDuration() < 5)
					m_TradeRoutes.ElementAt(i).SetDuration(m_TradeRoutes.GetAt(i).GetDuration()-6);
				// ansonsten normal einstellen
				else
					m_TradeRoutes.ElementAt(i).SetDuration(-1);
				return TRUE;
			}
			else
				return FALSE;
		}

	if (canAddTradeRoute)
	{
		CTradeRoute route;
		route.GenerateTradeRoute(dest);
		m_TradeRoutes.Add(route);
		return TRUE;
	}
	return FALSE;
}

// Funktion gibt das gesamte Credits zur�ck, was alle Handelsrouten aus diesem System generiert haben.
USHORT CSystem::CreditsFromTradeRoutes() const
{
	USHORT credits = 0;
	for (int i = 0; i < m_TradeRoutes.GetSize(); i++)
		credits += m_TradeRoutes.GetAt(i).GetCredits(this->m_Production.GetIncomeOnTradeRoutes());
	return credits;
}

// Funkt �berpr�ft alle Handelsrouten in dem System, ob sie Aufgrund der Bev�lkerung oder der Geb�ude noch
// da sein k�nnen. Wurden Handelsrouten gel�scht, so gibt die Funktion die Anzahl der L�schungen zur�ck
BYTE CSystem::CheckTradeRoutes(CResearchInfo* researchInfo)
{
	BYTE number = 0;
	// Dauer der Handelsrouten �berpr�fen. Wenn eine negative Dauer von -5 erreicht wurde, so wird die Handelsroute
	// gel�scht, wenn eine positive Dauer von 1 erreicht wurde, so verl�ngert sich die Dauer wieder auf 20 Runden.
	// Bei allen anderen F�llen wird um eins dekrementiert.
	for (int i = 0; i < m_TradeRoutes.GetSize(); i++)
	{
		// Wenn das System blockiert wird, dann produziert die Handelsroute kein Credits
		if (GetBlockade() > NULL)
			m_TradeRoutes.GetAt(i).SetCredits(0);
		if (m_TradeRoutes.GetAt(i).GetDuration() <= -5)
		{
			m_TradeRoutes.RemoveAt(i--);
			number++;
		}
		else if (m_TradeRoutes.GetAt(i).GetDuration() == 1)
			m_TradeRoutes.ElementAt(i).SetDuration(20);
		else
			m_TradeRoutes.ElementAt(i).SetDuration(m_TradeRoutes.GetAt(i).GetDuration()-1);
	}

	// Hier Bev�lkerungs und Geb�udecheck durchf�hren
	USHORT currentTradeRoutes = m_TradeRoutes.GetSize();
	short addResRoute = 1;
	USHORT maxTradeRoutes = (USHORT)(m_dHabitants / TRADEROUTEHAB) + m_Production.GetAddedTradeRoutes();
	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Hier die Boni durch die Uniqueforschung "Lager und Transport" -> eine Ressourcenroute mehr
	if (researchInfo->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
		addResRoute += researchInfo->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetBonus(3);
	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Bei dieser Spezialforschung gibt es wom�glich auch eine Ressourcenroute mehr
	// Hier die Boni durch die Uniqueforschung "Handel" -> mindestens eine Handelsroute
	if (researchInfo->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
	{
		if (maxTradeRoutes == NULL)
		{
			addResRoute += researchInfo->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetBonus(3);
			maxTradeRoutes += researchInfo->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetBonus(3);
		}
	}
	if (m_ResourceRoutes.GetSize() > addResRoute)
		currentTradeRoutes += m_ResourceRoutes.GetSize() - addResRoute;
	while (currentTradeRoutes > maxTradeRoutes)
	{
		if (m_TradeRoutes.GetSize() > 0)
		{
			m_TradeRoutes.RemoveAt(m_TradeRoutes.GetUpperBound());
			number++;
			currentTradeRoutes--;
		}
		else
			break;
	}
	return number;
}

unsigned CSystem::CheckTradeRoutesDiplomacy(CBotEDoc& pDoc) {
	unsigned deletedTradeRoutes = 0;
	for (int i = 0; i < m_TradeRoutes.GetSize(); i++)
	{
		CTradeRoute& trade_route = m_TradeRoutes.GetAt(i);
		const CPoint& dest = trade_route.GetDestKO();
		// Wenn die Handelsroute aus diplomatischen Gr�nden nicht mehr vorhanden sein kann
		if (!trade_route.CheckTradeRoute(m_KO, dest, &pDoc))
		{
			// dann m�ssen wir diese Route l�schen
			m_TradeRoutes.RemoveAt(i--);
			deletedTradeRoutes++;
		}
		// Ansonsten k�nnte sich die Beziehung zu der Minorrace verbessern
		else
			trade_route.PerhapsChangeRelationship(m_KO, dest, &pDoc);
	}
	return deletedTradeRoutes;
}

//////////////////////////////////////////////////////////////////////
// resource routes
//////////////////////////////////////////////////////////////////////

// Funktion generiert eine neue Ressourcenroute. Wenn die Funktion <code>TRUE</code> zur�ckgibt, dann konnte die
// Ressourcenroute erfolgreich angelegt werden. Als Parameter wird dabei die Koordinate <code>dest</code> des
// Zielsektors �bergeben sowie die Art der betroffenen Ressource <code>res</code> und einen Zeiger auf alle
// Systeme <code>systems</code> auf der Map.
BOOLEAN CSystem::AddResourceRoute(CPoint dest, BYTE res, const std::vector<CSystem>& systems, CResearchInfo* researchInfo)
{
	// Es gibt immer mindst. eine Ressourcenroute pro System. Insgesamt stehen "Anzahl Handelsrouten + 1" Ressourcen-
	// routen zur Vref�gung.
	short addResRoute = 1;
	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Hier die Boni durch die Uniqueforschung "Lager und Transport" -> eine Ressourcenroute mehr
	if (researchInfo->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
		addResRoute += researchInfo->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetBonus(3);
	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Bei dieser Spezialforschung gibt es wom�glich auch eine Ressourcenroute mehr
	// Hier die Boni durch die Uniqueforschung "Handel" -> mindestens eine Handelsroute
	if (researchInfo->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
	{
		USHORT maxTradeRoutes = (USHORT)(m_dHabitants / TRADEROUTEHAB) + m_Production.GetAddedTradeRoutes();
		if (maxTradeRoutes == NULL)
			addResRoute += researchInfo->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetBonus(3);
	}

	USHORT maxResourceRoutes = (USHORT)(m_dHabitants / TRADEROUTEHAB) + m_Production.GetAddedTradeRoutes() + addResRoute;

	if (systems.at(dest.x+(dest.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() != this->GetOwnerOfSystem())
		return FALSE;
	if (systems.at(dest.x+(dest.y)*STARMAP_SECTORS_HCOUNT).GetHabitants() == 0.0f || this->GetHabitants() == 0.0f)
		return FALSE;
	if (maxResourceRoutes <= m_ResourceRoutes.GetSize() + m_TradeRoutes.GetSize())
		return FALSE;
	// gibt es schon eine Route mit dem Rohstoff ins Zielsystem?
	for (int i = 0; i < m_ResourceRoutes.GetSize(); i++)
		if (m_ResourceRoutes.GetAt(i).GetResource() == res && m_ResourceRoutes.GetAt(i).GetKO() == dest)
			return FALSE;

	CResourceRoute route;
	route.GenerateResourceRoute(dest, res);
	m_ResourceRoutes.Add(route);
	return TRUE;
}

// Funktion �berpr�ft alle Ressourcenrouten in dem System, ob sie Aufgrund der Bev�lkerung oder der Geb�ude noch
// da sein d�rfen. Wurden Ressourcenrouten gel�scht, so gibt die Funktion die Anzahl der L�schungen zur�ck.
BYTE CSystem::CheckResourceRoutes(CResearchInfo* researchInfo)
{
	// Es gibt immer mindst. eine Ressourcenroute pro System. Insgesamt stehen "Anzahl Handelsrouten + 1" Ressourcen-
	// routen zur Vref�gung.
	short addResRoute = 1;
	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Hier die Boni durch die Uniqueforschung "Lager und Transport" -> eine Ressourcenroute mehr
	if (researchInfo->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
		addResRoute += researchInfo->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetBonus(3);
	///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
	// Bei dieser Spezialforschung gibt es wom�glich auch eine Ressourcenroute mehr
	// Hier die Boni durch die Uniqueforschung "Handel" -> mindestens eine Handelsroute
	if (researchInfo->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
	{
		USHORT maxTradeRoutes = (USHORT)(m_dHabitants / TRADEROUTEHAB) + m_Production.GetAddedTradeRoutes();
		if (maxTradeRoutes == NULL)
			addResRoute += researchInfo->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetBonus(3);
	}

	USHORT maxResourceRoutes = (USHORT)(m_dHabitants / TRADEROUTEHAB) + m_Production.GetAddedTradeRoutes() + addResRoute;
	USHORT currentResourceRoutes = m_ResourceRoutes.GetSize() + m_TradeRoutes.GetSize();
	BYTE number = 0;
	while (currentResourceRoutes > maxResourceRoutes)
	{
		if (m_ResourceRoutes.GetSize() > 0)
		{
			m_ResourceRoutes.RemoveAt(m_ResourceRoutes.GetUpperBound());
			number++;
			currentResourceRoutes--;
		}
		else
			break;
	}
	return number;
}

unsigned CSystem::CheckResourceRoutesExistence(CBotEDoc& pDoc) {
	unsigned deletedResourceRoutes = 0;
	// checken ob das System noch der Rasse geh�rt, welcher auch das Startsystem der Route geh�rt
	for (int i = 0; i < m_ResourceRoutes.GetSize(); i++)
	{
		CResourceRoute& res_route = m_ResourceRoutes.GetAt(i);
		const CPoint dest = res_route.GetKO();

		if (!res_route.CheckResourceRoute(m_sOwnerOfSystem, &pDoc.GetSector(dest.x, dest.y)))
		{
			m_ResourceRoutes.RemoveAt(i--);
			deletedResourceRoutes++;
		}
	}
	return deletedResourceRoutes;
}

//////////////////////////////////////////////////////////////////////
// troops
//////////////////////////////////////////////////////////////////////

// Diese Funktion berechnet die baubaren Truppen in diesem System
void CSystem::CalculateBuildableTroops(const CArray<CTroopInfo>* troopInfos, const CResearch *research)
{
	m_BuildableTroops.RemoveAll();
	m_BuildableTroops.FreeExtra();
	// Hier jetzt schauen ob wir eine Kaserne haben
	if (m_Production.GetBarrack())
		for (int i = 0; i < troopInfos->GetSize(); i++)
			if (troopInfos->GetAt(i).GetOwner() == m_sOwnerOfSystem)
			{
				BOOLEAN buildable = TRUE;
				// zuerstmal die Forschungsstufen checken
				if (research->GetBioTech() < troopInfos->GetAt(i).GetNeededTechlevel(0))
					buildable = FALSE;
				else if (research->GetEnergyTech() < troopInfos->GetAt(i).GetNeededTechlevel(1))
					buildable = FALSE;
				else if (research->GetCompTech() < troopInfos->GetAt(i).GetNeededTechlevel(2))
					buildable = FALSE;
				else if (research->GetPropulsionTech() < troopInfos->GetAt(i).GetNeededTechlevel(3))
					buildable = FALSE;
				else if (research->GetConstructionTech() < troopInfos->GetAt(i).GetNeededTechlevel(4))
					buildable = FALSE;
				else if (research->GetWeaponTech() < troopInfos->GetAt(i).GetNeededTechlevel(5))
					buildable = FALSE;
				if (buildable == TRUE)
					m_BuildableTroops.Add(troopInfos->GetAt(i).GetID());
			}
}

// Wenn in diesem System Truppen stationiert sind, dann wird deren Moralwert mit einbezogen.
// Ist die Moral im System unter 100, so wird der Moralwert der Einheit dazuaddiert, wenn er �ber 100 ist, dann wird
// der Moralwert abgezogen.
void CSystem::IncludeTroopMoralValue(CArray<CTroopInfo>* troopInfo)
{
	if (m_Troops.GetSize() > 0 && m_iMoral != 100)
		for (int i = 0; i < m_Troops.GetSize(); i++)
		{
			BYTE id = m_Troops.GetAt(i).GetID();
			// Wenn die Systemmoral kleiner als 100 ist, dann wird der Moralwert der Truppe dazuaddiert
			if (m_iMoral < 100)
			{
				m_iMoral += troopInfo->GetAt(id).GetMoralValue();
				if (m_iMoral > 100)
					m_iMoral = 100;
			}
			// Wenn die Systemmoral gr��er als 100 ist, dann wird der Moralwert der Truppe abgezogen
			else if (m_iMoral > 100)
			{
				m_iMoral -= troopInfo->GetAt(id).GetMoralValue();
				if (m_iMoral < 100)
					m_iMoral = 100;
			}
		}
}

void CSystem::TrainTroops()
{
	const int xp = m_Production.GetTroopTraining();
	for(int i = 0; i < m_Troops.GetSize(); ++i)
		m_Troops.GetAt(i).AddExperiancePoints(xp);
}

//////////////////////////////////////////////////////////////////////
// other functions
//////////////////////////////////////////////////////////////////////

static void ManagerMessage(const CString& text, CMajor& owner, const CPoint& p)
{
	CEmpireNews message;
	message.CreateNews(text,EMPIRE_NEWS_TYPE::ECONOMY,"",p);
	owner.GetEmpire()->AddMsg(message);
}

void CSystem::ExecuteManager(CMajor& owner, bool turn_change, bool energy)
{
	if(!m_Manager.Active() || !owner.IsHumanPlayer())
		return;

	const CString& name = GetName();

	if(energy && m_Manager.CheckEnergyConsumers(*this) && turn_change)
		ManagerMessage(CLoc::GetString("MANAGER_BOMB_WARNING",false, name), owner, m_KO);
	if(!m_Manager.DistributeWorkers(*this))
		ManagerMessage(CLoc::GetString("MANAGER_MALFUNCTION",false, name), owner, m_KO);
	if(turn_change && m_Manager.CheckFamine(*this))
		ManagerMessage(CLoc::GetString("MANAGER_FAMINE_WARNING",false, name), owner, m_KO);
}
