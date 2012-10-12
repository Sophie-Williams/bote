#pragma once

#include "Botf2Doc.h"

class CNewRoundDataCalculator
{
private:
	CNewRoundDataCalculator(void);
public:
	~CNewRoundDataCalculator(void);

	CNewRoundDataCalculator(CBotf2Doc* pDoc);

	static void GetIntelligenceBoniFromSpecialTechsAndSetThem(const std::map<CString, CMajor*>* pmMajors);
	void CheckRoutes(const CSector& sector, CSystem& system, CMajor* pMajor);
	static void CalcIntelligenceBoni(const CSystemProd* production, CIntelligence* intelligence);
	static void CalcMoral(const CSector& sector, CSystem& system, CArray<CTroopInfo>& TroopInfo);
	static void CNewRoundDataCalculator::CalcExtraVisibilityAndRangeDueToDiplomacy(CSector& sector, const std::map<CString, CMajor*>* pmMajors);
	void AddShipPortsFromMinors(const std::map<CString, CMajor*>& pmMajors);
	void CalcPreLoop();

private:
	CBotf2Doc* m_pDoc;
};