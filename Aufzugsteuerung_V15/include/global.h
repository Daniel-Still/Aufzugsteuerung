#include <Arduino.h> 
#include <AccelStepper.h>         // Schrittmotor

// Erwähnung aller Funktionen, die die main und die jeweilige Quelldatei kennen muss 
void func_Referenzierung();
void func_Referenz_erster_Stock();
void func_zweiter_Stock_von_unten();
void func_erster_Stock_von_oben();
void func_dritter_Stock_von_unten();
void func_zweiter_Stock_von_oben();
void func_vierter_Stock_von_unten();
void func_dritter_Stock_von_oben();
void func_fuenfter_Stock_von_unten();
void func_vierter_Stock_von_oben();
void func_sechster_Stock_von_unten();
void func_fuenfter_Stock_von_oben();

int get_Referenz_erster_Stock_state();
int get_zweiter_Stock_von_unten_state();
int get_dritter_Stock_von_unten_state();
int get_vierter_Stock_von_unten_state();
int get_fuenfter_Stock_von_unten_state();
int get_sechster_Stock_von_unten_state();
int get_erster_Stock_von_oben_state();
int get_zweiter_Stock_von_oben_state();
int get_dritter_Stock_von_oben_state();
int get_vierter_Stock_von_oben_state();
int get_fuenfter_Stock_von_oben_state();
// Erwähnung aller Funktionen, die die main und die jeweilige Quelldatei kennen muss 




extern AccelStepper stepper1;     // Stepper extern deklariert, damit in jeder Quelldatei der Bewegungsbefehl erfolgen kann

// Timer-Baustein global bekannt 
struct TON_typ {
  bool IN;                        // Eingang, dass Timer gestartet wird
  bool IN_OLD;                    // Merker, was vorheriger Eingang war
  bool TIMER_ALARM;               // HIGH oder LOW, ob PRESET_TIME abgelaufen ist
  unsigned long ELAPSED_TIME;     // Zeit, die seit dem Start des Timers vergangen ist
  unsigned long PRESET_TIME;      // Vorgabezeit für den Timer
  unsigned long START_TIME;       // Startzeit des Timers
};

extern TON_typ Timer_Tuer_oeffnen;  // Tür öffnen/schließen Timer wird in main.cpp und anderen Quelldateien benötigt
extern TON_typ Timer_Tuer_schliessen;
// Timer-Baustein global bekannt

extern int Hauptfunktionen_state;        // für switch Anweisung - 1: Kalibrierungsfahrt - 2: Aufzug Normalbetrieb - 3: Notbetrieb
#define Referenzierung 1
#define Referenz_erster_Stock 2
#define zweiter_Stock_von_unten 3
#define dritter_Stock_von_unten 4
#define vierter_Stock_von_unten 5
#define fuenfter_Stock_von_unten 6
#define sechster_Stock_von_unten 7
#define erster_Stock_von_oben 22
#define zweiter_Stock_von_oben 33
#define dritter_Stock_von_oben 44
#define vierter_Stock_von_oben 55
#define fuenfter_Stock_von_oben 66



extern bool Taster_aussen[7];
extern bool Taster_innen[7];
extern bool LED[7];
extern bool Stockwerk_gerufen[7];
extern bool Positionssensor;
extern bool Blinker;
extern bool Blinker_Notruf;
extern bool Notrufbetrieb;

// Positionen der Stockwerke, vorher durch manuelles Anfahren der Stockwerke ermittelt
#define Pos_Stockwerk_1 991
#define Pos_Stockwerk_2 4939
#define Pos_Stockwerk_3 8803
#define Pos_Stockwerk_4 12624
#define Pos_Stockwerk_5 16439
#define Pos_Stockwerk_6 20199
// Positionen der Stockwerke, vorher durch manuelles Anfahren der Stockwerke ermittelt

#define max_brems_steps 670 // maximaler Bremsweg, der zum Halten nicht unterschritten werden darf, um sicher in einem Stockwerk zu halten