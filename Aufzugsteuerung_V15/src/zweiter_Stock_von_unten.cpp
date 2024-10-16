#include <global.h>

int zweiter_Stock_von_unten_state = 1;       
int get_zweiter_Stock_von_unten_state()
{
    return zweiter_Stock_von_unten_state;
}

#define check_Haltebedingung 1          // Überprüfung, ob Stepper noch passend zum Stockwerk mit seiner Beschleunigung/Bremsung halten kann, ohne zu spät zu stehen
#define warte_auf_Position 2            
#define Tuer_oeffnen 3
#define Tuer_schliessen 4
#define check_Anforderung 5


void func_zweiter_Stock_von_unten(void)
{
    switch(zweiter_Stock_von_unten_state)
    {
        case check_Haltebedingung:
            
            if((Stockwerk_gerufen[2] == 1) || (Notrufbetrieb==1))                       // Fährt in diesen Stock wenn Stockwerk gerufen oder Notrufbetrieb
            {
                zweiter_Stock_von_unten_state = warte_auf_Position;
            }
            
            else if( (Pos_Stockwerk_2 - stepper1.currentPosition()) < max_brems_steps)  // Wenn Halteweg zu kurz wird Aufzug direkt in nächste Etage geschickt
            {
                stepper1.move( Pos_Stockwerk_3 - stepper1.currentPosition() );
                Hauptfunktionen_state = dritter_Stock_von_unten;
                zweiter_Stock_von_unten_state = check_Haltebedingung;                   // Zurücksetzen switch-Anweisung
            }

            break;

        case warte_auf_Position:                                                        // verweilt so lange in Anweisung, bis Position Stock 2 erreicht ist
            if(stepper1.currentPosition() == stepper1.targetPosition())
            {
                zweiter_Stock_von_unten_state = Tuer_oeffnen;
            }
            break;

        case Tuer_oeffnen:
            Timer_Tuer_oeffnen.IN = 1;
            if(Timer_Tuer_oeffnen.TIMER_ALARM == 1)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[2] = 0;     

                if(Notrufbetrieb == 0)                                                  // Türe kann nur geschlossen werden, wenn kein Notrufbetrieb
                {
                    zweiter_Stock_von_unten_state = Tuer_schliessen;  
                }

            }
            else if(Stockwerk_gerufen[2] == 1)                                          // Timer-Tür-öffnen zurückgesetzt, falls Stockwerk erneut gerufen wird (verlängerung Öffnung)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[2] = 0;     
            }

                LED[2] = Blinker;
            break;

        case Tuer_schliessen:
            Timer_Tuer_schliessen.IN = 1;
            if(Timer_Tuer_schliessen.TIMER_ALARM == 1)
            {
                Timer_Tuer_schliessen.IN = 0; 
                zweiter_Stock_von_unten_state = check_Anforderung;            
            }

            else if((Stockwerk_gerufen[2] == 1) || (Notrufbetrieb == 1))                // Ob beim Schließen nochmmal geöffnet wird oder Notrufbetrieb aktiv, ist das selbe. Tür dann öffnen
            {
                Timer_Tuer_schliessen.IN = 0;
                zweiter_Stock_von_unten_state = Tuer_oeffnen;  
            }

                LED[2] = Blinker;
            break;
        
        case check_Anforderung:
            bool hoeheres_Stockwerk_anfahren = 0;

            if((Stockwerk_gerufen[2] == 1) || (Notrufbetrieb == 1))                     // Ob beim check_Anforderung nochmmal geöffnet wird oder Notrufbetrieb aktiv, ist das selbe. Tür dann öffnen
            {
                zweiter_Stock_von_unten_state = Tuer_oeffnen;
            }

            else
            {
                for(int i = 3; i<=6; i++)                                               // höher gelegene Stockwerke anfragen, ob gerufen
                {
                    if(Stockwerk_gerufen[i] == 1)
                    {
                        Hauptfunktionen_state = dritter_Stock_von_unten;                // wenn gerufen, Fahrt ins nächste darübergelegene Stockwerk
                        zweiter_Stock_von_unten_state = check_Haltebedingung;
                        stepper1.move(Pos_Stockwerk_3 - (stepper1.currentPosition()));  // wenn gerufen, Fahrt ins nächste darübergelegene Stockwerk
                        hoeheres_Stockwerk_anfahren = 1;                                // Hochfahrt Priorität
                    }
                }
                if(hoeheres_Stockwerk_anfahren == 0)                                    // wenn Hochfahrpriorität nicht aktiv
                {
                    if(Stockwerk_gerufen[1] == 1)                                       // in niedrigeres Stockwerk fahren, wenn gerufen
                    {
                        Hauptfunktionen_state = erster_Stock_von_oben;
                        zweiter_Stock_von_unten_state = check_Haltebedingung;
                        stepper1.move(Pos_Stockwerk_1 - (stepper1.currentPosition()));
                    }
                }
            }        
            break;


        
    }

}


