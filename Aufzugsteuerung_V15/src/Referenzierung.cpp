#include <global.h>

int Referenzierung_state = 1;        


void func_Referenzierung(void)
{
    #define suchesensor 1           
    #define sensorgefunden 2
    #define warte_auf_Position 3
    switch(Referenzierung_state)                      
    {
        case suchesensor:                                       // suchesensor nur, wenn Sensorsignal nicht v.h. --> Aufzug muss sich 
            stepper1.setMaxSpeed(1000);                         // in einer höheren Position befinden. 
            if(Positionssensor == 0)
            {
                stepper1.move(-2000);                           // Stepper muss abwärts fahren, um Sensorsignal/Referenzpunkt zu erreichen
            }
            else
            {
                Referenzierung_state = sensorgefunden;          // Wenn Positionssensor == 1 --> Sensor gefunden
            }
            break; 
        
        case sensorgefunden:                                    // Wenn Sensor gefunden wurde, langsames auffahren, da nur beim Verlust des Sensirsignals 
            stepper1.setMaxSpeed(100);                          // ein genauer Referenzpunkt gefunden werden kann, da beim Hinfahren ungenauigkeiten entstehen können
            if(Positionssensor == 1)
            {
                stepper1.move(2000);                            // Stepper fährt mit langsamer Geschwindigkeit nach oben, bis Signal verloren wurde
            }                                                   
            else
            {
                Referenzierung_state = warte_auf_Position;      
                stepper1.setCurrentPosition(0);                 // Verluststelle des Signals ist Referenzpunkt 0
                stepper1.setMaxSpeed(1000);
                stepper1.move(991);                             // Bewegung zum Stepper-Punkt 991, wo sich erstes Stockwerk befindet
            }
            break;
        
        case warte_auf_Position:
            if(stepper1.currentPosition() == stepper1.targetPosition())     // Abfrage mit speziellen Befehlen der stepper-Library (wann erreicht Pos. Stepper die gesetzte Zielposition 991)
            {
                Hauptfunktionen_state = Referenz_erster_Stock;              // Geht über main.cpp in Hauptfunktionen_state in Referenz_erster_Stock
            }
            break;


    }
}

