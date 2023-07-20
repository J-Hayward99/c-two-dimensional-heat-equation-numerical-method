// INCLUDES
#include <stdio.h>                                                                  // Needed for printing
#include <math.h>                                                                   // Used for the fabs() function
#include <string.h>                                                                 // QoL for Strings
#include <windows.h>                                                                // Needed for sleep command on Windows
#include <conio.h>                                                                  // Needed for terminal commands on Windows
#include <wchar.h>                                                                  // Needed for block visuals
#include <locale.h>                                                                 // Also needed for block blocks visuals


// ENUMS
enum Visuals {
    VISUAL_RAW,
    VISUAL_BLOCKS,
    VISUAL_HEATMAP
};


// CONSTANTS
// Simulation Starting Parameters
const int SIM_TIME_DELAY_MILLI      = 20;                                           // How fast the program runs
const int NODES_X                   = 15;                                           // Manual array size
const int NODES_Y                   = 15;                                           // Manual array size

const int SHOW_STARTING_CONDITIONS  = FALSE;
const int VISUAL_SETTING            = VISUAL_HEATMAP;

// Thermal Parameters
const float ALPHA               = 0.00001f;                                         // The thermal conductivity K (or alpha)
const float DELTA_SPACE         = 0.01f;                                            // Space step "h"
const float D_SPACE_SQR         = DELTA_SPACE * DELTA_SPACE;                        // Used as h^2

// Ambient Temperatures
const float BAR_L_START_TEMP    = 0.0f;                                             // Temp at N=0
const float BAR_AMBIENT_TEMP    = 0.0f;                                             // Temp between N=1 and N=N_end-1
const float BAR_L_END_TEMP      = 0.0f;                                             // Temp at N=N_end

// Points of Temperature
struct Point
{
    const int   COORDS[2];
    const float TEMPERATURE_KELVIN;
};

// Time Limits
const float DELTA_TIME          = 0.040f;                                           // Time step
const float FINAL_TIME          = 600.0f;                                           // Final time



// LOOK UP TABLES
const char      heat_visuals[]      = {'7', '6', '5', '4', '3', '2', '1'};
const wchar_t   block_visuals[]     = {
    L'\u2588', 
    L'\u2593', 
    L'\u2592', 
    L'\u2591'
};



// GLOBALS
float   totalEnergy     = 0.0f;
int     numberOfPoints  = 0;


// FUNCTION DECLARATION
// Print Arrays
void    printArray          (float* field);
void    printBlockArray     (float* a_field);
void    printRawArray       (float* a_field);

// Array Helpers
void    showArrayStats      (float sum, float initialEnergy);
float   sumArray            (float* a_field);

// LUT Returns
char    heatVisualsReturn   (float value);
wchar_t heatBlockReturn     (float value);



int main()
{
    // INIT SECTION
    setlocale(LC_ALL, "");
    int frameCount = 1;
    
    // CHECKS
    char stability = DELTA_TIME < (D_SPACE_SQR/(4*ALPHA));
    if (stability == 0) {printf("UNSTABLE SIMULATION"); return 1;}


    // INITIAL CONDITIONS ARRAY
    float a_startField[NODES_Y][NODES_X];                                           //  // Base field
    
    for (int i = 0; i < NODES_Y; i++) 
    {
        for (int j = 0; j < NODES_X; j++) 
        {
                if (NODES_X == 1) 
                {
                    a_startField[i][j]  = BAR_L_START_TEMP;
                    continue; 
                }

                if (NODES_Y == 1) 
                {
                    a_startField[i][j]  = BAR_L_START_TEMP;
                    continue;
                }


            if (j == 0) 
            {
                a_startField[i][j]  = BAR_L_START_TEMP;                     
            }
            else if (j == (NODES_X - 1)) 
            {
                a_startField[i][j]  = BAR_L_END_TEMP;
            }  
            else 
            {
                a_startField[i][j]  = BAR_AMBIENT_TEMP;
            }

        }
    }
    
    // POINTS AND STATS
    // Add Points
    struct Point a_Points[] = 
    {
        {{NODES_Y/3             , 3}                , 100.0f},
        {{NODES_Y/3             , (NODES_X - 4)}    , 100.0f},
        {{2*NODES_Y/3           , 3}                , 100.0f},
        {{2*NODES_Y/3           , (NODES_X - 4)}    , 100.0f},
        {{(2*NODES_Y/3) + 1     , (NODES_X/2)}      , 100.0f}
    };                                                                              //  // INSERT POINTS HERE
    
    // Insert Points
    numberOfPoints  = sizeof(a_Points)/sizeof(a_Points[0]);
    
    for (int i = 0; i < numberOfPoints; i++)
    {
        int point_x                     = a_Points[i].COORDS[0];
        int point_y                     = a_Points[i].COORDS[1];
        float temperature               = a_Points[i].TEMPERATURE_KELVIN;
        
        a_startField[point_x][point_y]  = temperature;
        totalEnergy                     += temperature;
    }

    
    // FIELDS
    printArray(&a_startField[0][0]);
    float initialEnergy = sumArray(&a_startField[0][0]);


    // New Field
    float a_newField[NODES_X][NODES_Y];                                             //  // Frame of timestep k+1
    memcpy(&a_newField, &a_startField, sizeof(int)*NODES_X*NODES_Y);

    // Current Field
    float a_curField[NODES_X][NODES_Y];                                             //  // Frame of timestep k
    memcpy(&a_curField, &a_startField, sizeof(int)*NODES_X*NODES_Y);
    
    
    // DIFFERENTIATION VARIABLES
    float dTi_dt;                                                                   //  // Temperature driver
    float coefficient;                                                              //  // Coefficient for the current value


    // TEMPERATURE AVERAGES
    float aveTempLeft;                                                              //  // Average between the current node and the one to the left
    float aveTempRight;                                                             //  // Average between the current node and the one to the right
    float aveTempAbove;                                                             //  // Average between the current node and the one to the above
    float aveTempBelow;                                                             //  // Average between the current node and the one to the below
    

    // TIME COUNTER
    float time = 0.0f;
    

    // CLEARS TERMINAL OF OTHER INFO
    system("cls");


    // LOOP SECTION
    while(time < FINAL_TIME) 
    {
        // INFORMATION
        printf("\n\nIteration: %d\tTime: %4.3f\n", frameCount, time);
        switch (VISUAL_SETTING)
        {
            case VISUAL_HEATMAP:
                printArray(&a_newField[0][0]);
                break;
            
            case VISUAL_BLOCKS:
                printArray(&a_newField[0][0]);
                break;
            
            default:
                printRawArray(&a_newField[0][0]);
        }


        // ENERGY SANITY CHECK
        float energy = sumArray(&a_newField[0][0]);                                 //  // Total energy of the system
        showArrayStats(energy, initialEnergy);
        

        // INITIAL CONDITIONS PRINT
        if (SHOW_STARTING_CONDITIONS)
        {
            printf("\n======================\n");
            printf("Starting Conditions:\n");
            printArray(&a_startField[0][0]);
        }
        

        // COPIES THE ARRAY TO NEW FIELD
        memcpy(&a_curField, &a_newField, sizeof(int)*NODES_X*NODES_Y);              //  // Updates the working field
        

        // BEHIND-THE-SCENE STUFF
        frameCount++;
        Sleep(SIM_TIME_DELAY_MILLI);
        system("cls");
        time += DELTA_TIME;


        // CALCULATION LOOP
        for (int i = 0; i < NODES_Y; i++) 
        {
            for (int j = 0; j < NODES_X; j++) 
            {
                // BOUNDARIES LEFT-RIGHT
                // Left Wall
                if (j == 0) 
                {
                    aveTempLeft     = a_curField[i][j];                             //  // Ghost node = current node
                    aveTempRight    = a_curField[i][j+1];
                }

                // Right Wall
                else if (j == (NODES_X-1)) 
                {
                    aveTempLeft     = a_curField[i][j-1];
                    aveTempRight    = a_curField[i][j];                             //  // Ghost node = current node
                }

                // Remaining Elements (Middle)
                else
                {
                    aveTempLeft     = a_curField[i][j-1];
                    aveTempRight    = a_curField[i][j+1];
                }


                //BOUNDARIES UP-DOWN
                // Top Wall
                if (i == 0) 
                {
                    aveTempAbove    = a_curField[i][j];                             //  // Ghost node = current node
                    aveTempBelow    = a_curField[i+1][j];
                }

                // Bottom Wall
                else if (i == (NODES_Y-1)) 
                {
                    aveTempAbove    = a_curField[i-1][j];
                    aveTempBelow    = a_curField[i][j];                             //  // Ghost node = current node
                }

                // Remaining Elements (Middle)
                else 
                {
                    aveTempAbove    = a_curField[i-1][j];
                    aveTempBelow    = a_curField[i+1][j];
                }


                // LIMIT CONDITIONS
                if (NODES_X == 1) 
                {
                    aveTempLeft     = 0;
                    aveTempRight    = 0;  
                }
                if (NODES_Y == 1) 
                {
                    aveTempAbove    = 0;
                    aveTempBelow    = 0;  
                }


                // CALCULATIONS
                // Temperature Driver
                dTi_dt = 
                      ( aveTempLeft     + aveTempRight 
                    +   aveTempAbove    + aveTempBelow ) 
                    / ( D_SPACE_SQR ) * ALPHA * DELTA_TIME;
                
                // Coefficient
                coefficient = 1 - (4*DELTA_TIME*ALPHA/D_SPACE_SQR);


                // ARRAY UPDATE
                a_newField[i][j] = coefficient * a_curField[i][j] + dTi_dt;         //  // Last value + (temperature difference * the time difference)
            }
        }
    }

    return 0;
}


// FUNCTION DEFINITIONS
void printArray(float* a_field) 
{
    for (int i = 0; i < NODES_Y; i++) 
    {
        for (int j = 0; j < NODES_X; j++) 
        {
            float value = *(a_field + i * NODES_X + j);
            
            // NEGATIVE VALUE HANDLING
            if (value < 0.0f) value = 0.0f;                                         //  // Avoids negative numbers due to small floating point errors
            
            // VERTICAL CHECK
            if (NODES_X == 1) 
            {
                printf("[ %c ]\n", heatVisualsReturn(value));
                continue;
            }

            // DISPLAY DEVELOPMENT
            if (j == 0) 
            {
                printf("[ %c, ", heatVisualsReturn(value));
            }
            else if (j == (NODES_X - 1)) 
            {
                printf("%c ]\n", heatVisualsReturn(value));
            }
            else 
            {
                printf("%c, ", heatVisualsReturn(value));
            }
        }
    }
}


void printBlockArray(float* a_field) 
{
    for (int i = 0; i < NODES_Y; i++) 
    {
        for (int j = 0; j < NODES_X; j++) 
        {
            float value = *(a_field + i * NODES_X + j);

            // NEGATIVE VALUE HANDLING
            if (value < 0.0f) value = 0.0f;

            // VERTICAL CHECK
            if (NODES_X == 1) 
            {
                wprintf(L"[ %lc ]\n", heatBlockReturn(value));
                continue;
            }

            // DISPLAY DEVELOPMENT
            if (j == 0) 
            {
                wprintf(L"[ %lc", heatBlockReturn(value));
            }
            else if (j == (NODES_X - 1)) 
            {
                wprintf(L"%lc ]\n", heatBlockReturn(value));
            }
            else 
            {
                wprintf(L"%lc", heatBlockReturn(value));
            }
        }
    }
}


void printRawArray(float* a_field) 
{
    for (int i = 0; i < NODES_Y; i++) 
    {
        for (int j = 0; j < NODES_X; j++) 
        {
            float value = *(a_field + i * NODES_X + j);

            // NEGATIVE VALUE HANDLING
            if (value < 0.0f) value = 0.0f;
            
            // VERTICAL CHECK
            if (NODES_X == 1) 
            {
                printf("[ %-6.1f ]\n", value);
                continue;
            }

            // DISPLAY DEVELOPMENT
            if (j == 0) 
            {
                printf("[ %-6.1f, ", value);
            }
            else if (j == (NODES_X - 1)) 
            {
                printf("%-6.1f ]\n", value);
            }
            else 
            {
                printf("%-6.1f, ", value);
            }
        }
    }
}


void showArrayStats(float sum, float initialEnergy) 
{
    float deviation             = fabs((sum - initialEnergy) / initialEnergy);      //  // Percent difference of energy
    float averageEnergyDensity  = totalEnergy / (NODES_X*NODES_Y);                  //  // This is for when there's a bit of energy
    
    
    // PRINT
    // Main
    printf("\nEnergy: %-5f, deviation: %%%6.3f\n", round(sum), deviation*100);      //  // Prints the actual energy
    printf(
        "Intended Energy: %5.4f, Average Energy Density: %5.4f",
        initialEnergy, averageEnergyDensity
    );                                                                              //  // Print the theoretical energy
    
    // Respective Symbol
    if (VISUAL_SETTING == VISUAL_BLOCKS)
    {
        wprintf(L" (%lc)\n", heatBlockReturn(averageEnergyDensity));
    }
    else
    {
        printf(" (%c)\n", heatVisualsReturn(averageEnergyDensity));
    }

}


float sumArray(float* a_field) 
{
    float sum = 0.0f;


    // CUMULATIVE COUNT
    for (int i = 0; i < NODES_Y; i++) 
    {
        for (int j = 0; j < NODES_X; j++) 
        {
            float value = *(a_field + i * NODES_X + j);
            sum += value;
        }
    }
    
    return sum;
}


char heatVisualsReturn(float value) 
{
    float maxValue              = (totalEnergy) / numberOfPoints;                   //  // This can be calculated
    float averageEnergyDensity  = totalEnergy / (NODES_X*NODES_Y);                  //  // This is for when there's a bit of energy

    // CHECK
    if (value >  (maxValue/2))              {return heat_visuals[0];}               //  // I'm aware this looks messy
    if (value >  (maxValue/20))             {return heat_visuals[1];}               //  // Switch cases don't work for floats
    if (value >  (maxValue/40))             {return heat_visuals[2];}               //  // So had to do this
    if (value >  (maxValue/60))             {return heat_visuals[3];}
    if (value >  (maxValue/80))             {return heat_visuals[4];}
    if (value >  (maxValue/90))             {return heat_visuals[5];}
    if (value >  (maxValue/95))             {return heat_visuals[6];}
    
    if (value >  (averageEnergyDensity))    {return '#';}                           //  // End State
    
    if (value >= 1.0f)                      {return '0';}
    if (value >= 0.5f)                      {return 'x';}                           //  // Small case
    if (value >= 0.0f)                      {return '.';}                           //  // Negligible case
    else                                    {return 'E';}                           //  // Error case
}

wchar_t heatBlockReturn(float value) 
{
    float maxValue              = (totalEnergy) / numberOfPoints;                   //  // This can be calculated
    float averageEnergyDensity  = totalEnergy / (NODES_X*NODES_Y);                  //  // This is for when there's a bit of energy

    // CHECK
    if (value >  (maxValue/2))              {return block_visuals[0];}              //  // I'm aware this looks messy
    if (value >  (maxValue/40))             {return block_visuals[1];}              //  // Switch cases don't work for floats
    if (value >  (maxValue/80))             {return block_visuals[2];}              //  // So had to do this
    
    if (value >  (averageEnergyDensity))    {return '#';}                           //  // End State
    
    if (value >= 0.5f)                      {return block_visuals[3];}              //  // Small case
    if (value >= 0.0f)                      {return '.';}                           //  // Negligible case
    else                                    {return 'E';}                           //  // Error case
}