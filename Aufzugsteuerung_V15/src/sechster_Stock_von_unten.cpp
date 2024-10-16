#include <global.h>

int sechster_Stock_von_unten_state = 1;
int get_sechster_Stock_von_unten_state()
{
    return sechster_Stock_von_unten_state;
}

#define warte_auf_Position 1
#define Tuer_oeffnen 2
#define Tuer_schliessen 3
#define check_Anforderung 4


void func_sechster_Stock_von_unten(void)
{
    switch(sechster_Stock_von_unten_state)
    {

        case warte_auf_Position:
            if(stepper1.currentPosition() == stepper1.targetPosition())
            {
                sechster_Stock_von_unten_state = Tuer_oeffnen;
            }
            break;

        case Tuer_oeffnen:
            Timer_Tuer_oeffnen.IN = 1;
            if(Timer_Tuer_oeffnen.TIMER_ALARM == 1)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[6] = 0;     
                if(Notrufbetrieb == 0)
                {
                    sechster_Stock_von_unten_state = Tuer_schliessen;                  
                }
          
            }
            else if(Stockwerk_gerufen[6] == 1)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[6] = 0;     
            
            }

                LED[6] = Blinker;
            break;

        case Tuer_schliessen:
            Timer_Tuer_schliessen.IN = 1;
            if(Timer_Tuer_schliessen.TIMER_ALARM == 1)
            {
                Timer_Tuer_schliessen.IN = 0; 
                sechster_Stock_von_unten_state = check_Anforderung;            
            }
            else if((Stockwerk_gerufen[6] == 1) || (Notrufbetrieb == 1))
            {
                Timer_Tuer_schliessen.IN = 0;
                sechster_Stock_von_unten_state = Tuer_oeffnen;  
            
            }

                LED[6] = Blinker;
            break;
        
        case check_Anforderung:


            if((Stockwerk_gerufen[6] == 1) || (Notrufbetrieb == 1))
            {
                sechster_Stock_von_unten_state = Tuer_oeffnen;
            }
            else
            {
                for(int i = 5; i>=1; i--)
                {
                    if(Stockwerk_gerufen[i] == 1)
                    {
                        Hauptfunktionen_state = fuenfter_Stock_von_oben;
                        sechster_Stock_von_unten_state = warte_auf_Position;
                        stepper1.move(Pos_Stockwerk_5 - (stepper1.currentPosition()));

                    }
                }

            }        
            break;

        
    }
}

