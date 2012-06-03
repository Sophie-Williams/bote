/*
 *   Copyright (C)2004-2011 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de
 *
 */
#pragma once
#include "GraphicPool.h"

/**
 * Buttonklasse welche verschiedene Zust�nde eines Buttons darstellen kann. Gleichzeitig bietet diese Klasse
 * eine gute Abstraktion, um einfach Klicks abzufangen und Zeichenoperationen durchf�hren zu k�nnen.
 *
 * @author Sir Pustekuchen
 * @version 0.0.2
 */
class CMyButton : public CObject
{
public:
	/// Konstruktor mit kompletter Parameter�bergabe.
	/// @param point linke obere Koordinate des Buttons
	/// @param size Gr��e des Buttons in x- und y-Richtung
	/// @param text der Text auf dem Button
	/// @param normGraphicName Name des Grafikfiles f�r die normale Darstellung
	/// @param inactiveGraphicName Name des Grafikfiles f�r die inaktive Darstellung
	/// @param activeGraphicName Name des Grafikfiles f�r die aktive Darstellung
	CMyButton(CPoint point, CSize size, const CString& text,
		const CString& normGraphicName, const CString& inactiveGraphicName, const CString& activeGraphicName);

	/// Destruktor
	~CMyButton(void);

	// Zugriffsfunktionen
	/// Funktion gibt den aktuellen Zustand des Buttons zur�ck.
	BYTE GetState() const {return m_byStatus;}

	/// Funktion gibt das ben�tigte Rechteck des Buttons zur�ck.
	CRect GetRect() const {return CRect(m_KO.x, m_KO.y, m_KO.x+m_Size.cx, m_KO.y+m_Size.cy);}

	/// Funktion gibt den Text des Buttons zur�ck
	const CString& GetText() const {return m_strText;}

	/// Funktion setzt den Status/Zustand des Buttons.
	/// @param newState neuer Status f�r den Button (0 -> normal, 1 -> aktiviert, sonst deaktiviert)
	void SetState(BYTE newState) {m_byStatus = newState;}

	/// Funktion �ndert den Text auf dem Button.
	/// @param text Text auf dem Button
	void SetText(const CString& text) {m_strText = text;}

	// Funktionen
	/// Funktion �berpr�ft, ob der �bergebene Punkt (z.B. Mausklick) in dem Feld des Buttons liegt.
	/// @param pt der zur �berpr�fende Punkt
	BOOLEAN ClickedOnButton(const CPoint& pt);

	/// Diese Funktion zeichnet den Button in den �bergebenen Ger�tekontext.
	/// @param g Referenz auf Graphics Objekt
	/// @param graphicPool Zeiger auf die Sammlung aller Grafiken
	/// @param font Referenz auf zu benutzende Schrift
	/// @param brush Referenz auf Farbepinsel f�r Font (Schriftfarbe)
	void DrawButton(Gdiplus::Graphics &g, CGraphicPool* graphicPool, Gdiplus::Font &font, Gdiplus::SolidBrush &brush);

	/// Funktion aktiviert den Button. Wenn dieser inaktiv ist, kann er nicht aktiviert werden.
	BOOLEAN Activate();

	/// Funktion deaktiviert den Button. Dieser kehrt dann zu seinem alten Zustand zur�ck.
	BOOLEAN Deactivate();

private:
	// Attribute
	BYTE m_byStatus;				///< Der Status des Buttons, also ob normal, inaktiv oder aktiv.
	CString m_strText;				///< Der Text auf dem Button.
	const CPoint m_KO;				///< Koordinate der linken oberen Ecke des Buttons.
	const CSize m_Size;				///< Die Gr��e in x und y Richtung des Buttons.
	const CString m_strNormal;		///< Die Grafik f�r den Button im normalen Zustand.
	const CString m_strInactive;	///< Die Grafik f�r den Button im deaktivierten Zustand.
	const CString m_strActive;		///< Die Grafik f�r den Button im aktivierten Zustand.
};
