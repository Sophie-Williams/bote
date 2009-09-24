#include "stdafx.h"
#include "IntelCalc.h"
#include "Botf2Doc.h"
#include "Races\RaceController.h"
#include "Ships\Fleet.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
CIntelCalc::CIntelCalc(CBotf2Doc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;
}

CIntelCalc::~CIntelCalc(void)
{
}

//////////////////////////////////////////////////////////////////////
// sonstige Funktionen
//////////////////////////////////////////////////////////////////////
/// Funktion berechnet die kompletten Geheimdienstaktionen und nimmt gleichzeitig auch alle Ver�nderungen vor.
/// Die Funktion verschickt auch alle relevanten Nachrichten an die betroffenen Imperien.
void CIntelCalc::StartCalc(CMajor* pRace)
{
	ASSERT(pRace);
	srand((unsigned)time(NULL));
	// resultierende Spionagegeheimdienstpunkte bei einer Rasse
	map<CString, UINT> m_iSpySP;
	// resultierende Sabotagegeheimdienstpunkte bei einer Rasse
	map<CString, UINT> m_iSabSP;

	CIntelligence* pIntel = pRace->GetEmpire()->GetIntelligence();
	
	// Ein m�glicher Anschlag wird vor allen anderen Geheimdienstaktionen durchgef�hrt. Bei einem Anschlag werden
	// nur die angesammelten Geheimdienspunkte benutzt.
	CIntelObject* attemptObj = pIntel->GetIntelReports()->GetAttemptObject();
	if (attemptObj)
	{
		int ourSP = pIntel->GetSPStorage(1, attemptObj->GetEnemy());
		// + eventuellen Bonus aus dem Teilbereich dazurechnen
		ourSP += ourSP * pIntel->GetBonus(attemptObj->GetType(), 1) / 100;
		this->ExecuteAttempt(pRace, ourSP);
	}

	// zuerst werden die Geheimdienstgegner ermittelt. Nur wenn bei einem Gegner mehr als NULL resultierende
	// Geheimdienstpunkte vorhanden sind ist es ein wirklicher Gegner.
	map<CString, CMajor*>* pmMajors = m_pDoc->GetRaceCtrl()->GetMajors();
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); it++)
		if (it->first != pRace->GetRaceID())
		{
			m_iSpySP[it->first] = pIntel->GetSecurityPoints() *	pIntel->GetAssignment()->GetGlobalSpyPercentage(it->first) / 100;		
			m_iSabSP[it->first] = pIntel->GetSecurityPoints() *	pIntel->GetAssignment()->GetGlobalSabotagePercentage(it->first) / 100;
		}
	
	// dann k�nnen die Geheimdienstaktionen gestartet werden
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); it++)
		if (it->first != pRace->GetRaceID())
		{
			// sind effektive Geheimdienstpunkte f�r Spionage gegen Rasse i vorhanden oder es befinden sich Punkte im Depot?
			if (m_iSpySP[it->first] > NULL || pIntel->GetSPStorage(0,it->first) > NULL)
			{
				// die speziellen Bereiche durchgehen
				// Wirtschaft == 0, Forschung == 1, Milit�r == 2, Diplomatie == 3
				int b = rand()%4;
				int count = 0;
				while (count < 4)
				{
					// Spionagepunkte * spezielle Spionagezuteilung / 100 + Spionagedepot * spezielle Spionagezuteilung / 100
					int effectiveSP = m_iSpySP[it->first] * pIntel->GetAssignment()->GetSpyPercentages(it->first,b) / 100 + pIntel->GetSPStorage(0,it->first) * pIntel->GetAssignment()->GetSpyPercentages(it->first,b) / 100;
					// + eventuellen Bonus aus dem Teilbereich dazurechnen
					effectiveSP += effectiveSP * pIntel->GetBonus(b, 0) / 100;
					if (effectiveSP > NULL)
					{
						CMajor* pResponsibleRace = pRace;
						USHORT actions = this->IsSuccess(it->second, effectiveSP, TRUE, pResponsibleRace, b);
						this->DeleteConsumedPoints(pRace, it->second, TRUE, b, FALSE);
						// jetzt Geheimdienstaktion starten!
						int stop = 0;	// irgendwann mal abbrechen, falls aus irgendeinem Grund actions mal nie runtergez�hlt werden kann
						while (actions > 0 && stop < 100)
						{
							if (this->ExecuteAction(pRace, it->second, pResponsibleRace, b, TRUE))
							{
								actions--;
								// Die Moral verschlechtert sich beim betroffenen Imperium gegeb�ber der responsible Race
								if (pResponsibleRace != NULL)
									it->second->SetRelation(pResponsibleRace->GetRaceID(), -rand()%5);
							}
							stop++;
						}
					}
					if (b == 3)
						b = 0;
					else
						b++;
					count++;
				}
			}
			// sind effektive Geheimdienstpunkte f�r Sabotage gegen Rasse i vorhanden oder es befinden sich Punkte im Depot?
			if (m_iSabSP[it->first] > NULL || pIntel->GetSPStorage(1,it->first) > NULL)
			{
				// die speziellen Bereiche durchgehen
				// Wirtschaft == 0, Forschung == 1, Milit�r == 2, Diplomatie == 3
				int b = rand()%4;
				int count = 0;
				while (count < 4)
				{
					// Spionagepunkte * spezielle Spionagezuteilung / 100 + Spionagedepot * spezielle Spionagezuteilung / 100
					int effectiveSP = m_iSabSP[it->first] * pIntel->GetAssignment()->GetSabotagePercentages(it->first,b) / 100
						+ pIntel->GetSPStorage(1,it->first) * pIntel->GetAssignment()->GetSabotagePercentages(it->first,b) / 100;
					// + eventuellen Bonus aus dem Teilbereich dazurechnen
					effectiveSP += effectiveSP * pIntel->GetBonus(b, 1) / 100;
					if (effectiveSP > NULL)
					{
						CMajor* pResponsibleRace = pRace;
						USHORT actions = this->IsSuccess(it->second, effectiveSP, FALSE, pResponsibleRace, b);
						this->DeleteConsumedPoints(pRace, it->second, FALSE, b, FALSE);
						// jetzt Geheimdienstaktion starten!
						int stop = 0;	// irgendwann mal abbrechen, falls aus irgendeinem Grund actions mal nie runtergez�hlt werden kann
						while (actions > 0 && stop < 100)
						{
							if (this->ExecuteAction(pRace, it->second, pResponsibleRace, b, FALSE))
							{
								actions--;
								// Die Moral verschlechtert sich beim betroffenen Imperium gegeb�ber der responsible Race
								if (pResponsibleRace != NULL)
									it->second->SetRelation(pResponsibleRace->GetRaceID(), -rand()%10);
							}
							stop++;
						}
					}
					if (b == 3)
						b = 0;
					else
						b++;
					count++;
				}
			}
		}	
}

/// Funktion addiert Innere Sicherheitspunkte sowie die ganzen Depotgeheimdienstpunkte einer Rasse zu den vorhandenen.
void CIntelCalc::AddPoints(CMajor* pRace)
{
	ASSERT(pRace);
	CIntelligence* pIntel = pRace->GetEmpire()->GetIntelligence();

	// Punkte der inneren Sicherheit hinzuf�gen
	int add = pIntel->GetSecurityPoints() * pIntel->GetAssignment()->GetInnerSecurityPercentage() / 100;
	add += add * pIntel->GetInnerSecurityBoni() / 100;
	pIntel->AddInnerSecurityPoints(add);

	// Punkte den einzelnen Depots hinzuf�gen (hier kein Bonus durch eventuelle Boni auf die einzelnen Ressorts)
	map<CString, CMajor*>* pmMajors = m_pDoc->GetRaceCtrl()->GetMajors();
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); it++)
		if (it->first != pRace->GetRaceID())
		{
			// Spionagedepot auff�llen
			UINT depot = pIntel->GetSecurityPoints() *	pIntel->GetAssignment()->GetGlobalSpyPercentage(it->first) * pIntel->GetAssignment()->GetSpyPercentages(it->first, 4) / 10000;
			pIntel->AddSPStoragePoints(0, it->first, depot);

			// Sabotagedepot auff�llen
			depot = pIntel->GetSecurityPoints() * pIntel->GetAssignment()->GetGlobalSabotagePercentage(it->first) *	pIntel->GetAssignment()->GetSabotagePercentages(it->first, 4) / 10000;
			pIntel->AddSPStoragePoints(1, it->first, depot);
		}
}

/// Funktion zieht einen rassenabh�ngigen Prozentsatz von den einzelnen Depots ab. Funkion sollte nach Ausf�hrung
/// aller anderen Geheimdienstfunktionen aufgerufen werden.
void CIntelCalc::ReduceDepotPoints(CMajor* pRace, int perc)
{
	ASSERT(pRace);
	CIntelligence* pIntel = pRace->GetEmpire()->GetIntelligence();

	// bei perc == -1 wird der rassenspezifische Prozentsatz verwendet
	if (perc == -1)
	{
		int perc = 20;
		if (pRace->IsRaceProperty(FINANCIAL))
			perc += 15;
		if (pRace->IsRaceProperty(WARLIKE))
			perc += 5;
		if (pRace->IsRaceProperty(AGRARIAN))
			perc += 10;
		if (pRace->IsRaceProperty(INDUSTRIAL))
			perc += 5;
		if (pRace->IsRaceProperty(SECRET))
			perc -= 10;
		if (pRace->IsRaceProperty(SCIENTIFIC))
			perc += 0;
		if (pRace->IsRaceProperty(PRODUCER))
			perc += 0;
		if (pRace->IsRaceProperty(PACIFIST))
			perc += 35;
		if (pRace->IsRaceProperty(SNEAKY))
			perc -= 5;
		if (pRace->IsRaceProperty(SOLOING))
			perc -= 20;
		if (pRace->IsRaceProperty(HOSTILE))
			perc += 10;

		if (perc > 100)
			perc = 100;
		else if (perc < 0)
			perc = 0;
	}
	// Prozentsatz aus den einzelnen Depots abziehen
	// zuerst das Lager der angesammelten inneren Sicherheitspunkte
	int diff = pIntel->GetInnerSecurityStorage() * perc / 100;
	pIntel->AddInnerSecurityPoints(-diff);

	map<CString, CMajor*>* pmMajors = m_pDoc->GetRaceCtrl()->GetMajors();
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); it++)
		if (it->first != pRace->GetRaceID())
		{
			// Wenn die Rasse kein Systeme mehr besitzt, also ausgel�scht wurde, so verschwinden alle
			// angesammelten Geheimdienstpunkte
			int oldPerc = perc;
			if (it->second->GetEmpire()->GetNumberOfSystems() == NULL)
				perc = 100;
			// Depot der Spionagepunkte
			diff = pIntel->GetSPStorage(0,it->first) * perc / 100;
			pIntel->AddSPStoragePoints(0,it->first,-diff);
			// Depot der Sabotagepunkte
			diff = pIntel->GetSPStorage(1,it->first) * perc / 100;
			pIntel->AddSPStoragePoints(1,it->first,-diff);
			perc = oldPerc;
		}
}

//////////////////////////////////////////////////////////////////////
// private Funktionen
//////////////////////////////////////////////////////////////////////
/// Funktion berechnet ob eine Geheimdienstaktion gegen eine andere Rasse erfolgreich verl�uft.
USHORT CIntelCalc::IsSuccess(CMajor* pEnemyRace, UINT ourSP, BOOLEAN isSpy, CMajor* pResponsibleRace, BYTE type)
{	
#ifdef TRACE_INTEL
	MYTRACE(MT::LEVEL_INFO, "CIntelCalc::IsSuccess() begin...\n");
#endif
	ASSERT(pEnemyRace);
	ASSERT(pResponsibleRace);
	USHORT actions = NULL;

	UINT enemyInnerSec = GetCompleteInnerSecPoints(pEnemyRace);
		
#ifdef TRACE_INTEL
	MYTRACE(MT::LEVEL_INFO, "inner security of %s is %d\n", pEnemyRace, enemyInnerSec);
#endif

	// Aggressivit�t der angreifenden Rasse holen

	BYTE agg = pResponsibleRace->GetEmpire()->GetIntelligence()->GetAggressiveness(!isSpy, pEnemyRace->GetRaceID());

#ifdef TRACE_INTEL
	MYTRACE(MT::LEVEL_INFO, "intel aggressiveness is %d\n", agg);
#endif
	agg = 3 - agg; // -> vorsichtig == 3, normal == 2, aggressiv == 1 -> drehen die Zahlen im Prinzip um

	// Spionage hat gr��ere Erfolgsaussichten als Sabotage
	int minDiff = 0;
	if (isSpy)
	{
		for (int i = 0; i < 3; i++)
			minDiff += rand()%(500 * agg) + 1;
		minDiff /= 3;
	}
	else
	{
		for (int i = 0; i < 3; i++)
			minDiff += rand()%(1000 * agg) + 1;
		minDiff /= 3;
	}

#ifdef TRACE_INTEL
	MYTRACE(MT::LEVEL_INFO, "SP of race %s %d > enemies inner security %d + random %d\n", pResponsibleRace->GetRaceID(), ourSP, enemyInnerSec, minDiff);
#endif
	if (ourSP > (enemyInnerSec + minDiff))
	{	// wenn wir viel mehr Punkte als der Gegner haben, so k�nnen auch mehrere Geheimdienstaktionen gestarten werden.
		actions = ourSP / (enemyInnerSec + minDiff);
		// wenn unsere Geheimdienstpunkte doppelt so hoch sind wie die inneren Sicherheitspunkte, dann verschleiern wir
		// unsere Geheimdienstaktion
		if (isSpy == FALSE && ourSP > (2 * enemyInnerSec))
		{
			// Wenn sogar unsere Punkte dreimal so hoch sind, dann machen wir wom�glich eine andere Rasse daf�r verantworlich
			if (ourSP > (3 * enemyInnerSec))
			{
				CString sResponsibleRace = pResponsibleRace->GetEmpire()->GetIntelligence()->GetResponsibleRace();
				pResponsibleRace = dynamic_cast<CMajor*>(m_pDoc->GetRaceCtrl()->GetRace(sResponsibleRace));
				if (pResponsibleRace != NULL)
				{
					if (sResponsibleRace == pEnemyRace->GetRaceID())
						pResponsibleRace = NULL;
					// checken ob unser Geheimdienstopfer die "responisibleRace" auch kennt
					else if (pEnemyRace->IsRaceContacted(sResponsibleRace) == FALSE)
						pResponsibleRace = NULL;
				}
			}
			else
				pResponsibleRace = NULL;
		}
	}
	// Handelt es sich um eine fehlgeschlagene Aktion, so ist es m�glich, dass das vermeindliche Geheimdienstopfer
	// etwas davon erf�hrt.
	else if ((ourSP * agg) < (enemyInnerSec + minDiff) && rand()%2 == NULL)
		this->CreateMsg(pResponsibleRace, pEnemyRace, type);
#ifdef TRACE_INTEL
	MYTRACE(MT::LEVEL_INFO, "number of starting intel actions: %d\n", actions);
#endif
	return actions;
}

/// Funktion entfernt die durch eine Geheimdienstaktion verbrauchten Punkte auf Seiten des Geheimdienstopfers und
/// auf Seiten des Geheimdienstagressors.
void CIntelCalc::DeleteConsumedPoints(CMajor* pOurRace, CMajor* pEnemyRace, BOOLEAN isSpy, BYTE type, BOOLEAN isAttempt)
{
#ifdef TRACE_INTEL
	MYTRACE(MT::LEVEL_INFO, "CIntelCalc::DeleteConsumedPoints() begin...\n");
#endif

	CIntelligence* pOurIntel	= pOurRace->GetEmpire()->GetIntelligence();
	CIntelligence* pIntelEnemy	= pEnemyRace->GetEmpire()->GetIntelligence();


	// Punkte der gesammten Inneren Sicherheit des Opfers holen
	int enemyInnerSecPoints = pIntelEnemy->GetSecurityPoints() * pIntelEnemy->GetAssignment()->GetInnerSecurityPercentage() / 100;
	// + Bonus auf innere Sicherheit
	enemyInnerSecPoints += enemyInnerSecPoints * pIntelEnemy->GetInnerSecurityBoni() / 100;
	// beim Depot sind die Boni schon mit eingerechnet
	int enemyInnerSecDepot = pIntelEnemy->GetInnerSecurityStorage();
#ifdef TRACE_INTEL
	MYTRACE(MT::LEVEL_INFO, "enemies inner security points: %d - enemies inner security depot: %d\n", enemyInnerSecPoints, enemyInnerSecDepot);
#endif

	// nun die generierten Punkte des Agressors bestimmen
	int racePoints = 0;
	int raceDepot = 0;
	if (isSpy && !isAttempt)
	{
		racePoints = pOurIntel->GetSecurityPoints() * pOurIntel->GetAssignment()->GetGlobalSpyPercentage(pEnemyRace->GetRaceID())
			* pOurIntel->GetAssignment()->GetSpyPercentages(pEnemyRace->GetRaceID(), type) / 10000;
		racePoints += racePoints * pOurIntel->GetBonus(type, 0) / 100;
			
		raceDepot = pOurIntel->GetSPStorage(0, pEnemyRace->GetRaceID()) * pOurIntel->GetAssignment()->GetSpyPercentages(pEnemyRace->GetRaceID(), type) / 100;
		raceDepot += raceDepot * pOurIntel->GetBonus(type, 0) / 100;
	}
	else if (!isSpy && !isAttempt)
	{
		racePoints = pOurIntel->GetSecurityPoints() * pOurIntel->GetAssignment()->GetGlobalSabotagePercentage(pEnemyRace->GetRaceID())
			* pOurIntel->GetAssignment()->GetSabotagePercentages(pEnemyRace->GetRaceID(), type) / 10000;
		racePoints += racePoints * pOurIntel->GetBonus(type, 1) / 100;
			
		raceDepot = pOurIntel->GetSPStorage(1, pEnemyRace->GetRaceID()) * pOurIntel->GetAssignment()->GetSabotagePercentages(pEnemyRace->GetRaceID(), type) / 100;
		raceDepot += raceDepot * pOurIntel->GetBonus(type, 1) / 100;
	}
	else if (isAttempt)
	{
		raceDepot = pOurIntel->GetSPStorage(1, pEnemyRace->GetRaceID()) * pOurIntel->GetAssignment()->GetSabotagePercentages(pEnemyRace->GetRaceID(), type) / 100;
		raceDepot += raceDepot * pOurIntel->GetBonus(type, 1) / 100;
	}

#ifdef TRACE_INTEL
	MYTRACE(MT::LEVEL_INFO, "racePoints: %d - raceDepot: %d\n", racePoints, raceDepot);
#endif
	// jetzt gegenseitig die Punkte abziehen.
	// zuerst werden immer die Punkte abgezogen, welche nicht aus den Depots kommen
	int temp = racePoints + raceDepot - enemyInnerSecPoints;
	// in temp stehen jetzt nur noch die
	if (temp > 0)
		pIntelEnemy->AddInnerSecurityPoints(-temp);

	temp = enemyInnerSecPoints + enemyInnerSecDepot - racePoints;
	if (temp > 0)
		pOurIntel->AddSPStoragePoints(!isSpy, pEnemyRace->GetRaceID(), -temp);
}

/// Funktion ruft die jeweilige Unterfunktion auf, welche eine Geheimdienstaktion schlussendlich ausf�hrt.
BOOLEAN CIntelCalc::ExecuteAction(CMajor* pRace, CMajor* pEnemyRace, CMajor* pResponsibleRace, BYTE type, BOOLEAN isSpy)
{
	if (isSpy)
	{
		switch (type)
		{
		case 0: return this->ExecuteEconomySpy(pRace, pEnemyRace, pResponsibleRace, TRUE);
		case 1: return this->ExecuteScienceSpy(pRace, pEnemyRace, pResponsibleRace, TRUE);
		case 2: return this->ExecuteMilitarySpy(pRace, pEnemyRace, pResponsibleRace, TRUE);
		case 3: return this->ExecuteDiplomacySpy(pRace, pEnemyRace, pResponsibleRace, TRUE);
		}
	}
	else
	{
		switch (type)
		{
		case 0: return this->ExecuteEconomySabotage(pRace, pEnemyRace, pResponsibleRace);
		case 1: return this->ExecuteScienceSabotage(pRace, pEnemyRace, pResponsibleRace);
		case 2: return this->ExecuteMilitarySabotage(pRace, pEnemyRace, pResponsibleRace);
		case 3: return this->ExecuteDiplomacySabotage(pRace, pEnemyRace, pResponsibleRace);
		}
	}
	return FALSE;
}

/// Funktion f�hrt eine Wirtschatfsspionageaktion aus.
BOOLEAN CIntelCalc::ExecuteEconomySpy(CMajor* pRace, CMajor* pEnemyRace, CMajor* pResponsibleRace, BOOLEAN createText)
{
	/*
	Es gibt bis jetzt zwei verschiedene Arten der Wirtschaftsspionage. Zuerst kann das besitzte Latinum ausspioniert
	werden. Zweitens werden Geb�ude aus irgendeinem System ausspioniert.
	*/
	// 1. Versuch: Latinum ausspionieren
	if (rand()%8 == NULL)
	{
		int latinum = (int)pEnemyRace->GetEmpire()->GetLatinum();
		CEcoIntelObj* report = new CEcoIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, latinum);
		// Intelreport dem Akteur hinzuf�gen
		if (report)
		{
			if (createText)
				report->CreateText(m_pDoc, 1, pResponsibleRace->GetRaceID());
			pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
			return TRUE;
		}
		else
		{
			delete report;
			report = NULL;
			return FALSE;
		}
	}

	// 2. Versuch: Geb�ude ausspionieren
	// Die Spionage ist komplett zuf�llig. Man kann nicht beeinflussen in welchem System spioniert wird.
	CArray<CPoint> sectors;
	for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			if (m_pDoc->m_System[x][y].GetOwnerOfSystem() == pEnemyRace->GetRaceID())
				sectors.Add(CPoint(x,y));
	if (sectors.GetSize())
	{
		// in random steht ein zuf�llig ermittelter Sektor, in welchem spioniert werden kann.
		int random = rand()%sectors.GetSize();
		// bei der Wirtschaftsspionage kann man �ber alle Geb�ude, au�er Intelzentren und Laboratorien etwas erfahren.
		// Welches ausgew�hlt wird ist zuf�llig. Dabei werden mit etwas h�herer Wahrscheinlichkeit Geb�ude ausspioniert,
		// welche Arbeiter ben�tigen. Doch auch arbeiterfreie Geb�ude, wie z.B. eine Meeresfarm kann ermittelt werden.
		if (rand()%4 == NULL)	// zu 25% wird versucht ein arbeiterfreies Geb�ude zu spionieren
		{
			int number = m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetAllBuildings()->GetSize();
			int j = 0;
			if (number > NULL)
			{
				// zuf�llig ein Geb�ude aussuchen
				for (int i = rand()%number; i < number; i++)
				{
					CBuildingInfo buildingInfo = m_pDoc->GetBuildingInfo(m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetAllBuildings()->GetAt(i).GetRunningNumber());
					// wenn das Geb�ude keine Arbeiter ben�tigt
					if (buildingInfo.GetWorker() == FALSE)
					{
						// sobald das Geb�ude eine nicht wirtschaftsbezogene Leistung erbringen kann, kann es nicht spioniert werden
						if (!buildingInfo.GetBarrack() && !buildingInfo.GetFPProd() && !buildingInfo.GetGroundDefend() && !buildingInfo.GetGroundDefendBoni()
							&& !buildingInfo.GetMilitarySabotageBoni() && !buildingInfo.GetMilitarySpyBoni() && !buildingInfo.GetResearchBoni() && !buildingInfo.GetResearchSabotageBoni()
							&& !buildingInfo.GetResearchSpyBoni() && !buildingInfo.GetResistance() && !buildingInfo.GetScanPower() && !buildingInfo.GetScanPowerBoni()
							&& !buildingInfo.GetScanRange() && !buildingInfo.GetScanRangeBoni() && !buildingInfo.GetSecurityBoni() && !buildingInfo.GetShieldPower()
							&& !buildingInfo.GetShieldPowerBoni() && !buildingInfo.GetShipDefend() && !buildingInfo.GetShipDefendBoni() && !buildingInfo.GetShipTraining()
							&& !buildingInfo.GetShipYard() && !buildingInfo.GetSPProd() && !buildingInfo.GetTroopTraining())
						{
							int id = buildingInfo.GetRunningNumber();
							int n = m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetNumberOfBuilding(id);
							CEcoIntelObj* report = new CEcoIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(sectors.GetAt(random)), id, n);
							// Intelreport dem Akteur hinzuf�gen
							if (report)
							{
								if (createText)
									report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
								pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
								return TRUE;
							}
							else
							{
								delete report;
								report = NULL;
								break;
							}
						}
					}
					if (i == number-1)
						i = 0;
					j++;
					if (j == number)
						break;
				}
			}
		}
		// ansonsten werden Geb�ude welche Arbeiter ben�tigen ausspioniert
		int buildingTypes[IRIDIUM_WORKER+1] = {0};
		int start = rand()%(IRIDIUM_WORKER+1);
		int j = 0;
		for (int i = start; i <= IRIDIUM_WORKER; i++)
		{
			if (i != SECURITY_WORKER && i != RESEARCH_WORKER)
			{
				buildingTypes[i] = m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetNumberOfWorkbuildings(i, 0, NULL);
				if (buildingTypes[i] > NULL)
				{
					start = i;
					break;
				}
			}
			if (i == IRIDIUM_WORKER)
				i = FOOD_WORKER;
			j++;
			if (j == IRIDIUM_WORKER)
				break;
		}
		// in "start" steht m�glicherweise das Geb�ude zur Spionage
		if (buildingTypes[start] > 0)
		{
			CEcoIntelObj* report = new CEcoIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(sectors.GetAt(random)),
				m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetNumberOfWorkbuildings(start, 1, &m_pDoc->BuildingInfo), (BYTE)buildingTypes[start]);
			// Intelreport dem Akteur hinzuf�gen
			if (report)
			{
				if (createText)
					report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
				return TRUE;
			}
			else
			{
				delete report;
				report = NULL;
			}
		}		
	}
	return FALSE;
}

/// Funktion f�hrt eine Forschungsspionageaktion aus.
BOOLEAN CIntelCalc::ExecuteScienceSpy(CMajor* pRace, CMajor* pEnemyRace, CMajor* pResponsibleRace, BOOLEAN createText)
{
	/*
	Es gibt bis jetzt vier verschiedene Arten der Forschungsspionage. Zuerst k�nnen Forschungsgeb�ude aus irgendeinem System
	ausspioniert werden. Zweitens k�nnen die global fabrizierten Forschungspunkte ausspioniert werden. Drittens k�nnen
	einzelne Techlevel in Erfahrung gebracht werden und viertens wird etwas �ber die Spezialforschung spioniert.
	*/

	// 4. Versuch: etwas �ber die Spezialforschung herausbekommen
	if (rand()%8 == NULL)
	{
		short specialTech = -1;
		short choosen = -1;
		// zuf�llig mit einem Komplex starten und schauen ob dieser erforsch ist
		int t = rand()%NoUC;
		int j = 0;
		for (int i = t; i < NoUC; i++)
		{
			if (pEnemyRace->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(i)->GetComplexStatus() == RESEARCHED)
			{
				specialTech = i;
				for (int j = 1; j <= 3; j++)
					if (pEnemyRace->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(i)->GetFieldStatus(j) == RESEARCHED)
					{
						choosen = j;
						break;
					}
				break;
			}
			if (i == NoUC-1)
				i = 0;
			j++;
			if (j == NoUC)
				break;
		}
		// wenn das Imperium irgendeinen Komplex erforscht hat, dann einen Geheimdienstbericht anlegen
		if (specialTech != -1)
		{
			CScienceIntelObj* report = new CScienceIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, -1, -1, specialTech, choosen);
			// Intelreport dem Akteur hinzuf�gen
			if (report)
			{
				if (createText)
					report->CreateText(m_pDoc, 3, pResponsibleRace->GetRaceID());
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
				return TRUE;
			}
			else
			{
				delete report;
				report = NULL;
				return FALSE;
			}
		}
	}

	// 3. Versuch: etwas �ber ein bestimmtes Techlevel in Erfahrung bringen
	if (rand()%5 == NULL)
	{
		short techLevel = -1;
		short techType = rand()%6;
		switch (techType)
		{
		case 0: techLevel = pEnemyRace->GetEmpire()->GetResearch()->GetBioTech(); break;
		case 1: techLevel = pEnemyRace->GetEmpire()->GetResearch()->GetEnergyTech(); break;
		case 2: techLevel = pEnemyRace->GetEmpire()->GetResearch()->GetCompTech(); break;
		case 3: techLevel = pEnemyRace->GetEmpire()->GetResearch()->GetConstructionTech(); break;
		case 4: techLevel = pEnemyRace->GetEmpire()->GetResearch()->GetPropulsionTech(); break;
		case 5: techLevel = pEnemyRace->GetEmpire()->GetResearch()->GetWeaponTech(); break;
		}
		CScienceIntelObj* report = new CScienceIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, techLevel, techType, -1, -1);
		// Intelreport dem Akteur hinzuf�gen
		if (report)
		{
			if (createText)
				report->CreateText(m_pDoc, 2, pResponsibleRace->GetRaceID());
			pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
			return TRUE;
		}
		else
		{
			delete report;
			report = NULL;
			return FALSE;
		}
	}

	// 2. Versuch: global produzierte Forschungspunkte ausspionieren
	if (rand()%6 == NULL)
	{
		UINT fp = pEnemyRace->GetEmpire()->GetFP();
		CScienceIntelObj* report = new CScienceIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, fp);
		// Intelreport dem Akteur hinzuf�gen
		if (report)
		{
			if (createText)
				report->CreateText(m_pDoc, 1, pResponsibleRace->GetRaceID());
			pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
			return TRUE;
		}
		else
		{
			delete report;
			report = NULL;
			return FALSE;
		}
	}

	// wurde bis jetzt noch kein Versuch durchgef�hrt, so wird zwangsl�ufig der erste Versuch durchgef�hrt
	
	// 1. Versuch: Geb�ude ausspionieren
	// Die Spionage ist komplett zuf�llig. Man kann nicht beeinflussen in welchem System spioniert wird.
	CArray<CPoint> sectors;
	for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			if (m_pDoc->m_System[x][y].GetOwnerOfSystem() == pEnemyRace->GetRaceID())
				sectors.Add(CPoint(x,y));
	if (sectors.GetSize())
	{
		// in random steht ein zuf�llig ermittelter Sektor, in welchem spioniert werden kann.
		int random = rand()%sectors.GetSize();
		// bei der Forschungsspionage kann man nur etwas �ber Laboratorien oder andere Geb�ude erfahren, welche in irgendeinem
		// Zusammenhang mit Forschung stehen.
		// Welches ausgew�hlt wird ist zuf�llig. Dabei werden mit etwas h�herer Wahrscheinlichkeit Geb�ude ausspioniert,
		// welche Arbeiter ben�tigen. Doch auch arbeiterfreie Geb�ude, wie z.B. ein Theoriesimulator kann ermittelt werden.
		if (rand()%4 == NULL)	// zu 25% wird versucht ein arbeiterfreies Geb�ude zu spionieren
		{
			int number = m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetAllBuildings()->GetSize();
			int j = 0;
			if (number > NULL)
			{
				// zuf�llig ein Geb�ude aussuchen
				for (int i = rand()%number; i < number; i++)
				{
					CBuildingInfo buildingInfo = m_pDoc->GetBuildingInfo(m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetAllBuildings()->GetAt(i).GetRunningNumber());
					// wenn das Geb�ude keine Arbeiter ben�tigt
					if (buildingInfo.GetWorker() == FALSE)
					{
						const CBuilding* b = &m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetAllBuildings()->GetAt(i);
						// nur wenn das Geb�ude einen Forschungshintergrund hat, kann es spioniert werden
						if (buildingInfo.GetFPProd() > 0 || buildingInfo.GetResearchBoni() > 0 || buildingInfo.GetBioTechBoni() > 0 || buildingInfo.GetCompTechBoni() > 0 ||
							buildingInfo.GetConstructionTechBoni() > 0 || buildingInfo.GetEnergyTechBoni() > 0 || buildingInfo.GetPropulsionTechBoni() > 0 || 
							buildingInfo.GetWeaponTechBoni() > 0)
						{
							int id = buildingInfo.GetRunningNumber();
							int n = m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetNumberOfBuilding(id);
							CScienceIntelObj* report = new CScienceIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(sectors.GetAt(random)), id, n);
							// Intelreport dem Akteur hinzuf�gen
							if (report)
							{
								if (createText)
									report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
								pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
								return TRUE;
							}
							else
							{
								delete report;
								report = NULL;
								break;
							}
						}
					}
					if (i == number-1)
						i = 0;
					j++;
					if (j == number)
						break;
				}
			}
		}
		
		// ansonsten werden Geb�ude welche Arbeiter ben�tigen ausspioniert
		USHORT buildings = m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetNumberOfWorkbuildings(RESEARCH_WORKER, 0, NULL);
		if (buildings > 0)
		{
			CScienceIntelObj* report = new CScienceIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(sectors.GetAt(random)),
				m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetNumberOfWorkbuildings(RESEARCH_WORKER, 1, &m_pDoc->BuildingInfo), (BYTE)buildings);
			// Intelreport dem Akteur hinzuf�gen
			if (report)
			{
				if (createText)
					report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
				return TRUE;
			}
			else
			{
				delete report;
				report = NULL;
			}
		}
	}
	return FALSE;
}

/// Funktion f�hrt eine Milit�rspionageaktion aus.
BOOLEAN CIntelCalc::ExecuteMilitarySpy(CMajor* pRace, CMajor* pEnemyRace, CMajor* pResponsibleRace, BOOLEAN createText)
{
	/*
	Es gibt bis jetzt vier verschiedene Arten der Forschungsspionage. Zuerst k�nnen Milit�rgeb�ude und Intelzentren aus
	irgendeinem System ausspioniert werden. Zweitens k�nnen stationierte Truppen in einem System ausspioniert werden.
	Drittens k�nnen bestimmte Schiffe oder Au�enposten ausspioniert werden.
	*/
	// 3. Versuch: Schiffe und Stationen ausspionieren
	if (rand()%3 == NULL)	// zu 33% tritt dies ein
	{
		CArray<CShip*> ships;
		CArray<CShip*> stations;
		// Felder mit allen zu spionierenden Schiffe und Stationen anlegen
		for (int i = 0; i < m_pDoc->m_ShipArray.GetSize(); i++)
			if (m_pDoc->m_ShipArray[i].GetOwnerOfShip() == pEnemyRace->GetRaceID())
			{
				if (m_pDoc->m_ShipArray[i].GetShipType() == OUTPOST || m_pDoc->m_ShipArray[i].GetShipType() == STARBASE)
					stations.Add(&m_pDoc->m_ShipArray[i]);
				else
					ships.Add(&m_pDoc->m_ShipArray[i]);
			}
		CMilitaryIntelObj* report = NULL;
		// wenn Stationen vorhanden sind zu 33% dort spionieren
		if (stations.GetSize() > NULL && rand()%3 == NULL)
		{
			short t = rand()%stations.GetSize();
			report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(stations.GetAt(t)->GetKO()),
				stations.GetAt(t)->GetID(), 1, FALSE, TRUE, FALSE);
		}
		// ansonsten werden Schiffe ausspioniert
		else if (ships.GetSize() > NULL)
		{
			short t = rand()%ships.GetSize();
			USHORT number = 0;
			// Anzahl der Schiffe in dem Sektor ermitteln
			for (int i = 0; i < ships.GetSize(); i++)
				if (ships.GetAt(i)->GetKO() == ships.GetAt(t)->GetKO())
				{
					number++;
					if (ships.GetAt(i)->GetFleet())
						for (int j = 0; j < ships.GetAt(i)->GetFleet()->GetFleetSize(); j++)
							number++;
				}
			report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(ships.GetAt(t)->GetKO()),
				ships.GetAt(t)->GetID(), number, FALSE, TRUE, FALSE);
		}
		// Intelreport dem Akteur hinzuf�gen
		if (report)
		{
			if (createText)
				report->CreateText(m_pDoc, 2, pResponsibleRace->GetRaceID());
			pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
			return TRUE;
		}
		else
		{
			delete report;
			report = NULL;
		}	
	}

	// 2. Versuch: Stationierte Truppen ausspionieren
	if (rand()%4 == NULL)
	{
		// Systeme ermitteln, in denen �berhaupt Truppen stationiert sind
		CArray<CPoint> troopSectors;
		for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
			for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
				if (m_pDoc->m_System[x][y].GetOwnerOfSystem() == pEnemyRace->GetRaceID())
					if (m_pDoc->m_System[x][y].GetTroops()->GetSize())
						troopSectors.Add(CPoint(x,y));
		if (troopSectors.GetSize())
		{
			int random = rand()%troopSectors.GetSize();
			CMilitaryIntelObj* report = NULL;
			if (m_pDoc->m_System[troopSectors.GetAt(random).x][troopSectors.GetAt(random).y].GetTroops()->GetSize() == 1)
				report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(troopSectors.GetAt(random)),
				m_pDoc->m_System[troopSectors.GetAt(random).x][troopSectors.GetAt(random).y].GetTroops()->GetAt(0).GetID() + 20000, 1, FALSE, FALSE, TRUE);
			else
				report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(troopSectors.GetAt(random)),
					20000, m_pDoc->m_System[troopSectors.GetAt(random).x][troopSectors.GetAt(random).y].GetTroops()->GetSize(), FALSE, FALSE, TRUE);
			// Intelreport dem Akteur hinzuf�gen
			if (report)
			{
				if (createText)
					report->CreateText(m_pDoc, 1, pResponsibleRace->GetRaceID());
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
				return TRUE;
			}
			else
			{
				delete report;
				report = NULL;
				return FALSE;
			}
		}
	}

	// 1. Versuch: Geb�ude ausspionieren
	// Die Spionage ist komplett zuf�llig. Man kann nicht beeinflussen in welchem System spioniert wird.
	CArray<CPoint> sectors;
	for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			if (m_pDoc->m_System[x][y].GetOwnerOfSystem() == pEnemyRace->GetRaceID())
				sectors.Add(CPoint(x,y));
	if (sectors.GetSize())
	{
		// in random steht ein zuf�llig ermittelter Sektor, in welchem spioniert werden kann.
		int random = rand()%sectors.GetSize();
		// bei der Milit�rspionage kann man nur etwas �ber Intelzentren oder andere Geb�ude erfahren, welche in irgendeinem
		// Zusammenhang mit Milt�r oder Geheimdienst stehen.
		// Welches ausgew�hlt wird ist zuf�llig. Es werden mit gleicher Wahrscheinlichkeit Geb�ude ausspioniert,
		// welche Arbeiter und keine Arbeiter, wie z.B. eine Schiffswerft, ben�tigen. Wird kein arbeiterben�tigendes Geb�ude
		// gefunden, so wird versucht ein arbeiterfreies zu finden.
		if (rand()%3 == NULL)	// zu 33% wird versucht ein arbeiterben�tigendes Geb�ude zu spionieren
		{
			// ansonsten werden Geb�ude welche Arbeiter ben�tigen ausspioniert
			USHORT buildings = m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetNumberOfWorkbuildings(SECURITY_WORKER, 0, NULL);
			if (buildings > 0)
			{
				CMilitaryIntelObj* report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(sectors.GetAt(random)),
					m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetNumberOfWorkbuildings(SECURITY_WORKER, 1, &m_pDoc->BuildingInfo), (BYTE)buildings, TRUE, FALSE, FALSE);
				// Intelreport dem Akteur hinzuf�gen
				if (report)
				{
					if (createText)
						report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}			
		}

		int number = m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetAllBuildings()->GetSize();
		int j = 0;
		if (number > NULL)
		{
			// zuf�llig ein Geb�ude aussuchen
			for (int i = rand()%number; i < number; i++)
			{
				CBuildingInfo buildingInfo = m_pDoc->GetBuildingInfo(m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetAllBuildings()->GetAt(i).GetRunningNumber());
				// wenn das Geb�ude keine Arbeiter ben�tigt
				if (buildingInfo.GetWorker() == FALSE)
				{
					const CBuilding* b = &m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetAllBuildings()->GetAt(i);
					// nur wenn das Geb�ude einen Milit�r- oder Geheimdiensthintergrund hat, kann es spioniert werden
					if (buildingInfo.GetShipYard() || buildingInfo.GetSPProd() > NULL || buildingInfo.GetBarrack() || buildingInfo.GetBarrackSpeed() > NULL ||
						buildingInfo.GetEconomySabotageBoni() > NULL || buildingInfo.GetEconomySpyBoni() > NULL || buildingInfo.GetGroundDefend() > NULL ||
						buildingInfo.GetGroundDefendBoni() > NULL || buildingInfo.GetMilitarySabotageBoni() > NULL || buildingInfo.GetMilitarySpyBoni() > NULL ||
						buildingInfo.GetResistance() > NULL || buildingInfo.GetScanPower() > NULL || buildingInfo.GetScanPowerBoni() > NULL || buildingInfo.GetScanRange() > NULL ||
						buildingInfo.GetScanRangeBoni() > NULL || buildingInfo.GetSecurityBoni() > NULL || buildingInfo.GetShieldPower() > NULL ||
						buildingInfo.GetShieldPowerBoni() > NULL || buildingInfo.GetShipBuildSpeed() > NULL || buildingInfo.GetShipDefend() > NULL || buildingInfo.GetShipDefendBoni() > NULL ||
						buildingInfo.GetShipTraining() > NULL || buildingInfo.GetTroopBuildSpeed() > NULL || buildingInfo.GetTroopTraining() > NULL)						
					{
						int id = buildingInfo.GetRunningNumber();
						int n = m_pDoc->m_System[sectors.GetAt(random).x][sectors.GetAt(random).y].GetNumberOfBuilding(id);
						CMilitaryIntelObj* report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(sectors.GetAt(random)), id, n, TRUE, FALSE, FALSE);
						// Intelreport dem Akteur hinzuf�gen
						if (report)
						{
							if (createText)
								report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
							pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
							return TRUE;
						}
						else
						{
							delete report;
							report = NULL;
							break;
						}
					}
				}
				if (i == number-1)
					i = 0;
				j++;
				if (j == number)
					break;
			}
		}		
	}
	return FALSE;
}

/// Funktion f�hrt eine Diplomatiespionageaktion aus.
BOOLEAN CIntelCalc::ExecuteDiplomacySpy(CMajor* pRace, CMajor* pEnemyRace, CMajor* pResponsibleRace, BOOLEAN createText)
{
	/*
	Es gibt bis jetzt vier verschiedene Arten der Diplomatiespionage. Zuerst kann spioniert werden, ob eine Majorrace eine
	Minorrace kennt. Zweitens kann spioniert werden, welchen Vertrag eine Majorrace mit einer bekannten Minorrace aufrecht-
	erh�lt. Drittens kann spioniert werden, welchen Vertrag inkl. Rundendauer eine Majorrace mit einer anderen Majorrace
	besitzt und viertens kann in Erfahrung gebracht werden, welche Beziehung die Rassen untereinander haben.
	*/

	CArray<CString> minors;

	map<CString, CMinor*>* pmMinors = m_pDoc->GetRaceCtrl()->GetMinors();
	for (map<CString, CMinor*>::const_iterator it = pmMinors->begin(); it != pmMinors->end(); it++)
		if (pEnemyRace->IsRaceContacted(it->first))
			minors.Add(it->first);
	CArray<CString> majors;
	map<CString, CMajor*>* pmMajors = m_pDoc->GetRaceCtrl()->GetMajors();
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); it++)
		if (it->first != pRace->GetRaceID() && it->first != pEnemyRace->GetRaceID())
			if (pEnemyRace->IsRaceContacted(it->first))
				majors.Add(it->first);
	// 4. Versuch: Beziehungsspionage
	if (rand()%3 == NULL)
	{
		// entweder wird die Minorrace oder die Majorrace ausspioniert
		CDiplomacyIntelObj* report = NULL;
		if (rand()%2 == NULL && minors.GetSize())
		{
			CString minor = minors.GetAt(rand()%minors.GetSize());
			report = new CDiplomacyIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, (*pmMinors)[minor]->GetRaceKO(),
				NO_AGREEMENT, (*pmMinors)[minor]->GetRelation(pEnemyRace->GetRaceID()));
		}
		else if (majors.GetSize())
		{
			CString major = majors.GetAt(rand()%majors.GetSize());
			report = new CDiplomacyIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, major,
				NO_AGREEMENT, 0, (*pmMajors)[major]->GetRelation(pEnemyRace->GetRaceID()));
		}
		// Intelreport dem Akteur hinzuf�gen
		if (report)
		{
			if (createText)
				report->CreateText(m_pDoc, 3, pResponsibleRace->GetRaceID());
			pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
			return TRUE;
		}
		else
		{
			delete report;
			report = NULL;
			return FALSE;
		}
	}
	// 3. Versuch: Vertrag mit einer Majorrace ausspionieren
	if (rand()%4 == NULL)
	{
		if (majors.GetSize())
		{
			CString major = majors.GetAt(rand()%majors.GetSize());
			short agreement = NO_AGREEMENT;
			short duration = 0;
			// hier kann entweder ein normaler Vertrag, oder auch ein Verteidigungspakt spioniert werden
			if (rand()%3 == NULL && pEnemyRace->GetDefencePact(major))
			{
				agreement = DEFENCE_PACT;
				duration = pEnemyRace->GetDefencePactDuration(major);
			}
			else
			{
				agreement = pEnemyRace->GetAgreement(major);
				duration = pEnemyRace->GetAgreementDuration(major);
			}
			
			CDiplomacyIntelObj* report = new CDiplomacyIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, major, agreement,
				duration, (*pmMajors)[major]->GetRelation(pEnemyRace->GetRaceID()));
			// Intelreport dem Akteur hinzuf�gen
			if (report)
			{
				if (createText)
					report->CreateText(m_pDoc, 2, pResponsibleRace->GetRaceID());
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
				return TRUE;
			}
			else
			{
				delete report;
				report = NULL;
				return FALSE;
			}
		}
	}
	// 2. Versuch: Vertrag mit einer Minorrace ausspionieren
	if (rand()%2 == NULL)
	{
		if (minors.GetSize())
		{
			// in random steht eine zuf�llig ermittelte Minorrace, in welchem spioniert werden kann.
			CString minor = minors.GetAt(rand()%minors.GetSize());
			CDiplomacyIntelObj* report = new CDiplomacyIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE,
				(*pmMinors)[minor]->GetRaceKO(), (*pmMinors)[minor]->GetAgreement(pEnemyRace->GetRaceID()), (*pmMinors)[minor]->GetRelation(pEnemyRace->GetRaceID()));
			// Intelreport dem Akteur hinzuf�gen
			if (report)
			{
				if (createText)
					report->CreateText(m_pDoc, 1, pResponsibleRace->GetRaceID());
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
				return TRUE;
			}
			else
			{
				delete report;
				report = NULL;
				return FALSE;
			}
		}
	}
	// 1. Versuch: Kennen einer Minorrace ausspionieren
	if (minors.GetSize())
	{
		// in random steht eine zuf�llig ermittelte Minorrace, in welchem spioniert werden kann.
		CString minor = minors.GetAt(rand()%minors.GetSize());
		CDiplomacyIntelObj* report = new CDiplomacyIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE,
			(*pmMinors)[minor]->GetRaceKO());
		// Intelreport dem Akteur hinzuf�gen
		if (report)
		{
			if (createText)
				report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
			pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
			return TRUE;
		}
		else
		{
			delete report;
			report = NULL;
			return FALSE;
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AB HIER KOMMEN DIE GANZEN EINZELNEN SABOTAGEM�GLICHKEITEN
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Funktion f�hrt eine Wirtschatfssabotageaktion aus.
BOOLEAN CIntelCalc::ExecuteEconomySabotage(CMajor* pRace, CMajor* pEnemyRace, CMajor* pResponsibleRace, int reportNumber)
{
	// Es wird die zugeh�rige Spionageaktion durchgef�hrt. Diese generiert uns einen Geheimdienstreport.
	// Damit es schneller geht wird auf die Textgenerierung verzichtet. Sobald der neue Geheimdienstbericht
	// vorliegt wird aus dessen Informationen die Sabotageaktion generiert. Danach wird der alte Spionagebericht
	// entfernt und durch den neuen Sabotagebericht ersetzt.
	int oldReportNumber = reportNumber;
	int newReportNumber = oldReportNumber + 1;
	if (reportNumber == -1)
	{
		oldReportNumber = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports();
		this->ExecuteEconomySpy(pRace, pEnemyRace, FALSE);
		newReportNumber = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports();
	}
	// gab es eine erfolgreiche Spionageaktion, so kann diese jetzt in eine Sabotageaktion umgewandelt werden
	if (newReportNumber > oldReportNumber)
	{
		CEcoIntelObj* report = (CEcoIntelObj*)pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetReport(oldReportNumber);
		int latinum = report->GetLatinum() / 2;
		// 1. M�glichkeit: Latinum stehlen		
		if (latinum > NULL)
		{
			latinum = rand()%latinum + 1;
			pRace->GetEmpire()->SetLatinum(latinum);
			pEnemyRace->GetEmpire()->SetLatinum(-latinum);
			pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
			report = new CEcoIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, latinum);
			if (report)
			{
				report->CreateText(m_pDoc, 1, pResponsibleRace->GetRaceID());
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
				pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CEcoIntelObj(*report));
				return TRUE;
			}
			else
			{
				delete report;
				report = NULL;
				return FALSE;
			}
		}

		// 2. M�glichkeit: Geb�ude zerst�ren bzw. Bev�lkerung t�ten
		int buildings = report->GetNumber();
		if (buildings > NULL)
		{
			// zu 16.6% werden hier kein Geb�ude zerst�rt, sondern es werden direkt Nahrungsmittel
			// in einem System vergiftet, wodruch die Bev�lkerung stirbt
			if (rand()%6 == NULL)
			{
				CPoint ko = report->GetKO();
				BYTE currentHabs = (BYTE)m_pDoc->m_Sector[ko.x][ko.y].GetCurrentHabitants();
				// maximal die H�lfte der Bev�lkerung kann get�tet werden
				currentHabs /= 2;
				if (currentHabs > NULL)
				{
					currentHabs = rand()%currentHabs + 1;
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
					report = new CEcoIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, ko, 0, (BYTE)currentHabs);
					m_pDoc->m_Sector[ko.x][ko.y].LetPlanetsShrink(-(float)currentHabs);
					if (report)
					{
						report->CreateText(m_pDoc, 2, pResponsibleRace->GetRaceID());
						pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
						pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CEcoIntelObj(*report));
						return TRUE;
					}
					else
					{
						delete report;
						report = NULL;
						return FALSE;
					}
				}
			}
			// maximal die H�lfte der vorhanden Geb�ude k�nnen mit einem Mal zerst�rt werden
			buildings /= 2;
			// mindestens aber ein Geb�ude sollte zerst�rt werden
			if (buildings == NULL)
				buildings++;
			buildings = rand()%buildings + 1;
			CPoint ko = report->GetKO();
			int id = report->GetID();
			// jetzt die Geb�ude auf dem jeweiligen System zerst�ren
			int destroyed = 0;
			BuildingArray* allBuildings = m_pDoc->m_System[ko.x][ko.y].GetAllBuildings();
			for (int i = 0; i < allBuildings->GetSize(); i++)
				if (allBuildings->GetAt(i).GetRunningNumber() == report->GetID())
				{
					allBuildings->RemoveAt(i--);
					destroyed++;
					buildings--;
					if (buildings == NULL)
						break;
				}
			if (destroyed > NULL)
			{
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
				report = new CEcoIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, ko, id, destroyed);
				if (report)
				{
					report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CEcoIntelObj(*report));
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}
		}
		pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
	}	
	return FALSE;
}

/// Funktion f�hrt eine Forschungssabotageaktion aus.
BOOLEAN CIntelCalc::ExecuteScienceSabotage(CMajor* pRace, CMajor* pEnemyRace, CMajor* pResponsibleRace, int reportNumber)
{
	// Es wird die zugeh�rige Spionageaktion durchgef�hrt. Diese generiert uns einen Geheimdienstreport.
	// Damit es schneller geht wird auf die Textgenerierung verzichtet. Sobald der neue Geheimdienstbericht
	// vorliegt wird aus dessen Informationen die Sabotageaktion generiert. Danach wird der alte Spionagebericht
	// entfernt und durch den neuen Sabotagebericht ersetzt.
	int oldReportNumber = reportNumber;
	int newReportNumber = oldReportNumber + 1;
	if (reportNumber == -1)
	{
		oldReportNumber = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports();
		this->ExecuteScienceSpy(pRace, pEnemyRace, FALSE);
		newReportNumber = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports();
	}
	// gab es eine erfolgreiche Spionageaktion, so kann diese jetzt in eine Sabotageaktion umgewandelt werden
	if (newReportNumber > oldReportNumber)
	{
		CScienceIntelObj* report = (CScienceIntelObj*)pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetReport(oldReportNumber);

		// 4. Versuch: Spezialforschungen
		// ->	kann nicht sabotiert werden, d.h. wenn der Report etwas mit Spezialforschungen zu tun hatte, dann wird er
		//		sofort gel�scht und es wird versucht einen neuen zu erstellen. Solange bis nix mit Spezialforschungen drin
		//		vorkommt.
		int specialTech = report->GetSpecialTechComplex();
		while (specialTech != -1)
		{
			pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
			oldReportNumber = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports();
			BOOLEAN isTrue = FALSE;
			do {
				isTrue = this->ExecuteScienceSpy(pRace, pEnemyRace, FALSE);
			} while (isTrue == FALSE);
			report = (CScienceIntelObj*)pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetReport(oldReportNumber);
			specialTech = report->GetSpecialTechComplex();
			
		} 

		// 3. Versuch: einen Techbereich sabotieren
		int techType = report->GetTechType();
		if (techType != -1)
		{
			// in diesem Gebiet die prozentuale Erforschung einfach wieder niedriger ansetzen
			UINT currentFP = 0;
			switch (techType)
			{
			case 0: currentFP = pEnemyRace->GetEmpire()->GetResearch()->GetBioFP(); break;
			case 1: currentFP = pEnemyRace->GetEmpire()->GetResearch()->GetEnergyFP(); break;
			case 2: currentFP = pEnemyRace->GetEmpire()->GetResearch()->GetComputerFP(); break;
			case 3: currentFP = pEnemyRace->GetEmpire()->GetResearch()->GetConstructionFP(); break;
			case 4: currentFP = pEnemyRace->GetEmpire()->GetResearch()->GetPropulsionFP(); break;
			case 5: currentFP = pEnemyRace->GetEmpire()->GetResearch()->GetWeaponFP(); break;
			}
			// die aktuell schon erforschten Forschungspunkte zuf�llig neu bestimmen [0, aktuell erforschte FP]
			if (currentFP > NULL)
			{
				currentFP = rand()%currentFP + 1;
				short techLevel = report->GetTechLevel();
				pEnemyRace->GetEmpire()->GetResearch()->SetFP(techType, currentFP);
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
				report = new CScienceIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, techLevel, techType, -1, -1);
				if (report)
				{
					report->CreateText(m_pDoc, 2, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CScienceIntelObj(*report));
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}
		}
		// 2. Versuch: globale Forschungspunkte stehlen
		int fp = report->GetFP();
		if (fp > (int)pEnemyRace->GetEmpire()->GetFP())
			fp = pEnemyRace->GetEmpire()->GetFP();
		if (fp > NULL)
		{
			pRace->GetEmpire()->AddFP(fp);
			pEnemyRace->GetEmpire()->AddFP(-fp);
			pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
			report = new CScienceIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, fp);
			if (report)
			{
				report->CreateText(m_pDoc, 1, pResponsibleRace->GetRaceID());
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
				pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CScienceIntelObj(*report));
				return TRUE;
			}
			else
			{
				delete report;
				report = NULL;
				return FALSE;
			}
		}	
		// 1. Versuch: Forschungsgeb�ude zerst�ren
		int buildings = report->GetNumber();
		if (buildings > NULL)
		{
			// maximal die H�lfte der vorhanden Geb�ude k�nnen mit einem Mal zerst�rt werden
			buildings /= 2;
			// mindestens aber ein Geb�ude sollte zerst�rt werden
			if (buildings == NULL)
				buildings++;
			buildings = rand()%buildings + 1;
			CPoint ko = report->GetKO();
			int id = report->GetID();
			int destroyed = 0;
			// jetzt die Geb�ude auf dem jeweiligen System zerst�ren
			BuildingArray* allBuildings = m_pDoc->m_System[ko.x][ko.y].GetAllBuildings();
			for (int i = 0; i < allBuildings->GetSize(); i++)
				if (allBuildings->GetAt(i).GetRunningNumber() == report->GetID())
				{
					allBuildings->RemoveAt(i--);
					buildings--;
					destroyed++;
					if (buildings == NULL)
						break;
				}
			if (destroyed > NULL)
			{
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
				report = new CScienceIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, ko, id, destroyed);
				if (report)
				{
					report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CScienceIntelObj(*report));
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}
		}
		pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
	}
	return FALSE;
}

/// Funktion f�hrt eine Milit�rsabotageaktion aus.
BOOLEAN CIntelCalc::ExecuteMilitarySabotage(CMajor* pRace, CMajor* pEnemyRace, CMajor* pResponsibleRace, int reportNumber)
{
	// Es wird die zugeh�rige Spionageaktion durchgef�hrt. Diese generiert uns einen Geheimdienstreport.
	// Damit es schneller geht wird auf die Textgenerierung verzichtet. Sobald der neue Geheimdienstbericht
	// vorliegt wird aus dessen Informationen die Sabotageaktion generiert. Danach wird der alte Spionagebericht
	// entfernt und durch den neuen Sabotagebericht ersetzt.
	int oldReportNumber = reportNumber;
	int newReportNumber = oldReportNumber + 1;
	if (reportNumber == -1)
	{
		oldReportNumber = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports();
		this->ExecuteMilitarySpy(pRace, pEnemyRace, FALSE);
		newReportNumber = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports();
	}
	// gab es eine erfolgreiche Spionageaktion, so kann diese jetzt in eine Sabotageaktion umgewandelt werden
	if (newReportNumber > oldReportNumber)
	{
		CMilitaryIntelObj* report = (CMilitaryIntelObj*)pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetReport(oldReportNumber);

		// 3. Versuch: Schiffe besch�digen/zerst�ren/stehlen oder Stationen besch�digen/zerst�ren
		if (report->GetIsShip())
		{
			CShip* ship = NULL;
			// �berpr�fen, ob die jeweilige Station oder das jeweilige Schiff auch noch im System vorhanden ist
			CArray<CPoint> allShips;	// x f�r Schiffsposition im Feld, y f�r Schiffsposition in der Flotte
			for (int i = 0; i < m_pDoc->m_ShipArray.GetSize(); i++)
				if (m_pDoc->m_ShipArray.GetAt(i).GetKO() == report->GetKO() && m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip() != report->GetOwner())
				{
					// besitzt dieses Schiff eine Flotte, so k�nnte sich unser Schiff auch in der Flotte befinden
					if (m_pDoc->m_ShipArray.GetAt(i).GetFleet())
					{
						for (int j = 0; j < m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetFleetSize(); j++)
							if (m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetPointerOfShipFromFleet(j)->GetID() == report->GetID())
								allShips.Add(CPoint(i,j));
					}
					if (m_pDoc->m_ShipArray.GetAt(i).GetID() == report->GetID())
						allShips.Add(CPoint(i,-1));	// -1 als y Wert bedeutet, dass dieses Schiff in keiner Flotte vorkommt
				}
			// aus allen m�glichen Schiffen, welche die ID unseres Reports und im richtigen Sektor sind eins zuf�llig aussuchen
			CPoint n = CPoint(-1,0);
			if (allShips.GetSize() > NULL)
			{
				int random = rand()%allShips.GetSize();
				n = allShips.GetAt(random);
				// ist das Schiff in keiner Flotte sondern direkt im Feld zu finden
				if (n.y == -1)
					ship = &m_pDoc->m_ShipArray.GetAt(n.x);
				else
					ship = m_pDoc->m_ShipArray.GetAt(n.x).GetFleet()->GetPointerOfShipFromFleet(n.y);
			}
			// wurde kein Schiff mehr in diesem Sektor gefunden, sei es da es zerst�rt wurde oder jetzt in einem anderen
			// Sektor ist, kann es auch nicht mehr zerst�rt werden.
			if (ship == NULL)
				return FALSE;

			// Haben wir das alles �berstanden, so k�nnen wir jetzt ein Schiff/Station sabotieren und wissen gleichzeitig,
			// an welcher Position in welchem Feld (ShipArray oder Flotte eines Schiffes) es sich befindet.
			
			// eine Station wird entweder besch�digt oder mit geringerer Wahrscheinlichkeit komplett zerst�rt
			// eine Schiff wird entweder besch�digt, mit geringerer Wahrscheinlichkeit komplett zerst�rt oder sogar
			// gestohlen
			
			// Schiff stehlen
			if (rand()%2 == NULL && ship->GetShipType() != OUTPOST && ship->GetShipType() != STARBASE)
			{
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
				report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, ship->GetKO(), ship->GetID(), 1, FALSE, TRUE, FALSE);
				// die Station aus der ShipHistory der aktuellen Schiffe entfernen und den zerst�rten Schiffen hinzuf�gen
				pEnemyRace->GetShipHistory()->ModifyShip(ship, m_pDoc->m_Sector[ship->GetKO().x][ship->GetKO().y].GetName(TRUE),
					m_pDoc->GetCurrentRound(), CResourceManager::GetString("SABOTAGE"), CResourceManager::GetString("MISSED"));
				
				// neuen Besitzer hinzuf�gen
				ship->SetOwnerOfShip(pRace->GetRaceID());
				ship->SetIsShipFlagShip(FALSE);
				ship->SetCurrentOrder(AVOID);
				// gestohlenes Schiff zum n�chsten eigenen System verschieben
				CPoint oldKO = ship->GetKO();
				CPoint newKO = ship->GetKO();
				short minDist = MAXSHORT;
				for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
					for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
						if (m_pDoc->m_Sector[x][y].GetShipPort(pRace->GetRaceID()))
							if (minDist > min(abs(oldKO.x - x), abs(oldKO.y - y)))
							{
								minDist = (short)min(abs(oldKO.x - x), abs(oldKO.y - y));
								newKO = CPoint(x,y);
							}
				ship->SetKO(newKO);
				ship->SetTargetKO(newKO, 0);
				// wurde dieses Schiff jedoch schonmal gestohlen, dann ist es in der Missed Shiphistory. Ist dies der Fall kann das Schiff
				// wieder als aktives Schiff betrachtet werden.
				if (pRace->GetShipHistory()->ModifyShip(ship, m_pDoc->m_Sector[ship->GetKO().x][ship->GetKO().y].GetName(TRUE), 0) == false)
					// dem neuen Besitzer das Schiff als aktives Schiff hinzuf�gen
					pRace->GetShipHistory()->AddShip(ship, m_pDoc->m_Sector[ship->GetKO().x][ship->GetKO().y].GetName(TRUE), m_pDoc->GetCurrentRound());

				// jetzt Dinge wegen einer m�glichen Flotte beachten
				if (n.y == -1)		// Schiff nicht in Flotte
				{
					// das Schiff selbst kann aber eine Flotte anf�hren
					// Wenn das Schiff eine Flotte besa�, so geht die Flotte auf das erste Schiff in der Flotte �ber
					if (ship->GetFleet())
					{
						CFleet f = *(m_pDoc->m_ShipArray[n.x].GetFleet());
						// Flotte l�schen
						m_pDoc->m_ShipArray[n.x].GetFleet()->DeleteFleet();
						m_pDoc->m_ShipArray[n.x].DeleteFleet();
						// Nun die Flotte auf das n�chste Schiff �bergeben, daf�r das erste Schiff in der Flotte rausnehmen
						m_pDoc->m_ShipArray.Add(f.RemoveShipFromFleet(0));
						// f�r dieses eine Flotte erstellen
						m_pDoc->m_ShipArray[m_pDoc->m_ShipArray.GetUpperBound()].CreateFleet();
						for (USHORT i = 0; i < f.GetFleetSize(); i++)
							m_pDoc->m_ShipArray[m_pDoc->m_ShipArray.GetUpperBound()].GetFleet()->AddShipToFleet(f.GetShipFromFleet(i));
						m_pDoc->m_ShipArray[m_pDoc->m_ShipArray.GetUpperBound()].CheckFleet();
					}					
				}
				else	// Schiff ist in Flotte
				{
					m_pDoc->m_ShipArray.Add(m_pDoc->m_ShipArray.GetAt(n.x).GetFleet()->RemoveShipFromFleet(n.y));
					m_pDoc->m_ShipArray.GetAt(n.x).CheckFleet();
				}
				if (report)
				{
					report->CreateText(m_pDoc, 3, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CMilitaryIntelObj(*report));
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}
			// Schiff/Station zerst�ren
			else if ((rand()%4 == NULL && (ship->GetShipType() == OUTPOST || ship->GetShipType() == STARBASE))
				|| (rand()%2 == NULL && ship->GetShipType() != OUTPOST && ship->GetShipType() != STARBASE))
			{
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
				report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, ship->GetKO(), ship->GetID(), 1, FALSE, TRUE, FALSE);
				// die Station aus der ShipHistory der aktuellen Schiffe entfernen und den zerst�rten Schiffen hinzuf�gen
				pEnemyRace->GetShipHistory()->ModifyShip(ship, m_pDoc->m_Sector[ship->GetKO().x][ship->GetKO().y].GetName(TRUE),
					m_pDoc->GetCurrentRound(), CResourceManager::GetString("SABOTAGE"), CResourceManager::GetString("DESTROYED"));					
				if (n.y == -1)		// nicht in Flotte
				{
					// das Schiff selbst kann aber eine Flotte anf�hren
					// Wenn das Schiff eine Flotte besa�, so geht die Flotte auf das erste Schiff in der Flotte �ber
					if (ship->GetFleet())
					{
						BYTE oldOrder = m_pDoc->m_ShipArray.GetAt(n.x).GetCurrentOrder();
						CFleet f = *(m_pDoc->m_ShipArray.GetAt(n.x).GetFleet());
						// Flotte l�schen
						m_pDoc->m_ShipArray.GetAt(n.x).GetFleet()->DeleteFleet();
						m_pDoc->m_ShipArray.GetAt(n.x).DeleteFleet();
						// Nun die Flotte auf das n�chste Schiff �bergeben, daf�r das erste Schiff in der Flotte rausnehmen
						m_pDoc->m_ShipArray.SetAt(n.x, f.RemoveShipFromFleet(0));
						// f�r dieses eine Flotte erstellen
						m_pDoc->m_ShipArray.GetAt(n.x).CreateFleet();
						for (USHORT x = 0; x < f.GetFleetSize(); x++)
							m_pDoc->m_ShipArray.GetAt(n.x).GetFleet()->AddShipToFleet(f.GetShipFromFleet(x));
						m_pDoc->m_ShipArray.GetAt(n.x).CheckFleet();
						// Brauchen das alte Schiff hier auch nicht l�schen, da es mit einem neuen �berschrieben wurde
						m_pDoc->m_ShipArray.GetAt(n.x).SetCurrentOrder(oldOrder);
					}
					// ansonsten kann es einfach gel�scht werden
					else
						m_pDoc->m_ShipArray.RemoveAt(n.x);
				}
				else	// Schiff ist in Flotte
				{
					m_pDoc->m_ShipArray.GetAt(n.x).GetFleet()->RemoveShipFromFleet(n.y);
					m_pDoc->m_ShipArray.GetAt(n.x).CheckFleet();
				}
				if (report)
				{
					report->CreateText(m_pDoc, 2, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CMilitaryIntelObj(*report));
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}
			// Schiff/Station besch�digen
			else
			{
				UINT hull = ship->GetHull()->GetCurrentHull();
				if (hull > NULL)
				{
					UINT newhull = rand()%hull + 1;
					ship->GetHull()->SetCurrentHull(newhull - hull);
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
					report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, ship->GetKO(), ship->GetID(), 1, FALSE, TRUE, FALSE);
					if (report)
					{
						report->CreateText(m_pDoc, 4, pResponsibleRace->GetRaceID());
						pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
						pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CMilitaryIntelObj(*report));
						return TRUE;
					}
					else
					{
						delete report;
						report = NULL;
						return FALSE;
					}
				}
			}
		}		
		
		// 2. Versuch: Stationierte Truppen umbringen
		int troopNumber = report->GetNumber();
		if (troopNumber > NULL && report->GetIsTroop())
		{
			// maximal die H�lfte der vorhanden Truppen k�nnen mit einem Mal zerst�rt werden
			troopNumber /= 2;
			// mindestens aber eine vorhandene Truppe sollte zerst�rt werden
			if (troopNumber == NULL)
				troopNumber++;
			troopNumber = rand()%troopNumber + 1;
			CPoint ko = report->GetKO();
			int id = report->GetID();
			int destroyed = 0;
			// jetzt die Truppe/Truppen auf dem jeweiligen System zerst�ren
			CArray<CTroop>* allTroops = m_pDoc->m_System[ko.x][ko.y].GetTroops();
			for (int i = 0; i < allTroops->GetSize(); i++)
			{
				allTroops->RemoveAt(i--);
				troopNumber--;
				destroyed++;
				if (troopNumber == NULL)
					break;
			}
			if (destroyed > NULL)
			{
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
				report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, ko, id, destroyed, FALSE, FALSE, TRUE);
				if (report)
				{
					report->CreateText(m_pDoc, 1, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CMilitaryIntelObj(*report));
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}
		}
		// 1. Versuch: Milit�rgeb�ude zerst�ren
		int buildings = report->GetNumber();
		if (buildings > NULL && report->GetIsBuilding())
		{
			// maximal die H�lfte der vorhanden Geb�ude k�nnen mit einem Mal zerst�rt werden
			buildings /= 2;
			// mindestens aber ein Geb�ude sollte zerst�rt werden
			if (buildings == NULL)
				buildings++;
			int n = buildings = rand()%buildings + 1;
			CPoint ko = report->GetKO();
			int id = report->GetID();
			int destroyed = 0;
			// jetzt die Geb�ude auf dem jeweiligen System zerst�ren
			BuildingArray* allBuildings = m_pDoc->m_System[ko.x][ko.y].GetAllBuildings();
			for (int i = 0; i < allBuildings->GetSize(); i++)
				if (allBuildings->GetAt(i).GetRunningNumber() == report->GetID())
				{
					allBuildings->RemoveAt(i--);
					buildings--;
					destroyed++;
					if (buildings == NULL)
						break;
				}
			if (destroyed > NULL)
			{
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
				report = new CMilitaryIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, ko, id, destroyed, TRUE, FALSE, FALSE);
				if (report)
				{
					report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CMilitaryIntelObj(*report));
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}
		}
		pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
	}	
	return FALSE;
}

/// Funktion f�hrt eine Diplomatiesabotageaktion aus.
BOOLEAN CIntelCalc::ExecuteDiplomacySabotage(CMajor* pRace, CMajor* pEnemyRace, CMajor* pResponsibleRace, int reportNumber)
{
	// Es wird die zugeh�rige Spionageaktion durchgef�hrt. Diese generiert uns einen Geheimdienstreport.
	// Damit es schneller geht wird auf die Textgenerierung verzichtet. Sobald der neue Geheimdienstbericht
	// vorliegt wird aus dessen Informationen die Sabotageaktion generiert. Danach wird der alte Spionagebericht
	// entfernt und durch den neuen Sabotagebericht ersetzt.
	int oldReportNumber = reportNumber;
	int newReportNumber = oldReportNumber + 1;
	if (reportNumber == -1)
	{
		oldReportNumber = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports();
		this->ExecuteDiplomacySpy(pRace, pEnemyRace, FALSE);
		newReportNumber = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports();
	}
	// gab es eine erfolgreiche Spionageaktion, so kann diese jetzt in eine Sabotageaktion umgewandelt werden
	if (newReportNumber > oldReportNumber)
	{
		CDiplomacyIntelObj* report = (CDiplomacyIntelObj*)pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetReport(oldReportNumber);
		// Bei der Diplomatiesabotage kann entweder die Beziehung zwischen zwei Majorraces oder zwischen
		// einer Majorrace und einer Minorrace verschlechtert werden. Eine andere M�glichkeit w�re die Beziehung
		// zwischen uns und einer Minorrace oder einer Majorrace zu verbessern.
		
		// zwischen Major <-> Major
		if (report->GetMajorRaceID() != "")
		{
			CString major = report->GetMajorRaceID();
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pDoc->GetRaceCtrl()->GetRace(major));
			ASSERT(pMajor);
			// hier zwei M�glichkeiten: verschlechtern der Beziehung zwischen Geheimdienstopfer und Major oder Verbesserung
			// der Beziehung zwischen uns und dem Geheimdienstopfer
			
			// zu uns selbst verbessern
			if (rand()%5 == NULL)
			{
				int relationAdd = rand()%20 + 1;
				pEnemyRace->SetRelation(pRace->GetRaceID(), relationAdd);
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
				report = new CDiplomacyIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, pEnemyRace->GetRaceID(), FALSE, FALSE, FALSE);
				if (report)
				{
					report->CreateText(m_pDoc, 0, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CDiplomacyIntelObj(*report));
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}
			// zwischen zwei Majors verschlechtern
			else
			{
				int relationSub = -rand()%20 + 1;
				// falls nur computergesteuerte Rassen an der Aktion beteiligt sind, so wird die Beziehung nur um ein
				// Viertel verschlechtert. Liegt ganz einfach daran, dass sich die Computergegner nicht alle immer
				// untereinadner plattmachen
				if (pRace->IsHumanPlayer() == false && pEnemyRace->IsHumanPlayer() == false	&& pMajor->IsHumanPlayer() == false)
					relationSub /= 4;
				pEnemyRace->SetRelation(pMajor->GetRaceID(), relationSub);
				pMajor->SetRelation(pEnemyRace->GetRaceID(), relationSub);
				pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
				report = new CDiplomacyIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, major, FALSE, FALSE, FALSE);
				if (report)
				{
					report->CreateText(m_pDoc, 1, pResponsibleRace->GetRaceID());
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CDiplomacyIntelObj(*report));
					return TRUE;
				}
				else
				{
					delete report;
					report = NULL;
					return FALSE;
				}
			}			
		}
		// zwischen Major und Minor
		else
		{
			CMinor* minor = m_pDoc->GetRaceCtrl()->GetMinorRace(m_pDoc->GetSector(report->GetMinorRaceKO()).GetName());
			if (minor)
			{
				// hier zwei M�glichkeiten: verschlechtern der Beziehung zwischen Geheimdienstopfer und Minor oder Verbesserung
				// der Beziehung zwischen uns und der Minor
				// zu uns selbst verbessern -> kein Bericht bei anderer Majorrace
				if (rand()%2 == NULL && minor->IsRaceContacted(pRace->GetRaceID()) == true && minor->GetRelation(pRace->GetRaceID()) < 90)
				{
					int relationAdd = rand()%20 + 1;
					minor->SetRelation(pRace->GetRaceID(), relationAdd);
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
					report = new CDiplomacyIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, minor->GetRaceKO());
					if (report)
					{
						report->CreateText(m_pDoc, 2, pResponsibleRace->GetRaceID());
						pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
						return TRUE;
					}
					else
					{
						delete report;
						report = NULL;
						return FALSE;
					}					
				}
				// bei unserem gegen�ber verschlechtern
				else
				{
					int relationSub = rand()%20 + 1;
					minor->SetRelation(pEnemyRace->GetRaceID(), relationSub);
					pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
					report = new CDiplomacyIntelObj(pRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), FALSE, minor->GetRaceKO());
					if (report)
					{
						report->CreateText(m_pDoc, 3, pResponsibleRace->GetRaceID());
						pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);
						pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(new CDiplomacyIntelObj(*report));
						return TRUE;
					}
					else
					{
						delete report;
						report = NULL;
						return FALSE;
					}
				}
			}
		}
		pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveReport(oldReportNumber);
	}
	return FALSE;
}

/// Funktion erstellt den Report, welcher aussagt, dass versucht wurde eine Rasse auszuspionieren/auszusabotieren.
void CIntelCalc::CreateMsg(CMajor* pResponsibleRace, CMajor* pEnemyRace, BYTE type)
{
	CString csInput;													// auf csInput wird die jeweilige Zeile gespeichert
	CString fileName;
	fileName = *((CBotf2App*)AfxGetApp())->GetPath() + "Data\\Races\\MajorIntel.data";				// Name des zu �ffnenden Files 
	CStdioFile file;													// Varibale vom Typ CStdioFile
	if (file.Open(fileName, CFile::modeRead | CFile::typeText))			// Datei wird ge�ffnet
	{
		while (file.ReadString(csInput))
		{
			int pos = 0;
			CString s = csInput.Tokenize(":", pos);
			// Rasse bestimmen
			if (s == pEnemyRace->GetRaceID())
			{
				csInput.Delete(0, pos);
				// in csInput steht nun die Beschreibung
				// Jetzt m�ssen noch die Variablen mit dem richtigen Text gef�llt werden
				s = pResponsibleRace->GetEmpireNameWithAssignedArticle();
				csInput.Replace("$race$", s);
				CIntelObject* report = NULL;
				switch (type)
				{
				case 0: report = new CEcoIntelObj(pResponsibleRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, 0); break;
				case 1: report = new CScienceIntelObj(pResponsibleRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, 0); break;
				case 2: report = new CMilitaryIntelObj(pResponsibleRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(-1,-1), 0, 0, FALSE, FALSE, FALSE); break;
				case 3: report = new CDiplomacyIntelObj(pResponsibleRace->GetRaceID(), pEnemyRace->GetRaceID(), m_pDoc->GetCurrentRound(), TRUE, CPoint(-1,-1)); break;
				}
				if (report)
				{
					report->SetEnemyDesc(csInput);
					pEnemyRace->GetEmpire()->GetIntelligence()->GetIntelReports()->AddReport(report);						
				}
				break;
			}			
		}
	}
	else
	{	
		AfxMessageBox("Error! Could not open file \"MajorIntel.data\"...");
		exit(1);
	}
	file.Close();
}

/// Funktion f�hrt einen Anschlag durch.
BOOLEAN CIntelCalc::ExecuteAttempt(CMajor* pRace, UINT ourSP)
{
	ASSERT(pRace);
	CMajor* pResponsibleRace = pRace;
	// Ist ein Anschlagsobjekt vorhanden?
	if (pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetAttemptObject())
	{
		CIntelObject* attemptObj = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetAttemptObject();
		CMajor* pEnemy = dynamic_cast<CMajor*>(m_pDoc->GetRaceCtrl()->GetRace(attemptObj->GetEnemy()));
		if (!pEnemy)
			return false;
		this->DeleteConsumedPoints(pRace, pEnemy, FALSE, attemptObj->GetType(), TRUE);
		// bei Anschl�gen werden die ben�tigten Punkte etwas reduziert, indem hier einfach gesagt wird, dass es sich
		// um Spionage handeln w�rde.
		if (IsSuccess(pEnemy, ourSP, TRUE, pResponsibleRace, attemptObj->GetType()))
		{
			// Die Moral verschlechtert sich beim betroffenen Imperium gegeb�ber der responsible Race
			if (pResponsibleRace)
				pEnemy->SetRelation(pResponsibleRace->GetRaceID(), -rand()%10);
			// wenn der Anschlag erfolgreich durchgef�hrt werden konnte, muss der dazugeh�rige Spionagereport aus dem
			// Feld gel�scht werden
			for (int i = 0; i < pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetNumberOfReports(); i++)
			{
				CIntelObject* intelObj = pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->GetReport(i);
				// Sind immer Spionageobjekte
				if (intelObj->GetIsSpy())
					if (intelObj->GetRound() == attemptObj->GetRound())
						if (intelObj->GetEnemy() == attemptObj->GetEnemy())
							if (intelObj->GetType() == attemptObj->GetType())
								if (*intelObj->GetOwnerDesc() == *attemptObj->GetOwnerDesc())
								{
									BOOLEAN returnValue = FALSE;
									switch (attemptObj->GetType())
									{
									case 0: returnValue = this->ExecuteEconomySabotage(pRace, pEnemy, pResponsibleRace, i); break;
									case 1: returnValue = this->ExecuteScienceSabotage(pRace, pEnemy, pResponsibleRace, i); break;
									case 2: returnValue = this->ExecuteMilitarySabotage(pRace, pEnemy, pResponsibleRace, i); break;
									case 3: returnValue = this->ExecuteDiplomacySabotage(pRace, pEnemy, pResponsibleRace, i); break;
									}
									intelObj = NULL;
									pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveAttemptObject();
									return returnValue;									
								}						
			}			
		}
	}
	pRace->GetEmpire()->GetIntelligence()->GetIntelReports()->RemoveAttemptObject();
	return FALSE;
}

/// Funktion gibt die aktuell komplett generierten inneren Sicherheitspunkte eines Imperiums zur�ck.
UINT CIntelCalc::GetCompleteInnerSecPoints(CMajor* pEnemyRace)
{
	ASSERT(pEnemyRace);

	CIntelligence* pIntel = pEnemyRace->GetEmpire()->GetIntelligence();

	// Depot der inneren Sicherheitspunkte + aktuell produzierte Sicherheitspunkte * prozentuale Zuweisung + Bonus auf innere Sicherheit + prozentuale Zuteilung
	UINT enemyInnerSec = pIntel->GetSecurityPoints() * pIntel->GetAssignment()->GetInnerSecurityPercentage() / 100;
	// + Bonus auf innere Sicherheit
	enemyInnerSec += enemyInnerSec * pIntel->GetInnerSecurityBoni() / 100;
	// + prozentuale Zuteilung
	enemyInnerSec += pIntel->GetAssignment()->GetInnerSecurityPercentage();
	// + Punkte aus dem Lager (darin ist der Bonus schon vorhanden)
	enemyInnerSec += pIntel->GetInnerSecurityStorage();

	return enemyInnerSec;
}