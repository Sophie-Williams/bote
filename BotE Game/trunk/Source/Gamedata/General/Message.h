/*
 *   Copyright (C)2004-2011 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de
 *
 */
// Message.h: Schnittstelle f�r die Klasse CMessage.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGE_H__7E51BA90_EEDD_455F_9324_542F163F91AD__INCLUDED_)
#define AFX_MESSAGE_H__7E51BA90_EEDD_455F_9324_542F163F91AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Options.h"

class CMessage : public CObject  
{
public:
	// Klasse serialisierbar machen
	DECLARE_SERIAL (CMessage)
	
	/// Konstruktor
	CMessage(void);
	
	/// Destruktor	
	virtual ~CMessage(void);
	
	/// Kopierkonstruktor
	CMessage(const CMessage & rhs);
	
	/// Zuweisungsoperator
	CMessage & operator=(const CMessage &);
	
	/// Die Serialisierungsfunktion
	virtual void Serialize(CArchive &ar);
	
	// Zugriffsfunktionen
	// zum Lesen der Membervariablen
	MESSAGE_TYPE::Typ GetMessageType(void) const {return m_nMessageType;}
	const CPoint& GetKO(void) const {return m_KO;}
	BYTE GetFlag(void) const {return m_byFlag;}
	const CString& GetMessageString(void) const {return m_strMessage;}
		
	// sonstige Funktionen
	void GenerateMessage(const CString& sMessage, MESSAGE_TYPE::Typ nMessageType, const CString& sSystemName, const CPoint& SystemKO, bool bUpdate, BYTE byFlag = 0);
	
private:
	CString	m_strMessage;
	MESSAGE_TYPE::Typ m_nMessageType;
	CPoint	m_KO;			// Koordinaten des Systems, worauf sich die Nachricht bezieht
	BYTE	m_byFlag;		// irgendeine Zusatzinformation, die ich mit �bergeben kann
};

typedef CArray<CMessage,CMessage> MessageArray;		// Das dynamische Feld wird vereinfacht als MessageArray angegeben


#endif // !defined(AFX_MESSAGE_H__7E51BA90_EEDD_455F_9324_542F163F91AD__INCLUDED_)



/* TODO:

1.Testen ob sich die Beziehung der Minorrace �ndert, wenn wir ihrem B�ndnispartner (Majorrace) den
Krieg erkl�ren. F�r uns sollte die Beziehung sich enorm verschlechtern (auf 0) und f�r die 
Majorrace, mit welche mit dieser Minorrace mindst. ein B�ndnis hat leicht verbessern. Aktuell
ist das zwischen 0 und 15 Punkten. -> funzt erstmal

2.Wenn der Computer eine Majorrace spielt und wir ihr den Krieg erkl�ren, dann Beziehung verschlechtern.
Auch Beziehung der B�ndnispartner dann verschlechtern, wenn wir den Krieg erkl�rt haben. -> funzt

--> ab hier

3.Als n�chstes sollte der Kriegspakt eingebaut werden. Danach sind erstmal alle diplomatischen Angebote
implementiert. Dann k�nnte man die Auswirkungen dieser Vertr�ge einbauen und abschlie�end die
Computer-KI. -> funzt

4.KI weiter implementieren. 
Es fehlt noch Geschenke geben und die Beziehungsverbesserung bei Geschenken. (-> funzt)
Mitgifte bei Vertragsangeboten werden noch nicht beachtet. (jetzt schon ;-) -> funzt)
Die KI gibt noch nicht selbstst�ndig Mitgifte (-> funzt eigentlich alles)
und die Dauer eines Vertrages wird noch nicht beim Angebot von einer KI-Majorrace beachtet. (-> funzt)
Kriegspakte, Geschenke und Forderungen werden auch noch nicht angeboten. (-> funzt)
Die Reaktion auf eine Forderung ist auch noch nicht implementiert. (-> funzt)
Diplomatische Texte bei einem Angebot von einer KI-Majorrace sind noch nicht vorhanden. (-> funzt)

5.KI soll auch kleinen Rassen Angebote machen. Das irgendwie so machen, das die KI nicht ihr Geld sinnlos
verpulvert, sondern gescheite Angebote macht. (-> funzt gr��tenteils, es fehlt noch Geschenke geben, Bestechen (-> funzt)
und vlt. auch Vertrag aufheben (eher unwahrscheinlich), bei Kriegserkl�rung m�gliche B�ndnisse und Pakte
beachten (ich will nicht einem Freund zuf�llig den Krieg erkl�ren), Beziehung bei Rassen denen wir nicht (-> funzt)
direkt den Krieg erkl�rt haben nicht immer auf 0 setzten (auch mal nur 50 Punkte abziehen oder so)) (-> funzt)

6.Dauer eines Vertrages sollte auch noch angezeigt werden + farblich die Vertragsart differenzieren (-> funzt)

7.Schiffsdesignbildschirm fertig stellen! Dazu geh�rt noch die M�glichkeit die Art des Torpedos zu w�hlen, sowie
die Art des Torpedowerfers w�hlen zu k�nnen. (-> funzt)

8.Geb�ude abrei�en mu� jetzt in eine dynamische Variable gespeichert werden. Es kann sein, dass wir deswegen auch
manche Zugriffsfunktionen und weitere �ndern m�ssen. (->funzt)

9.Baubare Geb�ude und Updates Berechnungsfunktion wiedermal neu schreiben. (->funzt)
Geb�udeinformationen �berarbeiten. Es gibt viele neue. M�ssen die anders einlesen, bin�r. (-> funzt)
Dadurch neue Geb�udeeigenschaften hinzuf�gen und auch deren Behandlung/Beachtung implementieren. (-> funzt)
Baulistencheck noch hinzuf�gen. (-> weiter arbeiten: Updates gehen schon, noch Max X beachten!) (-> funzt)
Imperienweite Voraussetzungen noch �berpr�fen. (-> funzt)

10. BUG: Bei Meeresfarm (glaub geb�ude mit Attribut "immer online") kommt Meldung, das die Arbeiter fehlen (-> funzt)

11. BUG: Wenn ich eine Mitgliedschaft (vlt. auch bei anderen Vertrag) aufhebe und das Spiel sp�ter verlasse, kommt
	     eine Fehlermeldung 


LATER TODO:
1. Auswirkungen eines Vertrages komplett programmieren. Bis jetzt haben wir die Auswirkungen eines Vertrages,
aber sobald der Vertrag endet, dann m�ssen die Auswirkungen teilweise wieder r�ckg�ngig gemacht werden.
Das geht erst dann, wenn wir die Scannung und Schiffswerften & Basen richtig implementiert haben. Also jede
neue Runde diese Werte abgefragt werden. -> (m�sste funzen)

2. Rassenabh�ngige Texte bei diplomatischen Angeboten programmieren. (-> ist da --> Arbeit f�r den Texter)

3. Wenn wir einen Krieg erkl�rt bekommen und wir aktuell noch einen Vertrag mit der kriegserkl�renden Rasse hatten,
dann Nachricht von K�ndigung des Vertrages.

4. Die Sektormarkierungen (also wem der Sektor geh�rt) nur dann zeichnen, wenn wir diese gro�e Rasse auch schon
kennengelernt haben.

5. Ob eine Minorrace einen Vertrag von uns annimmt noch von der Art der kleinen Rasse abh�ngig machen.
Bis jetzt wird die Eigenschaft nur bei Credits-, Ressourcengabe und bei einer m�glichen Kriegserkl�rung
an uns ber�cksichtigt. Z.B. nehmen geheimnisvolle Minorraces nicht so schnell ne Freundschaft an. (-> funzt)

6. ShipPort mit beachten, wenn wir eine Station einem Sektor bekanntgeben. Genauso auch ShipPort anlegen, wenn wir
eine Werft in dem System haben.

7. Wenn wir einen Bauauftrag (speziell ein Schiff) kaufen, wird noch nicht das n�tige Deritium mit in den Kaufpreis
eingerechnet. Das mal testen ob es notwendig ist.

8. Kopierkonstruktoren und Zuweisungsoperatoren in allen Klassen checken ob diese auch funktionieren.

9. In der Klasse CSystem in der Funktion CalculateBuildableBuildings() Punkt 0.2 �berpr�fen, ob ich wirklich
alle potentiellen Vorg�nger in der Liste suchen mu�, oder ob es reicht wie es jetzt ist. Indem ich immer nur
den n�chsten Vorg�nger �berpr�fe.

10. Wenn wir von einem Geb�ude nur eine maximale Anzahl auf einem System oder im Imperium haben d�rfen und diese 
maximale Anzahl erreicht haben, verschwindet dieses Geb�ude auch richtig aus der Bauliste. Entfernen wir ein solches
Geb�ude aus der AssemblyList, so erscheint dieses richtigerweise auch wieder in der Bauliste. Wenn wir aber die
maximale Anzahl erreicht haben, eine Runde weiterklicken und dann erst das Geb�ude aus der AssemblyList nehmen, so
erscheint dieses nicht in der Bauliste. Noch eine Runde sp�ter erscheint es aber wieder. Dies liegt daran, dass die 
Variable "m_BuildableWithoutAssemblyListCheck" dieses Geb�ude in der neuen Runde nicht mehr f�hrt, weil es ja eigentlich
nicht mehr baubar ist.
*/
