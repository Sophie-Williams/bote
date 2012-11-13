#pragma once

class CShips;
class CShip;
class CSector;
class CSystem;
class CBotf2Doc;

#include <set>


class CSanity
{
	private:
		CSanity(void);
	public:
		~CSanity(void);


	static void SanityCheckFleet(const CShips& ship);
	static void SanityCheckSectorAndSystem(
		const CSector& sector,
		const CSystem& system,
		const CBotf2Doc& doc);
	static void ShipInfo(const CArray<CShip, CShip>& shiparray, int index, const CString& indexname);
	static void CheckShipUniqueness(const CShips& ship, std::set<CString>& already_encountered);
	//static void ShipArray(const CBotf2Doc& doc);


};
