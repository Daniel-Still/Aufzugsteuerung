// link https://github.com/sstaub/LCDi2c/tree/master

// Allgemeine Informationen:
// Die Beschreibung des Programmcodes wird nur auf main.cpp; Referenzierung.cpp; zweiter_Stock_von_unten; zweiter_Stock_von_oben; erster_Stock_von_oben vorgenommen.
// Referenzierung und Referenz_erster_Stock sind Sonderfälle. erster_Stock_von_oben ist vergleichbar mit sechster_Stock_von_unten.
// zweiter_Stock_von_unten und zweiter_Stock_von_oben sind 1 zu 1 aufgebaut (nur umgekehrte Funktionsweise auf/abfahrt), wie die restlichen Stockwerke, die von oben/unten anfahrbar sind.
// Der Notrufbetrieb funktioniert identisch wie eine Normale fahrt ins nächste Stockwerk, nur dass dort die Anfragen auch zurückgesetzt werden

#include <Arduino.h>
#include <AccelStepper.h>         // Schrittmotor
#include <LCDi2c.h>               // 4x20 LCD i2c
#include <Keypad.h>               // Keypad

#include <global.h>               // globale header-Datei 

int Hauptfunktionen_state = 1;    // für switch Anweisung - Referenzierung und Fahrten von oben/unten zwischen allen Stockwerken


bool Taster_aussen[7];            // Taster
bool Taster_innen[7];             // Keypad
bool LED[7];                      // LEDs außen, rot
bool Stockwerk_gerufen[7];        // Variable, die weiter unten von Taster aussen/innen gesetzt wird
bool Positionssensor;             // Näherungsschalter bzw. Hall-Sensor (magnetisch)
bool Blinker;                     // Blinkfunktion für Türe
bool Blinker_Notruf;              // Blinkfunktion für Notbetrieb
bool Notrufbetrieb = 0;           // Variable für Notbetrieb * und # durch Keypad

////////////////////////////////////////////////////// Funktionsdeklarationen

void Eingaenge_abfragen();        // Funktion fragt alle Eingänge ab, von Taster, Keypad, Näherungssensor
void Ausgaenge_setzen();          // Funktion setzt alle Ausgänge wie LEDs

////////////////////////////////////////////////////// Funktionsdeklarationen
////////////////////////////////////////////////////// Variablendeklarationen

char customKey;                   // für Keypad

int Position_Stockwerk[] =        // für LCD Stockwerk und Bewegungsrichtung - for-Schleife
{0, Pos_Stockwerk_1, Pos_Stockwerk_2, Pos_Stockwerk_3, Pos_Stockwerk_4, Pos_Stockwerk_5, Pos_Stockwerk_6};

////////////////////////////////////////////////////// Variablendeklarationen
////////////////////////////////////////////////////// LEDs / Taster / Position_Sensor - Definierung Pins

#define LED_1 40
#define LED_2 42
#define LED_3 44  
#define LED_4 46 
#define LED_5 48  
#define LED_6 50  
  
#define TASTER_1_PIN 41
#define TASTER_2_PIN 43
#define TASTER_3_PIN 45
#define TASTER_4_PIN 47
#define TASTER_5_PIN 49
#define TASTER_6_PIN 51

#define POSITION_SENSOR_PIN 39

////////////////////////////////////////////////////// LEDs / Taster / Position_Sensor - Definierung Pins
/////////////////////////////////////////////////////////////////////////////////////////////////// Keypad

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {37, 35, 33, 31}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {29, 27, 25, 23}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

/////////////////////////////////////////////////////////////////////////////////////////////////// Keypad
/////////////////////////////////////////////////////////////////////////////////////////////////// Stepper

const byte Fullstep = 4;
const byte Halfstep = 8;
AccelStepper stepper1(Halfstep, 11, 9, 10, 8);

/////////////////////////////////////////////////////////////////////////////////////////////////// Stepper
/////////////////////////////////////////////////////////////////////////////////////////////////// LCD

#define LCD_CHARS   20
#define LCD_LINES   4
LCDi2c lcd(0x27);   // I2C Adresse

/////////////////////////////////////////////////////////////////////////////////////////////////// LCD
/////////////////////////////////////////////////////////////////////////////////////////////////// TON (struct in global.h , da überall bekannt sein muss)

/*struct TON_typ {
  bool IN;                        // Eingang, dass Timer gestartet wird
  bool IN_OLD;                    // Merker, was vorheriger Eingang war (also quasi, ob IN schon HIGH oder LOW war, also schauen, ob Flankenwechsel gekommen ist)
  bool TIMER_ALARM;               // HIGH oder LOW, ob PRESET_TIME abgelaufen ist
  unsigned long ELAPSED_TIME;     // Zeit, die ab Timerstart vergangen ist
  unsigned long PRESET_TIME;      // Zeit, ab wann TIMER_ALARM ausgelöst wird (Weck-Zeit)
  unsigned long START_TIME;       // Wert von millis() bei steigender Flanke
};*/

void TON(TON_typ *pTON) {
  if (pTON->IN == 0) {                                                    // wenn input IN auf 0 gesetzt wird, alles zurücksetzen
    pTON->ELAPSED_TIME = 0;
    pTON->TIMER_ALARM = 0;
    pTON->IN_OLD = 0;
  } else {
    if (pTON->IN_OLD == 0) {                                              // Flanke also wechsel von IN hat stattgefunden
      pTON->IN_OLD = 1;
      pTON->START_TIME = millis();
    } else {
      unsigned long current_time = millis();
      if (current_time >= pTON->START_TIME) {                             // kein Überlauf
        pTON->ELAPSED_TIME = current_time - pTON->START_TIME;
      } else {                                                            // Überlauf
        pTON->ELAPSED_TIME = 0xFFFF - pTON->START_TIME + current_time;
      }

      if (pTON->ELAPSED_TIME >= pTON->PRESET_TIME) {                      // Timer erreicht
        pTON->TIMER_ALARM = 1; 
        pTON->ELAPSED_TIME = pTON->PRESET_TIME;                           // Zeit auf PRESET setzen
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////// TON (struct in global.h , da überall bekannt sein muss)
/////////////////////////////////////////////////////////////////////////////////////////////////// TON anlegen

TON_typ Timer_Tuer_oeffnen;
TON_typ Timer_Tuer_schliessen;
TON_typ Timer_Blinken;
TON_typ Timer_Blinken_Notruf;
TON_typ Timer_LCD_100ms;
TON_typ Timer_loop_10ms;

/////////////////////////////////////////////////////////////////////////////////////////////////// TON anlegen
////////////////////////////////////////////////////////// Interrupt Routine für run-Befehl Schrittmotor

ISR(TIMER1_COMPA_vect) 
{
  stepper1.run();             // Stepper run-Funktion wird im Interrupt ausgeführt
}

////////////////////////////////////////////////////////// Interrupt Routine für run-Befehl Schrittmotor
///////////////////////////////////////////////////////////// Sonderzeichen LCD

byte ae[8] =  // Buchstabe "ä"
{  
  B01010,
  B00000,
  B01110,
  B00001,
  B01111,
  B10001,
  B01111,
  B00000
};

byte ue[8] =  // Buchstabe "ü"
{  
  B01010,  
  B00000,  
  B10001,  
  B10001,  
  B10001, 
  B10011,  
  B01101,  
  B00000 
};

byte oe[8] =  // Buchstabe "ö"
{  
  B01110,  
  B00000,  
  B01110,  
  B10001,  
  B10001, 
  B10001,  
  B01110,  
  B00000 
};

byte pfeil_hoch[8]= // Pfeil nach oben
{
  B00100,
  B01110,
  B11111,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100
};

byte pfeil_runter[8]= // Pfeil nach unten
{
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B11111,
  B01110,
  B00100
};

byte pfeil_links[8]= // Pfeil nach links
{
  B00000,
  B00100,
  B01100,
  B11111,
  B01100,
  B00100,
  B00000,
  B00000
};

byte pfeil_rechts[8]= // Pfeil nach rechts
{
  B00000,
  B00100,
  B00110,
  B11111,
  B00110,
  B00100,
  B00000,
  B00000
};

///////////////////////////////////////////////////////////// Sonderzeichen LCD



void setup() 
{

  Serial.begin(9600);
  Timer_Tuer_oeffnen.PRESET_TIME = 2500;      // Tür offen 2.5s
  Timer_Tuer_schliessen.PRESET_TIME = 2500;   // Tür schließen 2.5s
  Timer_Blinken.PRESET_TIME = 500;            // Normales Blinken 0.5s Takt
  Timer_Blinken_Notruf.PRESET_TIME = 100;     // Notruf Blinken 0.1s Takt
  Timer_LCD_100ms.PRESET_TIME = 100;          // für LCD, nur alle 100ms aktualisiert
  Timer_loop_10ms.PRESET_TIME = 10;           // für Hauptprogramm, nur alle 10ms ausgeführt

/////////////////////////////////////////////////////////////  LCD Sonderzeichen

  lcd.begin(LCD_CHARS, LCD_LINES);
  lcd.create(0, ae);
  lcd.create(1, pfeil_hoch);
  lcd.create(2, pfeil_runter);
  lcd.create(3, ue);
  lcd.create(4, oe);
  lcd.create(5, pfeil_links);
  lcd.create(6, pfeil_rechts);

/////////////////////////////////////////////////////////////  LCD Sonderzeichen
////////////////////////////////////////////////////// LEDs / Taster / Positionssensor

  pinMode(TASTER_1_PIN, INPUT_PULLUP);  // Taster 1
  pinMode(TASTER_2_PIN, INPUT_PULLUP);  // Taster 2
  pinMode(TASTER_3_PIN, INPUT_PULLUP);  // Taster 3
  pinMode(TASTER_4_PIN, INPUT_PULLUP);  // Taster 4
  pinMode(TASTER_5_PIN, INPUT_PULLUP);  // Taster 5
  pinMode(TASTER_6_PIN, INPUT_PULLUP);  // Taster 6

  pinMode(LED_1, OUTPUT);        // LED 1
  pinMode(LED_2, OUTPUT);        // LED 2
  pinMode(LED_3, OUTPUT);        // LED 3
  pinMode(LED_4, OUTPUT);        // LED 4
  pinMode(LED_5, OUTPUT);        // LED 5
  pinMode(LED_6, OUTPUT);        // LED 6

  pinMode(POSITION_SENSOR_PIN, INPUT_PULLUP);  // Positionstaster

////////////////////////////////////////////////////// LEDs / Taster / Positionssensor
////////////////////////////////////////////////////// Stepper Einstellungen

  stepper1.setMaxSpeed(1000);
  stepper1.setAcceleration(1000);
  stepper1.setCurrentPosition(0);

////////////////////////////////////////////////////// Stepper Einstellungen
/////////////////////////////////////////////////////////// Interrupt setup

  noInterrupts();         // Interrupts deaktivieren
  TCCR1A = 0;             // Steuerregister A auf 0 setzen
  TCCR1B = 0;             // Steuerregister B auf 0 setzen
  TCNT1  = 0;      // Zähler auf 0 setzen
  OCR1A = 1000;     // (16MHz ) * 100us = 1000 Takte
  TCCR1B |= (1 << WGM12);  // CTC-Modus
  TCCR1B |= (1 << CS11);   // Prescaler auf 8
  TIMSK1 |= (1 << OCIE1A); // Timer Compare Interrupt aktivieren
  interrupts();  // Interrupts aktivieren

/////////////////////////////////////////////////////////// Interrupt setup

}




void loop() 
{
////////////////////////////////////////////////////// TON Timer

  TON(&Timer_LCD_100ms);
  Timer_LCD_100ms.IN = 1;

  TON(&Timer_loop_10ms);
  Timer_loop_10ms.IN = 1;

////////////////////////////////////////////////////// TON Timer
////////////////////////////////////////////////////////////////////////////////////////////////////////////// Timer 10ms Hauptprogramm
  if (Timer_loop_10ms.TIMER_ALARM == 1) 
  {
    TON(&Timer_Blinken);
    Timer_Blinken.IN = 1;
    if(Timer_Blinken.TIMER_ALARM == 1)        // Auslösung Timer-Alarm
    {
      Timer_Blinken.IN = 0;
      Blinker =! Blinker;                     // Blinken bzw abwechselndes Signal 
    }

    TON(&Timer_Blinken_Notruf);
    Timer_Blinken_Notruf.IN = 1;
    if(Timer_Blinken_Notruf.TIMER_ALARM == 1) // Auslösung Timer-Alarm
    {
      Timer_Blinken_Notruf.IN = 0;
      Blinker_Notruf =! Blinker_Notruf;       // Blinken bzw abwechselndes Signal 
    }


    TON(&Timer_Tuer_oeffnen);
    TON(&Timer_Tuer_schliessen);

    Timer_loop_10ms.IN = 0;                   // 10ms-Timer zurücksetzen für neuen Durchlauf
    
    Eingaenge_abfragen();

    for(int i=1; i<=6; i++)                   
    {
      if((Taster_innen[i] == 1) || (Taster_aussen[i] == 1))
      {
        Stockwerk_gerufen[i] = 1;             
      }
      LED[i] = Stockwerk_gerufen[i];
      if(Notrufbetrieb == 1)                  // wenn Notrufbetrieb aktiv, werden alle Rufsignale gelöscht, dass wenn Notrufbetrieb deaktiviert wird, der Aufzug nicht direkt den alten Betrieb aufnimmt, sondern erst auf neue Anfragen wartet
      {
        Stockwerk_gerufen[i] = 0;
      }
  
    }

    // Wenn Notruf aktiv, setzt alle LEDs in Blinker_Notruf, jedoch wird Tür öffnen Blinker später noch geändert und angepasst ausgegeben
    if(Notrufbetrieb == 1)
    {
      for(int i=1; i<=6; i++)
      {
        LED[i] = Blinker_Notruf;
      }
    }


    switch(Hauptfunktionen_state)             // Haupt-State-Machine und Ablauf des Hauuptprogramms
    {
      case Referenzierung:
        func_Referenzierung();
        break;

      case Referenz_erster_Stock:
        func_Referenz_erster_Stock();
        break;
      
      case zweiter_Stock_von_unten:
        func_zweiter_Stock_von_unten();
        break;

      case erster_Stock_von_oben:
        func_erster_Stock_von_oben();
        break;

      case dritter_Stock_von_unten:
        func_dritter_Stock_von_unten();
        break;
      
      case zweiter_Stock_von_oben:
        func_zweiter_Stock_von_oben();
        break;
      
      case vierter_Stock_von_unten:
        func_vierter_Stock_von_unten();
        break;

      case dritter_Stock_von_oben:
        func_dritter_Stock_von_oben();
        break;

      case fuenfter_Stock_von_unten:
        func_fuenfter_Stock_von_unten();
        break;

      case vierter_Stock_von_oben:
        func_vierter_Stock_von_oben();
        break;

      case sechster_Stock_von_unten:
        func_sechster_Stock_von_unten();
        break;

      case fuenfter_Stock_von_oben:
        func_fuenfter_Stock_von_oben();
        break;
    }
    


    Ausgaenge_setzen();
  }
////////////////////////////////////////////////////////////////////////////////////////////////////////////// Timer 10ms Hauptprogramm
////////////////////////////////////////////////////////////////////////////////////////////////////////////// Timer 100ms LCD printen
  if (Timer_LCD_100ms.TIMER_ALARM == 1) 
  {
    Timer_LCD_100ms.IN = 0;

    //LCD Zeile 1 - gerufene Stockwerke
    lcd.locate(0, 0);
    lcd.print("Gerufen: |");
    lcd.locate(9, 0);
    if(Notrufbetrieb == 0)
    {
      for(int i=1; i<=6; i++)
      {
        if(Stockwerk_gerufen[i] == 1)
        {
          lcd.locate((i+9), 0);
          lcd.print(i);
        }
        else
        {
          lcd.locate((i+9), 0);
          lcd.print("-");
        }
      }
      lcd.print("|");
    }
    else
    {
      lcd.print("Nothalt!");
    }
    //LCD Zeile 1 - gerufene Stockwerke
    //LCD Zeile 2 - Zustand Stockwerke (Bewegungsrichtung, nächstes Stockwerk, Halt)

    lcd.locate(0,1);
    if((stepper1.speed() > 0) && (Hauptfunktionen_state != Referenzierung)) // hoch
    {
      lcd.print("N");
      lcd.write((uint8_t)0);
      lcd.print("chster Stock: ");
      for(int i=1; i<=6; i++)
      {
        if((Position_Stockwerk[i] < stepper1.currentPosition()) && (stepper1.currentPosition() < Position_Stockwerk[i+1]))
        {
          lcd.print(i+1);
        }
      }
      lcd.write((uint8_t)1);
    }
    else if((stepper1.speed() < 0) && (Hauptfunktionen_state != Referenzierung)) // runter
    {
      lcd.print("N");
      lcd.write((uint8_t)0);
      lcd.print("chster Stock: ");
      for(int i=1; i<=6; i++)
      {
        if((Position_Stockwerk[i] < stepper1.currentPosition()) && (stepper1.currentPosition() < Position_Stockwerk[i+1]))
        {
          lcd.print(i);
        }
      }
      lcd.write((uint8_t)2);
    }
    else if((stepper1.speed() == 0) && (Hauptfunktionen_state != Referenzierung))  // Stillstand
    {
      lcd.print("Halt bei Stock: ");
      for(int i=1; i<=6; i++)
      {
        if(stepper1.currentPosition() == Position_Stockwerk[i])
        {
          lcd.print(i);
          lcd.print(" ");
        }
      }      
    }

    //LCD Zeile 2 - Zustand Stockwerke (Bewegungsrichtung, nächstes Stockwerk, Halt)
    //LCD Zeile 3 - Zustand Türe
    lcd.locate(0,2);
   
    if((get_Referenz_erster_Stock_state() == 2) || (get_zweiter_Stock_von_unten_state() == 3) || (get_dritter_Stock_von_unten_state() == 3) 
    || (get_vierter_Stock_von_unten_state() == 3) || (get_fuenfter_Stock_von_unten_state() == 3) || (get_sechster_Stock_von_unten_state() == 2) 
    || (get_erster_Stock_von_oben_state() == 2) || (get_zweiter_Stock_von_oben_state() == 3) || (get_dritter_Stock_von_oben_state() == 3) 
    || (get_vierter_Stock_von_oben_state() == 3) || (get_fuenfter_Stock_von_oben_state() == 3) )  // get-Funktionen für Zustand Türe
    {
      lcd.print("T");
      lcd.write((uint8_t)3);
      lcd.print("re ");
      lcd.write((uint8_t)4);
      lcd.print("ffnet     ");
      lcd.print("|");
      lcd.write((uint8_t)5);
      lcd.write((uint8_t)6);
      lcd.print("|");

    }
    else if((get_Referenz_erster_Stock_state() == 3) || (get_zweiter_Stock_von_unten_state() == 4) || (get_dritter_Stock_von_unten_state() == 4) 
    || (get_vierter_Stock_von_unten_state() == 4) || (get_fuenfter_Stock_von_unten_state() == 4) || (get_sechster_Stock_von_unten_state() == 3) 
    || (get_erster_Stock_von_oben_state() == 3) || (get_zweiter_Stock_von_oben_state() == 4) || (get_dritter_Stock_von_oben_state() == 4) 
    || (get_vierter_Stock_von_oben_state() == 4) || (get_fuenfter_Stock_von_oben_state() == 4) )
    {
      lcd.print("T");
      lcd.write((uint8_t)3);
      lcd.print("re schliesst  ");      
      lcd.print("|");
      lcd.write((uint8_t)6);
      lcd.write((uint8_t)5);
      lcd.print("|");
    }
    else
    {
      lcd.print("T");
      lcd.write((uint8_t)3);
      lcd.print("re geschlossen || ");
    }


    //LCD Zeile 3 - Zustand Türe
    //LCD Zeile 4 - Betriebsmodus (Referenz, Normal, Not)
    if(Hauptfunktionen_state == Referenzierung)
    {
      lcd.locate(0, 3);
      lcd.print("Referenzierung");
    }
    else if(Notrufbetrieb == 1)
    {
      lcd.locate(0, 3);
      lcd.print("Notbetrieb    ");
    }
    else
    {
      lcd.locate(0,3);
      lcd.print("Normalbetrieb ");
    }

    //LCD Zeile 4 - Betriebsmodus (Referenz, Normal, Not)

  }
////////////////////////////////////////////////////////////////////////////////////////////////////////////// Timer 100ms LCD printen
}



void Eingaenge_abfragen(void)
{
    Taster_aussen[1] = !digitalRead(TASTER_1_PIN);
    Taster_aussen[2] = !digitalRead(TASTER_2_PIN);
    Taster_aussen[3] = !digitalRead(TASTER_3_PIN);
    Taster_aussen[4] = !digitalRead(TASTER_4_PIN);
    Taster_aussen[5] = !digitalRead(TASTER_5_PIN);
    Taster_aussen[6] = !digitalRead(TASTER_6_PIN);
    customKey = customKeypad.getKey();

    if(customKey == '1')
    {
      Taster_innen[1] = 1;
    }
    else 
    {
      Taster_innen[1] = 0;
    }
    if(customKey == '2')
    {
      Taster_innen[2] = 1;
    }
    else 
    {
      Taster_innen[2] = 0;
    }
    if(customKey == '3')
    {
      Taster_innen[3] = 1;
    }
    else 
    {
      Taster_innen[3] = 0;
    }
    if(customKey == '4')
    {
      Taster_innen[4] = 1;
    }
    else 
    {
      Taster_innen[4] = 0;
    }
    if(customKey == '5')
    {
      Taster_innen[5] = 1;
    }
    else 
    {
      Taster_innen[5] = 0;
    }
    if(customKey == '6')
    {
      Taster_innen[6] = 1;
    }
    else 
    {
      Taster_innen[6] = 0;
    }
    if(customKey == '*')
    {
      Notrufbetrieb = 1;
    }
    if(customKey == '#')
    {
      Notrufbetrieb = 0;
    }
  
    Positionssensor = digitalRead(POSITION_SENSOR_PIN);
}


void Ausgaenge_setzen (void)
{
  digitalWrite(LED_1, LED[1]);
  digitalWrite(LED_2, LED[2]);
  digitalWrite(LED_3, LED[3]);
  digitalWrite(LED_4, LED[4]);
  digitalWrite(LED_5, LED[5]);
  digitalWrite(LED_6, LED[6]);
}