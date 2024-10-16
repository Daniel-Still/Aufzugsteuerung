#include <global.h>

int Referenz_erster_Stock_state = 1; 
int get_Referenz_erster_Stock_state()
{
    return Referenz_erster_Stock_state;
}

#define check_Anforderung 1         // Überprüft, ob weitere Rufe vom Aufzug getätigt wurden
#define Tuer_oeffnen 2              // Tür öffnen (siehe LCD)
#define Tuer_schliessen 3           // Tür schließen (siehe LCD)


void func_Referenz_erster_Stock(void)
{
    
    switch(Referenz_erster_Stock_state)
    {
        case check_Anforderung:
            if((Stockwerk_gerufen[1] == 1) || (Notrufbetrieb == 1))         // ob Stockwerk 1 gerufen oder Notrufbetrieb - in beiden Fällen Türe öffnen
            {
                Referenz_erster_Stock_state = Tuer_oeffnen;
            }
            else                                                            // andernfalls wird überprüft, ob anderes Stockwerk geruufen wurde
            {
                for(int i = 2; i<=6; i++)
                {
                    if(Stockwerk_gerufen[i] == 1)                           // falls anderes Stockwerk gerufen wurde, soll in den zweiten Stock (von unten) gefahren werden
                    {
                        Hauptfunktionen_state = zweiter_Stock_von_unten;
                        Referenz_erster_Stock_state = check_Anforderung;                // switch-Anweisung wird auf Ausgangspunkt zurückgestellt
                        stepper1.move(Pos_Stockwerk_2 - (stepper1.currentPosition()));  // Bewegungsbefehl zu 2. Stock für stepper
                    }
                }
            }

            break;
        
        case Tuer_oeffnen:
            Timer_Tuer_oeffnen.IN = 1;
            if(Timer_Tuer_oeffnen.TIMER_ALARM == 1)                     // Timer für 2.5s Tür öffnen
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[1] = 0;     
                if(Notrufbetrieb == 0)
                {
                    Referenz_erster_Stock_state = Tuer_schliessen;      // Türe kann nur geschlossen werden, wenn kein Notrufbetrieb
                }
          
            }
            else if(Stockwerk_gerufen[1] == 1)                          // Tür öffnen kann durch  erneutes Rufen des Stocks im selbigen Stock verlängert werden
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[1] = 0;     
            
            }

                LED[1] = Blinker;                                           // Blinkfunktion für LED 1 Türen
            
            break;
        
        case Tuer_schliessen:                                               
            Timer_Tuer_schliessen.IN = 1;
            if(Timer_Tuer_schliessen.TIMER_ALARM == 1)                      // Timer für 2.5s Tür schließen
            {
                Timer_Tuer_schliessen.IN = 0;    
                Referenz_erster_Stock_state = check_Anforderung;            
            }
            else if((Stockwerk_gerufen[1] == 1) || (Notrufbetrieb == 1))    // Tür öffnen kann durch Notruf oder erneutes Rufen des Stocks im selbigen Stock verlängert werden
            {
                Timer_Tuer_schliessen.IN = 0;
                Referenz_erster_Stock_state = Tuer_oeffnen;
            
            }

                LED[1] = Blinker;                                           // Blinkfunktion für LED 1 Türen
            
            break;

    }
}