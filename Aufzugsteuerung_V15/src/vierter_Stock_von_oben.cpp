#include <global.h>

int vierter_Stock_von_oben_state = 1;
int get_vierter_Stock_von_oben_state()
{
    return vierter_Stock_von_oben_state;
}

#define check_Haltebedingung 1
#define warte_auf_Position 2
#define Tuer_oeffnen 3
#define Tuer_schliessen 4
#define check_Anforderung 5


void func_vierter_Stock_von_oben(void)
{
    switch(vierter_Stock_von_oben_state)
    {
        case check_Haltebedingung:
            if((Stockwerk_gerufen[4] == 1) || (Notrufbetrieb == 1))
            {
                vierter_Stock_von_oben_state = warte_auf_Position;
            }
            else if( (stepper1.currentPosition() - Pos_Stockwerk_4  ) < max_brems_steps) 
            {
                stepper1.move(Pos_Stockwerk_3 - stepper1.currentPosition() );
                Hauptfunktionen_state = dritter_Stock_von_oben;
                vierter_Stock_von_oben_state = check_Haltebedingung;
            }

            break;

        case warte_auf_Position:
            if(stepper1.currentPosition() == stepper1.targetPosition())
            {
                vierter_Stock_von_oben_state = Tuer_oeffnen;
            }
            break;

        case Tuer_oeffnen:
            Timer_Tuer_oeffnen.IN = 1;
            if(Timer_Tuer_oeffnen.TIMER_ALARM == 1)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[4] = 0;     
                if(Notrufbetrieb == 0)
                {
                    vierter_Stock_von_oben_state = Tuer_schliessen;                  
                }
          
            }
            else if(Stockwerk_gerufen[4] == 1)
            {
                Timer_Tuer_oeffnen.IN = 0;
                Stockwerk_gerufen[4] = 0;     
            
            }

                LED[4] = Blinker;
            break;

        case Tuer_schliessen:
            Timer_Tuer_schliessen.IN = 1;
            if(Timer_Tuer_schliessen.TIMER_ALARM == 1)
            {
                Timer_Tuer_schliessen.IN = 0; 
                vierter_Stock_von_oben_state = check_Anforderung;            
            }
            else if((Stockwerk_gerufen[4] == 1) || (Notrufbetrieb == 1))
            {
                Timer_Tuer_schliessen.IN = 0;
                vierter_Stock_von_oben_state = Tuer_oeffnen;  
            
            }

                LED[4] = Blinker;
            break;
        
        case check_Anforderung:
            bool niedrigeres_Stockwerk_anfahren = 0;

            if((Stockwerk_gerufen[4] == 1) || (Notrufbetrieb == 1))
            {
                vierter_Stock_von_oben_state = Tuer_oeffnen;
            }
            else
            {
                
                
                for(int i=3; i>=1; i--)
                {                
                    if(Stockwerk_gerufen[i] == 1)
                    {
                        Hauptfunktionen_state = dritter_Stock_von_oben;
                        vierter_Stock_von_oben_state = check_Haltebedingung;
                        stepper1.move(Pos_Stockwerk_3 - (stepper1.currentPosition()));
                        niedrigeres_Stockwerk_anfahren = 1;        // Abfahrt Priorität
                    }
                }
               
                if(niedrigeres_Stockwerk_anfahren == 0)
                {
                    for(int i=5; i<=6; i++)
                    {
                        if(Stockwerk_gerufen[i] == 1)
                        {
                            Hauptfunktionen_state = fuenfter_Stock_von_unten;
                            vierter_Stock_von_oben_state = check_Haltebedingung;
                            stepper1.move(Pos_Stockwerk_5 - (stepper1.currentPosition()));
                        }
                    }
                }
            }        
            break;

        
    }
}


