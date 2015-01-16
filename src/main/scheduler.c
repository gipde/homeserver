/*! \file \brief Scheduler
 *
 * This scheduler is the heart of home automation
 *
 * Über einen Timerinterrupt muss der Dispatcher immer wieder die Kontrolle erhalten
 *
 * Das Hauptprogramm muss sowohl eine dynamische Zeittabelle abarbeiten
 * als auch Anfragen von Aussen (Webserver, Display, Taster) in echtzeit verarbeiten.
 *
 * 1. Timerinterrupt unterbricht laufende Tätigkeit.
 *  Zeitscheibenverfahren
 * 	wie geht das?
 * 	wann wird dann an die alte stelle zurückgekehrt
 * 	- ein Thread der den Zeitplan abarbeitet
 * 		- die Aufgaben werden in weiteren Threas getriggert
 * 		- teilweise (je nach sensor) auch seriell
 * 	- ein Thread der als HTTP-Server arbeitet
 * 	- ein Thread der als Konsolen-Eingabe arbeitet
 * 	- ein Thread für die Display-/Tastenansteuerung
 *
 *
 */


typedef struct  {
		int8_t year;
		int8_t month;
		int8_t day;
		int8_t hour;
		int8_t minute;
		int8_t second;
} date;

// wir brauchen eine verkettete Liste mit Event-Elementen. Wenn eins erledigt ist,
// muss der Pointer auf das nächste gesetzt werden. Damit vermeiden wir eine Doppelbearbeitung


typedef struct  {
		date event;
		date* next;
} plan_element;

static struct plan_element* next_event;


date* get_current_date();

void gen_schedule_plan() {
	add((plan_element){-1,-1,-1,5,
	plan_element first_e;

}

void schedule() {
	// TODO: Read plan from sdcard
	for(;;) {
		date* d = get_current_date();
		if 
	}

void run() {
	debug("Starte SchedulerThread");


}
