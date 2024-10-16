#include <global.h>

int erster_Stock_von_oben_state = 1;
int get_erster_Stock_von_oben_state()
{
    return erster_Stock_von_oben_state;
}
// check_Haltebedingung nicht benötigt, da hier nicht zu knapp gehalten werden KANN, da kein Stockwerk darunter ist
#define warte_auf_Position 1
#define Tuer_oeffnen 2
#define Tuer_schliessen 3
#define check_Anforderung 4


void func_erster_Stock_von_oben(void)
{
    switch(erster_Stock_von_oben_state)
    {

        case warte_auf_Position:
            if(stepper1.currentPosition() == stepper1.targetPosition())
            {
                erster_Stock_von_oben_state = Tuer_oeffnen;
            }
            break;

        case Tuer_oeffnen:
            Timer_Tuer_oeffnen.IN = 1;
            if(Timer_Tuer_oeffnen.TIMER_ALARM == 1)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[1] = 0;     
                if(Notrufbetrieb == 0)
                {
                    erster_Stock_von_oben_state = Tuer_schliessen; 
                }
           
            }
            else if(Stockwerk_gerufen[1] == 1)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[1] = 0;     
            
            }

                LED[1] = Blinker;
            break;

        case Tuer_schliessen:
            Timer_Tuer_schliessen.IN = 1;
            if(Timer_Tuer_schliessen.TIMER_ALARM == 1)
            {
                Timer_Tuer_schliessen.IN = 0; 
                erster_Stock_von_oben_state = check_Anforderung;            
            }
            else if((Stockwerk_gerufen[1] == 1) || (Notrufbetrieb == 1))
            {
                Timer_Tuer_schliessen.IN = 0;
                erster_Stock_von_oben_state = Tuer_oeffnen;  
            
            }

                LED[1] = Blinker;
            break;
        
        case check_Anforderung:


            if((Stockwerk_gerufen[1] == 1) || (Notrufbetrieb == 1))
            {
                erster_Stock_von_oben_state = Tuer_oeffnen;
            }
            else
            {
                for(int i = 2; i<=6; i++)
                {
                    if(Stockwerk_gerufen[i] == 1)
                    {
                        Hauptfunktionen_state = zweiter_Stock_von_unten;
                        erster_Stock_von_oben_state = warte_auf_Position;
                        stepper1.move(Pos_Stockwerk_2 - (stepper1.currentPosition()));

                    }
                }

            }        
            break;

        
    }
}

