#include <global.h>

int fuenfter_Stock_von_unten_state = 1;
int get_fuenfter_Stock_von_unten_state()
{
    return fuenfter_Stock_von_unten_state;
}

#define check_Haltebedingung 1
#define warte_auf_Position 2
#define Tuer_oeffnen 3
#define Tuer_schliessen 4
#define check_Anforderung 5


void func_fuenfter_Stock_von_unten(void)
{
    switch(fuenfter_Stock_von_unten_state)
    {
        case check_Haltebedingung:
            if((Stockwerk_gerufen[5] == 1) || (Notrufbetrieb == 1))
            {
                fuenfter_Stock_von_unten_state = warte_auf_Position;
            }
            else if( (Pos_Stockwerk_5 - stepper1.currentPosition()) < max_brems_steps) 
            {
                stepper1.move( Pos_Stockwerk_6 - stepper1.currentPosition() );
                Hauptfunktionen_state = sechster_Stock_von_unten;
                fuenfter_Stock_von_unten_state = check_Haltebedingung;
            }

            break;

        case warte_auf_Position:
            if(stepper1.currentPosition() == stepper1.targetPosition())
            {
                fuenfter_Stock_von_unten_state = Tuer_oeffnen;

            }
            break;

        case Tuer_oeffnen:
            Timer_Tuer_oeffnen.IN = 1;
            if(Timer_Tuer_oeffnen.TIMER_ALARM == 1)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[5] = 0;    
                if(Notrufbetrieb == 0)
                {
                    fuenfter_Stock_von_unten_state = Tuer_schliessen; 
                }            
            }
            else if(Stockwerk_gerufen[5] == 1)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[5] = 0;     
            
            }

                LED[5] = Blinker;
            break;

        case Tuer_schliessen:
            Timer_Tuer_schliessen.IN = 1;
            if(Timer_Tuer_schliessen.TIMER_ALARM == 1)
            {
                Timer_Tuer_schliessen.IN = 0; 
                fuenfter_Stock_von_unten_state = check_Anforderung;            
            }
            else if((Stockwerk_gerufen[5] == 1) || (Notrufbetrieb == 1))
            {
                Timer_Tuer_schliessen.IN = 0;
                fuenfter_Stock_von_unten_state = Tuer_oeffnen;  
            
            }

                LED[5] = Blinker;
            break;
        
        case check_Anforderung:
            bool hoeheres_Stockwerk_anfahren = 0;
            if((Stockwerk_gerufen[5] == 1) || (Notrufbetrieb == 1))
            {
                fuenfter_Stock_von_unten_state = Tuer_oeffnen;
            }
            else
            {
                if(Stockwerk_gerufen[6] == 1)
                {
                    Hauptfunktionen_state = sechster_Stock_von_unten;
                    fuenfter_Stock_von_unten_state = check_Haltebedingung;
                    stepper1.move(Pos_Stockwerk_6 - (stepper1.currentPosition()));
                    hoeheres_Stockwerk_anfahren = 1;        // Hochfahrt Priorität
                }
                
                if(hoeheres_Stockwerk_anfahren == 0)
                {
                    for(int i=4; i>=1; i--)
                    {
                        if(Stockwerk_gerufen[i] == 1)
                        {
                            Hauptfunktionen_state = vierter_Stock_von_oben;
                            fuenfter_Stock_von_unten_state = check_Haltebedingung;
                            stepper1.move(Pos_Stockwerk_4 - (stepper1.currentPosition()));
                        }
                    }
                }
            }        
            break;

        
    }
}


