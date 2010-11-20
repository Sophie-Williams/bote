#include "stdafx.h"
#include "ShipAI.h"
#include "Botf2Doc.h"
#include "SectorAI.h"
#include "Races\RaceController.h"
#include "Ships\Fleet.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
CShipAI::CShipAI(CBotf2Doc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;

	map<CString, CRace*>* mRaces = m_pDoc->GetRaceCtrl()->GetRaces();
	ASSERT(mRaces);
	
	// Durchschnittsmoral berechnen
	map<CString, int> moralAll;
	map<CString, int> systems;
	for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			if (m_pDoc->m_System[x][y].GetOwnerOfSystem() != "")
			{
				moralAll[m_pDoc->m_System[x][y].GetOwnerOfSystem()] += m_pDoc->m_System[x][y].GetMoral();
				systems[m_pDoc->m_System[x][y].GetOwnerOfSystem()] += 1;
			}
	
	// alles initial initialisieren
	for (map<CString, CRace*>::const_iterator it = mRaces->begin(); it != mRaces->end(); ++it)
	{
		m_AttackSector[it->first] = CPoint(-1,-1);
		m_BombardSector[it->first] = CPoint(-1,-1);

		if (systems[it->first] > NULL)
			m_iAverageMoral[it->first] = moralAll[it->first] / systems[it->first];
		else
			m_iAverageMoral[it->first] = 0;
	}
}

CShipAI::~CShipAI(void)
{
}

//////////////////////////////////////////////////////////////////////
// sonstige Funktionen
//////////////////////////////////////////////////////////////////////
/// Diese Funktion erteilt allen Schiffen aller computergesteuerten Rassen Befehle.
void CShipAI::CalculateShipOrders(CSectorAI* SectorAI)
{
	ASSERT(SectorAI);
	m_pSectorAI = SectorAI;

	// einen m�glichen Angriffssektor berechnen lassen
	CalcAttackSector();
	// danach einen m�glichen Bombardierungssektor finden
	CalcBombardSector();

	for (int i = 0; i < m_pDoc->m_ShipArray.GetSize(); i++)
	{
		CString sOwner	= m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip();
		CRace* pOwner	= m_pDoc->GetRaceCtrl()->GetRace(sOwner);
		if (!pOwner)
			continue;
		// gilt erstmal nur f�r Majors
		if (pOwner->GetType() == MAJOR)
		{
			// gilt nicht f�r menschliche Spieler
			if (((CMajor*)pOwner)->IsHumanPlayer())
				continue;
			// Flotte versuchen zu erstellen
			DoMakeFleet(i);

			// Vielleicht haben unsere Schiffe ein Ziel, welches sie angreifen m�ssen/k�nnen			
			if (DoAttackMove(i))
			{				
				DoCamouflage(i);
				continue;
			}
			if (m_pDoc->m_ShipArray.GetAt(i).GetPath()->GetSize() == NULL)
			{
				// Scouts und Kriegsschiffe fliegen zuerst einmal zu den Minorracesystemen
				if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() > COLONYSHIP)
				{
					// Zeiger auf Vektor mit Minorracessektoren holen
					vector<CPoint>* vMinorraceSectors = m_pSectorAI->GetMinorraceSectors(sOwner);

					BOOLEAN set = FALSE;
					int count = vMinorraceSectors->size() * 2;
					while (vMinorraceSectors->size() && count--)
					{
						int j = rand()%vMinorraceSectors->size();
						CPoint ko = vMinorraceSectors->at(j);
						// Wenn Gefahr der anderen Rassen kleiner als die der meinen ist
						if (m_pSectorAI->GetCompleteDanger(sOwner, ko) == NULL || 
							(m_pSectorAI->GetCompleteDanger(sOwner, ko) <= m_pSectorAI->GetDangerOnlyFromCombatShips(sOwner, m_pDoc->m_ShipArray.GetAt(i).GetKO())))
							if (((CMajor*)pOwner)->GetStarmap()->GetRange(ko) <= m_pDoc->m_ShipArray.GetAt(i).GetRange())
							{
								// Zielkoordinate f�r das Schiff setzen
								m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(ko,0);
#ifdef TRACE_SHIPAI
								MYTRACE(MT::LEVEL_INFO, "Race %s: Ship to Minor: %s (%s) - Target: %d,%d\n",sOwner, m_pDoc->m_ShipArray.GetAt(i).GetShipName(), m_pDoc->m_ShipArray.GetAt(i).GetShipTypeAsString(), ko.x,ko.y);
#endif								
								vMinorraceSectors->erase(vMinorraceSectors->begin() + j--);	
								set = TRUE;
								break;
							}
					}
					if (set)
					{
						DoCamouflage(i);
						continue;
					}
				}
				// Kolonieschiffe zum Terraformen schicken. Andere Schiffe fliegen manchmal auch dort hin, wenn
				// sie gerade keinen anderen Flugauftrag haben.
				if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() >= COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() != TERRAFORM)
				{
					// Zeiger auf Vektor mit Terraformsektoren holen
					vector<CSectorAI::SectorToTerraform>* vSectorsToTerrform = m_pSectorAI->GetSectorsToTerraform(sOwner);

					for (UINT j = 0; j < vSectorsToTerrform->size(); j++)
					{
						CPoint ko = vSectorsToTerrform->at(j).p;
						// Wenn das Kolonieschiff schon auf einem Sektor f�r unser Terraforming steht, so fliegt es nicht
						// weiter
						if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() == COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetKO() == ko)
							break;
						// Wenn Gefahr der anderen Rassen kleiner als die der meinen ist
						if (m_pSectorAI->GetCompleteDanger(sOwner, ko) == NULL || (m_pSectorAI->GetCompleteDanger(sOwner, ko) < m_pSectorAI->GetDanger(sOwner, m_pDoc->m_ShipArray.GetAt(i).GetKO())))
							if (((CMajor*)pOwner)->GetStarmap()->GetRange(ko) <= m_pDoc->m_ShipArray.GetAt(i).GetRange())
							{
								// Zielkoordinate f�r das Schiff setzen
								m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(ko,0);
#ifdef TRACE_SHIPAI
								MYTRACE(MT::LEVEL_INFO, "Race %s: Ship %s (%s) has terraforming target: %d,%d\n",sOwner, m_pDoc->m_ShipArray.GetAt(i).GetShipName(), m_pDoc->m_ShipArray.GetAt(i).GetShipTypeAsString(), ko.x,ko.y);
#endif
								break;
							}
					}
				}
				// Truppentransporter zu einem m�glichen Sektor fliegen lassen um dort einen Au�enposten bauen zu k�nnen
				if (m_pSectorAI->GetStationBuildSector(sOwner).points > MINBASEPOINTS && m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() != BUILD_OUTPOST)
				{
					// nur Truppentransporter oder andere Schiffe ohne Ziel fliegen zu diesem Punkt, niemals aber
					// Kolonieschiffe
					if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() == TRANSPORTER
						|| (m_pDoc->m_ShipArray.GetAt(i).GetShipType() != COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetTargetKO() == m_pDoc->m_ShipArray.GetAt(i).GetKO()))
					{
						CPoint ko(m_pSectorAI->GetStationBuildSector(sOwner).position.x, m_pSectorAI->GetStationBuildSector(sOwner).position.y);
						// Wenn Gefahr der anderen Rassen kleiner als die der meinen ist
						if (m_pSectorAI->GetCompleteDanger(sOwner, ko) == NULL || (m_pSectorAI->GetCompleteDanger(sOwner, ko) < m_pSectorAI->GetDanger(sOwner, m_pDoc->m_ShipArray.GetAt(i).GetKO())))
							if (((CMajor*)pOwner)->GetStarmap()->GetRange(ko) <= m_pDoc->m_ShipArray.GetAt(i).GetRange())
							{
								// Zielkoordinate f�r das Schiff setzen
								m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(ko,0);
#ifdef TRACE_SHIPAI
								MYTRACE(MT::LEVEL_INFO, "Race %s: Ship %s (%s) has stationbuild target: %d,%d\n",sOwner, m_pDoc->m_ShipArray.GetAt(i).GetShipName(), m_pDoc->m_ShipArray.GetAt(i).GetShipTypeAsString(), ko.x,ko.y);
#endif
							}
					}
				}				

				DoCamouflage(i);
				CPoint shipKO = m_pDoc->m_ShipArray.GetAt(i).GetKO();
				if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetSunSystem())
					if (!DoTerraform(i))
						DoColonize(i);
				DoStationBuild(i);
			}
			else
				DoCamouflage(i);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// private Funktionen
//////////////////////////////////////////////////////////////////////
/// Funktion erteilt einen Terraformbefehl, sofern dies auch m�glich ist. �bergebn wird daf�r der index
/// <code>index</code> des Schiffes aus dem Schiffsarray des Documents. Der R�ckgabewert der Funktion ist
/// <code>TRUE</code>, wenn ein Terraformbefehl gegeben werden k�nnte.
BOOLEAN CShipAI::DoTerraform(int index)
{
	/*
	   Es wird jeder Planet sofort genommen, welcher weniger als 7 Terraformpunkte
	   ben�tigt. Planeten welche mehr als 6 Terraformpunkte ben�tigen werden nicht
	   unbedingt sofort gew�hlt. Dort m�ssen wir die doppelte Anzahl an Runden erreicht
	   haben. Au�er es gibt keinen Planeten, welcher in diesem Moment kolonisiert werden
	   k�nnte.
    */
	int i = index;
	if (m_pDoc->m_ShipArray.GetAt(i).GetColonizePoints() == NULL)
		return FALSE;
	CPoint shipKO = m_pDoc->m_ShipArray.GetAt(i).GetKO();
	BYTE minTerraPoints = MAXBYTE;
	short planet = -1;
	BYTE terraPoints = m_pDoc->m_ShipArray.GetAt(i).GetColonizePoints();
	for (int j = 0; j < m_pDoc->m_Sector[shipKO.x][shipKO.y].GetNumberOfPlanets(); j++)
		if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetTerraformed() == FALSE
			&& m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetHabitable() == TRUE)
		{
			if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints() < minTerraPoints)
				minTerraPoints = m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints();
			// minTerraPoints - TerraformingPoints des Schiffes > 6 + Planetentyp
			if ((minTerraPoints - terraPoints) > (4 + m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetType()) && 2*minTerraPoints > m_pDoc->m_iRound)
			{
				minTerraPoints = MAXBYTE;
				planet = -1;
			}
			if (minTerraPoints == MAXBYTE && (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints() - terraPoints) <= (4 + m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetType()))
				minTerraPoints = m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints();
			if (minTerraPoints == m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints())
				planet = j;
		}
	if (planet != -1)
	{
		// Hier muss als erstes ein m�glicher neuer Kurs gel�scht werden
		m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(shipKO,0);
		m_pDoc->m_ShipArray.GetAt(i).SetTerraformingPlanet(planet);
		m_pDoc->m_ShipArray.GetAt(i).SetCurrentOrder(TERRAFORM);
		return TRUE;
	}
	// Wenn kein Planet zum Terraformen gefunden wurde, wir aber auch keinen Planeten kolonisieren
	// k�nnen, dann w�hle den mit der k�rzesten Terraformdauer.
	else
	{
		minTerraPoints = MAXBYTE;
		planet = -1;
		for (int j = 0; j < m_pDoc->m_Sector[shipKO.x][shipKO.y].GetNumberOfPlanets(); j++)
		{
			if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetTerraformed() == TRUE
				&& m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetColonized() == FALSE)
			{
				planet = -1;
				break;
			}
			else if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetTerraformed() == FALSE
				&& m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetHabitable() == TRUE)
			{
				if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints() < minTerraPoints)
				{
					minTerraPoints = m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints();
					planet = j;
				}
			}
		}
		if (planet != -1)
		{
			// Hier muss als erstes ein m�glicher neuer Kurs gel�scht werden
			m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(shipKO,0);
			m_pDoc->m_ShipArray.GetAt(i).SetTerraformingPlanet(planet);
			m_pDoc->m_ShipArray.GetAt(i).SetCurrentOrder(TERRAFORM);
			return TRUE;
		}
	}
	return FALSE;
}

/// Funktion erteilt einen Kolonisierungsbefehl, sofern dies auch m�glich ist. �bergebn wird daf�r der index
/// <code>index</code> des Schiffes aus dem Schiffsarray des Documents. Der R�ckgabewert der Funktion ist
/// <code>TRUE</code>, wenn ein Kolonisierungsbefehl gegeben werden k�nnte.
BOOLEAN CShipAI::DoColonize(int index)
{
	int i = index;
	// Kolonieschiffe eine Kolonisierung vorschlagen
	if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() == COLONYSHIP)
	{
		CPoint shipKO = m_pDoc->m_ShipArray.GetAt(i).GetKO();
		// Geh�rt der Sektor aktuell auch keiner Minorrace (also niemanden oder uns selbst)
		if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetOwnerOfSector() == "" || m_pDoc->m_Sector[shipKO.x][shipKO.y].GetOwnerOfSector() == m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip())
			// Kolonisierungsbefehl geben
			for (int j = 0; j < m_pDoc->m_Sector[shipKO.x][shipKO.y].GetNumberOfPlanets(); j++)
				if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetTerraformed() == TRUE
					&& m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetColonized() == FALSE)
				{
					// Hier muss als erstes ein m�glicher neuer Kurs gel�scht werden
					m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(shipKO,0);
					m_pDoc->m_ShipArray.GetAt(i).SetCurrentOrder(COLONIZE);
					return TRUE;
				}
	}
	return FALSE;
}

/// Funktion schickt Kriegsschiffe zu einem m�glichen Offensivziel. Unter Umst�nden wird auch der Befehl zum
/// Angriff automatisch erteilt. Auch kann es passieren, das diese Funktion die Kriegsschiffe zu einem
/// gemeinsamen Sammelpunkt schickt. �bergebn wird daf�r der index <code>index</code> des Schiffes aus dem
/// Schiffsarray des Documents. Der R�ckgabewert der Funktion ist <code>TRUE</code>, wenn ein Bewegungsbefehl
/// gegeben werden k�nnte.
BOOLEAN CShipAI::DoAttackMove(int index)
{
	if (m_pDoc->m_ShipArray.GetAt(index).GetShipType() <= COLONYSHIP
		|| m_pDoc->m_ShipArray.GetAt(index).GetShipType() >= OUTPOST)
		return FALSE;

	if (DoBombardSystem(index))
		return TRUE;
		
	CString sRace = m_pDoc->m_ShipArray.GetAt(index).GetOwnerOfShip();
	BYTE min  = MAXBYTE;
	CPoint targetKO(-1,-1);
	int z = 0;
	// �berpr�ft wird zuerst, ob hier ein m�gliches Offensivziel in der N�he des Schiffes ist. Wenn ja, dann
	// wird �berpr�ft, ob unsere Schiffsst�rke gr��er als die beim Ziel ist oder wir 75% bis 90% unserer Flotte
	// auf unserem Sektorfeld schon versammelt haben. Wenn ja wird angegriffen.
	for (UINT i = 0; i < m_pSectorAI->GetOffensiveTargets(sRace)->size(); i++)
	{
		CPoint p = m_pSectorAI->GetOffensiveTargets(sRace)->at(i);
		// In der N�he ist das Ziel mit der geringsten Entfernung zu unserem Sektor.
		if (max(p.x,p.y) < min)
		{
			min = (BYTE)max(p.x,p.y);
			targetKO = p;
			z = i;
		}
	}
	// Es gibt ein Ziel in der N�he unseres Schiffes
	if (targetKO != CPoint(-1,-1))
	{
#ifdef TRACE_SHIPAI
		MYTRACE(MT::LEVEL_INFO, "Race %s: ship '%s' maybe has near offensive target %d/%d\n",sRace, m_pDoc->m_ShipArray.GetAt(index).GetShipName(), targetKO.x, targetKO.y);
#endif
		// pr�fen ob wir st�rker sind oder ob wie 75 bis 90% unserer Flotte hier stationiert haben
		if (m_pSectorAI->GetDangerOnlyFromCombatShips(sRace, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > m_pSectorAI->GetCompleteDanger(sRace, targetKO) * 1.25
			|| m_pSectorAI->GetDangerOnlyFromCombatShips(sRace, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > (m_pSectorAI->GetCompleteDanger(sRace) * (rand()%16+75) / 100))
		{
			// !!!
			// (besser w�re noch, wir bilden hier vorher eine Flotte, so l��t sich koordinierter Angreifen!!!)
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pDoc->GetRaceCtrl()->GetRace(sRace));

			// Wenn dies zutrifft werden alle Schiffe aus diesem Sektor zum Zielsektor geschickt.
			for (int j = 0; j < m_pDoc->m_ShipArray.GetSize(); j++)
				if (m_pDoc->m_ShipArray.GetAt(j).GetOwnerOfShip() == sRace)
					if (m_pDoc->m_ShipArray.GetAt(j).GetKO() == m_pDoc->m_ShipArray.GetAt(index).GetKO())
						if (m_pDoc->m_ShipArray.GetAt(j).GetShipType() <= COLONYSHIP
							|| m_pDoc->m_ShipArray.GetAt(j).GetShipType() >= OUTPOST)
							if (pMajor != NULL && pMajor->GetStarmap()->GetRange(targetKO) <= m_pDoc->m_ShipArray.GetAt(j).GetRange())
							{
								m_pDoc->m_ShipArray.GetAt(j).SetTargetKO(targetKO, 0);
								#ifdef TRACE_SHIPAI
								MYTRACE(MT::LEVEL_INFO, "Race %d: ATTACK at near target: %d/%d\n",targetKO.x,targetKO.y);
								#endif
								m_pSectorAI->GetOffensiveTargets(sRace)->erase(m_pSectorAI->GetOffensiveTargets(sRace)->begin() + z);								
								return TRUE;
							}
		}
	}
	// wenn es kein Ziel in der N�he gab, daf�r aber ein Bombardierungsziel ausgegeben wurde, so wird dieses mit
	// der h�chsten Priorit�t angeflogen
	if (m_BombardSector[sRace] != CPoint(-1,-1))
	{
		// pr�fen ob wir im Bombardierungssektor st�rker w�ren
		if (m_pSectorAI->GetDangerOnlyFromCombatShips(sRace, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > m_pSectorAI->GetCompleteDanger(sRace, m_BombardSector[sRace]))
		{
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pDoc->GetRaceCtrl()->GetRace(sRace));
			// hier �berpr�fen, ob der Sektor erreicht werden kann
			if (pMajor != NULL && pMajor->GetStarmap()->GetRange(m_BombardSector[sRace]) <= m_pDoc->m_ShipArray.GetAt(index).GetRange())
			{
				m_pDoc->m_ShipArray.GetAt(index).SetTargetKO(m_BombardSector[sRace], 0);
				#ifdef TRACE_SHIPAI
				MYTRACE(MT::LEVEL_INFO, "Race %s: BOMBARDTARGET in sector: %d/%d\n",sRace,m_BombardSector[sRace].x,m_BombardSector[sRace].y);
				#endif
				return TRUE;
			}
		}
	}

	// wenn es kein Ziel in der N�he gibt, oder wenn das Ziel in der N�he nicht f�r den Angriff geeignet war, dann
	// wenn m�glich die Schiffe zu dem Punkt schicken, welcher als generelles Angriffsziel auserkoren wurde
	if (m_AttackSector[sRace] != CPoint(-1,-1))
	{
		CMajor* pMajor = dynamic_cast<CMajor*>(m_pDoc->GetRaceCtrl()->GetRace(sRace));

		// pr�fen ob wir st�rker sind oder ob wie 75 bis 90% unserer Flotte hier stationiert haben
		if (m_pSectorAI->GetDangerOnlyFromCombatShips(sRace, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > m_pSectorAI->GetCompleteDanger(sRace, m_AttackSector[sRace])
			|| m_pSectorAI->GetDangerOnlyFromCombatShips(sRace, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > (m_pSectorAI->GetCompleteDanger(sRace) * (rand()%16+75) / 100))
		{
			// hier noch �berpr�fen, ob der Sektor erreicht werden kann
			if (pMajor != NULL && pMajor->GetStarmap()->GetRange(m_AttackSector[sRace]) <= m_pDoc->m_ShipArray.GetAt(index).GetRange())
			{
				m_pDoc->m_ShipArray.GetAt(index).SetTargetKO(m_AttackSector[sRace], 0);
#ifdef TRACE_SHIPAI
				MYTRACE(MT::LEVEL_INFO, "Race %s: GLOBAL ATTACK in sector: %d/%d\n",sRace,m_AttackSector[sRace-1].x,m_AttackSector[sRace-1].y);
				MYTRACE(MT::LEVEL_INFO, "Ship: %s\n",m_pDoc->m_ShipArray.GetAt(index).GetShipName());
				MYTRACE(MT::LEVEL_INFO, "OnlyDangerFromShips: %d in Sector: %d/%d\n",m_pSectorAI->GetDangerOnlyFromCombatShips(sRace, m_pDoc->m_ShipArray.GetAt(index).GetKO()),m_pDoc->m_ShipArray.GetAt(index).GetKO().x,m_pDoc->m_ShipArray.GetAt(index).GetKO().y);
				MYTRACE(MT::LEVEL_INFO, "CompleteDangerInTargetSector: %d\n",m_pSectorAI->GetCompleteDanger(sRace, m_AttackSector[sRace]));
				MYTRACE(MT::LEVEL_INFO, "Our Complete Danger overall: %d\n",(m_pSectorAI->GetCompleteDanger(sRace) * 100 / (rand()%16+75)));
#endif
				return TRUE;
			}
		}
		// falls wir nicht st�rker sind, so fliegen wir zu dem Feld, wo unserer st�rkste Schiffsansammlung ist
		else if (m_pSectorAI->GetHighestShipDanger(sRace) != CPoint(-1,-1))
		{
			CPoint p = m_pSectorAI->GetHighestShipDanger(sRace);
			// hier noch �berpr�fen, ob der Sektor erreicht werden kann
			if (pMajor != NULL && pMajor->GetStarmap()->GetRange(p) <= m_pDoc->m_ShipArray.GetAt(index).GetRange())
			{
				m_pDoc->m_ShipArray.GetAt(index).SetTargetKO(p, 0);
#ifdef TRACE_SHIPAI
				MYTRACE(MT::LEVEL_INFO, "Race %s: COLLECT ships in sector: %d/%d\n", sRace,p.x,p.y);
#endif
				return TRUE;
			}
		}
	}
	return FALSE;
}

/// Funktion schickt Kriegsschiffe zu einem m�glichen System, welches Bombardiert werden k�nnte. �bergeben wird
/// daf�r der index <code>index</code> des Schiffes aus dem Schiffsarray des Documents. Der R�ckgabewert der
/// Funktion ist <code>TRUE</code>, wenn ein Bewegungsbefehl gegeben werden konnte oder eine Bombardierung
/// befohlen wurde oder das Schiff auf Verst�rkung zur Bombardierung im System wartet.
BOOLEAN CShipAI::DoBombardSystem(int index)
{
	if (m_pDoc->m_ShipArray.GetAt(index).GetKO() == m_BombardSector[m_pDoc->m_ShipArray.GetAt(index).GetOwnerOfShip()])
	{
		// Hier muss als erstes ein m�glicher neuer Kurs gel�scht werden und ein alter Systemangriffsbefehl aufgehoben werden
		m_pDoc->m_ShipArray.GetAt(index).SetTargetKO(m_pDoc->m_ShipArray.GetAt(index).GetKO(), 0);
		// Wenn die defensive Schiffsst�rke im System des Angreifers ungef�hr doppelt so gro� als der Systemverteidigungswert
		// des Verteidigers ist, wird angegriffen.
		int shipValue = 0;
		for (int i = 0; i < m_pDoc->m_ShipArray.GetSize(); i++)
			if (m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip() == m_pDoc->m_ShipArray.GetAt(index).GetOwnerOfShip())
				if (m_pDoc->m_ShipArray.GetAt(i).GetKO() == m_pDoc->m_ShipArray.GetAt(index).GetKO())
				{
					if (m_pDoc->m_ShipArray.GetAt(i).GetCloak() == FALSE)
						shipValue += (int)m_pDoc->m_ShipArray.GetAt(i).GetHull()->GetCurrentHull();
					if (m_pDoc->m_ShipArray.GetAt(i).GetFleet() != NULL)
						for (int j = 0; j < m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetFleetSize(); j++)
							if (m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetShipFromFleet(j)->GetCloak() == FALSE)
								shipValue += (int)m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetShipFromFleet(j)->GetHull()->GetCurrentHull();
					
				}
		int shipDefend = m_pDoc->GetSystem(m_pDoc->m_ShipArray.GetAt(index).GetKO().x, m_pDoc->m_ShipArray.GetAt(index).GetKO().y).GetProduction()->GetShipDefend();
		shipDefend *= 1.25;
		//CString s;
		//s.Format("shipValue = %d\nshipDefend = %d\nSektor = %s", shipValue, shipDefend, m_pDoc->GetSector(m_pDoc->m_ShipArray.GetAt(index).GetKO().x, m_pDoc->m_ShipArray.GetAt(index).GetKO().y).GetName(true));
		//AfxMessageBox(s);
		// Wenn das Schiff bzw Schiffe aus der Flotte getarnt sind, dann m�ssen diese erst enttarnt werden
		if (m_pDoc->m_ShipArray.GetAt(index).GetCloak() || (m_pDoc->m_ShipArray.GetAt(index).GetFleet() != 0 && m_pDoc->m_ShipArray.GetAt(index).GetFleet()->CheckOrder(&m_pDoc->m_ShipArray.GetAt(index), ATTACK_SYSTEM) == FALSE))
		{
			// AfxMessageBox(m_pDoc->GetSector(m_pDoc->m_ShipArray.GetAt(index).GetKO()).GetName());
			// Schiff enttarnen
			m_pDoc->m_ShipArray.GetAt(index).SetCurrentOrder(CLOAK);
			return TRUE;
		}		
		if (shipValue > shipDefend)
		{		
			#ifdef TRACE_SHIPAI
			MYTRACE(MT::LEVEL_INFO, "Race %s: Ship %s (%s) is bombarding system: %d,%d\n",m_pDoc->m_ShipArray.GetAt(index).GetOwnerOfShip(), m_pDoc->m_ShipArray.GetAt(index).GetShipName(), m_pDoc->m_ShipArray.GetAt(index).GetShipTypeAsString(), m_pDoc->m_ShipArray.GetAt(index).GetKO().x,m_pDoc->m_ShipArray.GetAt(index).GetKO().y);
			#endif
			m_pDoc->m_ShipArray.GetAt(index).SetCurrentOrder(ATTACK_SYSTEM);
			return TRUE;
		}		
	}
	return FALSE;
}

/// Funktion erteilt einen Tarnbefehl, wenn <code>camouflage</code> wahr ist. Ist <code>camouflage</code> falsch,
/// wird ein Enttarnbefehl gegeben. �bergebn wird daf�r der index <code>index</code> des Schiffes aus dem
/// Schiffsarray des Documents. Der R�ckgabewert der Funktion ist <code>TRUE</code>, wenn der Befehl gegeben wurde.
BOOLEAN CShipAI::DoCamouflage(int index, BOOLEAN camouflage)
{
	// Nur wenn das Schiff sich tarnen kann und nicht gerade dabei ist ein System zu bombardieren soll es sich tarnen
	if (m_pDoc->m_ShipArray.GetAt(index).GetStealthPower() > 3 && m_pDoc->m_ShipArray.GetAt(index).GetCurrentOrder() != ATTACK_SYSTEM)
	{
		// wollen tarnen
		if (camouflage)
		{
			if (m_pDoc->m_ShipArray.GetAt(index).GetStealthPower() > 3)
				if (m_pDoc->m_ShipArray.GetAt(index).GetCloak() == FALSE)
				{
					m_pDoc->m_ShipArray.GetAt(index).SetCurrentOrder(CLOAK);
					//m_pDoc->m_ShipArray.GetAt(index).SetCloak();
					return TRUE;
				}
		}
		// wollen enttarnen
		else
			if (m_pDoc->m_ShipArray.GetAt(index).GetCloak() == TRUE)
			{
				m_pDoc->m_ShipArray.GetAt(index).SetCurrentOrder(CLOAK);
				//m_pDoc->m_ShipArray.GetAt(index).SetCloak();
				return TRUE;
			}
	}
	return FALSE;
}

/// Funktion erstellt eine Flotte. Schiffe werden der Flotte nur hinzugef�gt, wenn diese bestimmte Voraussetzungen erf�llen.
/// So muss der ungef�hre Schiffstyp �bereinstimmen (Combat <-> NonCombat) sowie die Reichweite des Schiffes.
void CShipAI::DoMakeFleet(int index)
{
	CShip* ship = &m_pDoc->m_ShipArray.GetAt(index);
	if (ship->GetShipType() == OUTPOST || ship->GetShipType() == STARBASE)
		return;

	for (int i = index + 1; i < m_pDoc->m_ShipArray.GetSize(); i++)
		// Schiffe m�ssen von der selben Rasse sein
		if (ship->GetOwnerOfShip() == m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip())
		{
			// Schiffe m�ssen sich im selben Sektor befinden
			if (ship->GetKO() == m_pDoc->m_ShipArray.GetAt(i).GetKO())
			{
				// beide Schiffe m�ssen die selbe Reichweite haben
				if (ship->GetRange() == m_pDoc->m_ShipArray.GetAt(i).GetRange())
				{
					// das hinzuzuf�gende Schiff darf kein Au�enposten oder Sternbasis sein
					if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() != OUTPOST && m_pDoc->m_ShipArray.GetAt(i).GetShipType() != STARBASE)
					{
						// das hinzuzuf�gende Schiff darf kein individuelles Ziel haben
						if (m_pDoc->m_ShipArray.GetAt(i).GetPath()->GetSize() == NULL)
						{
							// der Tarnstatus muss gleich sein (also keine getarnten und ungetarnte Schiffe in eine Flotte)
							if (ship->GetCloak() == m_pDoc->m_ShipArray.GetAt(i).GetCloak())
							{
								// wenn sich das F�hrungsschiff tarnen kann, dann muss das hinzuzuf�gende Schiff sich auch tarnen k�nnen
								if ((ship->GetStealthPower() > 3 && m_pDoc->m_ShipArray.GetAt(i).GetStealthPower() > 3) || (ship->GetStealthPower() <= 3 && m_pDoc->m_ShipArray.GetAt(i).GetStealthPower() <= 3))
								{
									// es muss sich bei beiden Schiffen um Kriegsschiffe handeln oder bei beiden Schiffen um Transporter oder bei beiden
									// Schiffen um Kolonieschiffe
									if ((!ship->IsNonCombat() && !m_pDoc->m_ShipArray.GetAt(i).IsNonCombat())
										||(ship->GetShipType() == TRANSPORTER && m_pDoc->m_ShipArray.GetAt(i).GetShipType() == TRANSPORTER && m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() < BUILD_OUTPOST)
										||(ship->GetShipType() == COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetShipType() == COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() < COLONIZE))
									{
										ship->CreateFleet();
										// hat das hinzuzuf�gende Schiff eine eigene Flotte
										if (m_pDoc->m_ShipArray.GetAt(i).GetFleet() != NULL)
										{
											// Schiffe aus der Flotte der neuen Flotte hinzuf�gen
											for (int n = 0; n < m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetFleetSize(); n++)
												ship->GetFleet()->AddShipToFleet(m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetShipFromFleet(n));
											m_pDoc->m_ShipArray.GetAt(i).DeleteFleet();
										}
										ship->GetFleet()->AddShipToFleet(&m_pDoc->m_ShipArray.GetAt(i));
										m_pDoc->m_ShipArray.RemoveAt(i--);							
									}
								}
							}
						}
					}
				}
			}
		}
}

/// Funktion erteilt einen Au�enpostenbaubefehl, sofern dies auch m�glich ist. �bergeben wird daf�r der index
/// <code>index</code> des Schiffes aus dem Schiffsarray des Dokuments. Der R�ckgabewert der Funktion ist
/// <code>TRUE</code>, wenn ein Au�enpostenbaubefehl gegeben werden k�nnte.
BOOLEAN CShipAI::DoStationBuild(int index)
{
	int i = index;
	if (m_pDoc->m_ShipArray.GetAt(i).GetStationBuildPoints() == NULL || m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() == BUILD_OUTPOST)
		return FALSE;

	CString sRace = m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip();
	if (m_pSectorAI->GetStationBuildSector(sRace).points <= MINBASEPOINTS)
		return FALSE;

	CPoint ko(m_pSectorAI->GetStationBuildSector(sRace).position.x, m_pSectorAI->GetStationBuildSector(sRace).position.y);
	if (m_pDoc->m_ShipArray.GetAt(i).GetKO() == ko)
	{
		// Nur wenn der Sektor uns bzw. niemanden geh�rt
		if (m_pDoc->m_Sector[ko.x][ko.y].GetOwnerOfSector().IsEmpty() || m_pDoc->m_Sector[ko.x][ko.y].GetOwnerOfSector() == sRace)
		{
			m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(ko, 0);
			m_pDoc->m_ShipArray.GetAt(i).SetCurrentOrder(BUILD_OUTPOST);
			return TRUE;
		}
	}
	return FALSE;
}

/// Funkion berechnet einen m�glichen Angriffssektor, welcher sp�ter gesammelt angegriffen werden kann.
void CShipAI::CalcAttackSector(void)
{
	// f�r alle Majors den Angriffssektor berechnen
	map<CString, CMajor*>* pmMajors = m_pDoc->GetRaceCtrl()->GetMajors();
	
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		int nearestSector = MAXSHORT;
		// beinhaltet die Sektorkoordinate mit unserem Angriffsziel
		CPoint sector = CPoint(-1,-1);
		for (UINT j = 0; j < m_pSectorAI->GetOffensiveTargets(it->first)->size(); j++)
		{
			int space = max(abs(m_pSectorAI->GetHighestShipDanger(it->first).x - m_pSectorAI->GetOffensiveTargets(it->first)->at(j).x),
				abs(m_pSectorAI->GetHighestShipDanger(it->first).y - m_pSectorAI->GetOffensiveTargets(it->first)->at(j).y));
			if (space < nearestSector)
			{
				// Wenn dieses Ziel eine kleinere Gefahr darstellt als unsere gr��te Flottenansammlung oder wir
				// in schon mindst 50% unserer Flotte hier versammelt haben, so wird es als Ziel aufgenommen
				if ((m_pSectorAI->GetDangerOnlyFromCombatShips(it->first, m_pSectorAI->GetHighestShipDanger(it->first)) > m_pSectorAI->GetCompleteDanger(it->first, m_pSectorAI->GetOffensiveTargets(it->first)->at(j)))
					|| (m_pSectorAI->GetDangerOnlyFromCombatShips(it->first, m_pSectorAI->GetHighestShipDanger(it->first)) > (m_pSectorAI->GetCompleteDanger(it->first) * 50 / 100)))
				{
					nearestSector = space;
					sector = m_pSectorAI->GetOffensiveTargets(it->first)->at(j);
				}
			}
		}
		m_AttackSector[it->first] = sector;
#ifdef TRACE_SHIPAI
		MYTRACE(MT::LEVEL_INFO, "Race %s: global attack sector is %d/%d\n",it->first, m_AttackSector[it->first].x, m_AttackSector[it->first].y);
#endif
	}
}

/// Funktion berechnet einen m�glich besten Sektor f�r eine Bombardierung. Wurde solch ein Sektor ermittelt hat dieser
/// die allerh�chste Priorit�t.
void CShipAI::CalcBombardSector(void)
{
	// f�r alle Majors den Angriffssektor berechnen
	map<CString, CMajor*>* pmMajors = m_pDoc->GetRaceCtrl()->GetMajors();
	
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		// die Hauptrassen bombardieren nicht alle gleichoft und gleich lang. Die F�deration z.B. kann wegen den
		// Moralabzu�gen nicht so lang bombardieren.
		BOOLEAN bombard = TRUE;

		// Moralwert f�r Bombardierungen holen
		short nMoralValue = CMoralObserver::GetMoralValue(it->second->GetRaceMoralNumber(), 19);
		if (nMoralValue < 0)
		{
			nMoralValue = abs(nMoralValue);
			if (m_iAverageMoral[it->first] < 80 + nMoralValue * 5)
				bombard = FALSE;
		}

		// pazifistische Rassen bombardieren nie
		if (it->second->IsRaceProperty(PACIFIST))
			bombard = FALSE;
		
		if (!bombard)
			continue;
		// ist der globale Angriffssektor auch bei den Bombardierungzielen vorhanden, so wird dieser benutzt und kein
		// am n�chsten befindlicher Bombardierungssektor
		for (UINT j = 0; j < m_pSectorAI->GetBombardTargets(it->first)->size(); j++)
			if (m_pSectorAI->GetBombardTargets(it->first)->at(j) == m_AttackSector[it->first])
			{
				m_BombardSector[it->first] = m_pSectorAI->GetBombardTargets(it->first)->at(j);
				break;				
			}
		// Wurde kein Bombardierungziel mit dem gleichen globalen Attackziel gefunden, so wird versucht ein n�chstes
		// zu unserer gr��ten Flottenansammlung zu finden
		if (m_BombardSector[it->first] == CPoint(-1,-1))
		{
			int nearestSector = MAXSHORT;
			// beinhaltet die Sektorkoordinate mit unserem Bombardierungsziel
			CPoint sector = CPoint(-1,-1);
			for (UINT j = 0; j < m_pSectorAI->GetBombardTargets(it->first)->size(); j++)
			{
				// Es wird das Ziel ausgew�hlt, welches am n�chsten von unserer gr��ten Flottensammlung entfernt ist
				int space = max(abs(m_pSectorAI->GetHighestShipDanger(it->first).x - m_pSectorAI->GetBombardTargets(it->first)->at(j).x),
					abs(m_pSectorAI->GetHighestShipDanger(it->first).y - m_pSectorAI->GetBombardTargets(it->first)->at(j).y));
				if (space < nearestSector)
				{
					// Wenn dieses Ziel eine kleinere Gefahr darstellt als unsere gr��te Flottenansammlung, so wird
					// es als Ziel aufgenommen
					if (m_pSectorAI->GetDangerOnlyFromCombatShips(it->first, m_pSectorAI->GetHighestShipDanger(it->first)) > m_pSectorAI->GetCompleteDanger(it->first, m_pSectorAI->GetBombardTargets(it->first)->at(j)))
					{
						nearestSector = space;
						sector = m_pSectorAI->GetBombardTargets(it->first)->at(j);
					}
				}
			}
			m_BombardSector[it->first] = sector;
		}
#ifdef TRACE_SHIPAI
		MYTRACE(MT::LEVEL_INFO, "Race %s: global bombard sector is %d/%d\n",it->first, m_BombardSector[it->first].x, m_BombardSector[it->first].y);
#endif
	}
}
