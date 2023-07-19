// INCLUDES
#include <stdio.h>                                                                  // Needed for printing
#include <math.h>                                                                   // Used for the fabs() function
#include <string.h>
#include <windows.h>                                                                // Needed for sleep command on Windows
#include <conio.h>                                                                  // Needed for terminal commands on Windows
#include <wchar.h>
#include <locale.h>

// CONSTANTS
// Simulation Starting Parameters
const int SIM_TIME_DELAY_MILLI      = 20;                                           // How fast the program runs
const int NODES_X                   = 15;                                           // Manual array size
const int NODES_Y                   = 15;                                           // Manual array size

const int SHOW_STARTING_CONDITIONS  = FALSE;
const int SHOW_RAW_CONDITIONS       = FALSE;
const int SHOW_BLOCK_VISUALS        = FALSE;


// Thermal Parameters
const float ALPHA               = 0.00001f;                                         // The thermal conductivity K (or alpha)
const float DELTA_SPACE         = 0.01f;                                            // Space step "h"
const float D_SPACE_SQR         = DELTA_SPACE * DELTA_SPACE;                        // Used as h^2

// Ambient Temperatures
const float BAR_L_START_TEMP    = 0.0f;                                             // Temp at N=0
const float BAR_AMBIENT_TEMP    = 0.0f;                                             // Temp between N=1 and N=N_end-1
const float BAR_L_END_TEMP      = 0.0f;                                             // Temp at N=N_end

// Points of Temperature
struct point
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
void    printArray          (float field[][NODES_X]);                               // This prints the array
void    printBlockArray     (float field[][NODES_X]);                               // This prints the array
void    printRawArray       (float field[][NODES_X]);
void    showArrayStats      (float sum, float initialEnergy);
float   sumArray            (float field[][NODES_X]);                               // This totals the values of the field, used to calculate the total energy of the system (for error checking)
char    heatVisualsReturn   (float value);                                          // This returns the LUT values
wchar_t heatBlockReturn     (float value);                                          // This returns the LUT values



int main()                                                                          // Script start
{
    // INIT SECTION
    setlocale(LC_ALL, "");
    printf("Hello world!\n");                                                       //  // Sanity test
    int frameCount = 1;                                                             //  // Frame counter init
    char stability = DELTA_TIME < (D_SPACE_SQR/(4*ALPHA));
    if (stability == 0)
    {
        printf("UNSTABLE SIMULATION");
        return 1;
    }


    // INITIAL CONDITIONS ARRAY
    float a_startField[NODES_Y][NODES_X];                                           //  // Initiates the field
    
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
    struct point a_Points[] = 
    {
        {{NODES_Y/3, 3}                     , 100.0f},
        {{NODES_Y/3, (NODES_X - 4)}         , 100.0f},
        {{2*NODES_Y/3, 3}                   , 100.0f},
        {{2*NODES_Y/3, (NODES_X - 4)}       , 100.0f},
        {{(2*NODES_Y/3) + 1, (NODES_X/2)}   , 100.0f}
        

    };                                                                              //  // INSERT POINTS HERE
    
    numberOfPoints  = sizeof(a_Points)/sizeof(a_Points[0]);

    for (int i = 0; i < numberOfPoints; i++)
    {
        int point_x         = a_Points[i].COORDS[0];
        int point_y         = a_Points[i].COORDS[1];
        float temperature   = a_Points[i].TEMPERATURE_KELVIN;
        
        a_startField[point_x][point_y]  = temperature;
        totalEnergy                     += temperature;
    }

    
    // FIELDS
    printArray(a_startField);
    float initialEnergy = sumArray(a_startField);                                   //  // What is the starting energy


    // New Field
    float a_newField[NODES_X][NODES_Y];                                             //  // This handles the frame of timestep k+1
    memcpy(&a_newField, &a_startField, sizeof(int)*NODES_X*NODES_Y);

    // Current Field
    float a_curField[NODES_X][NODES_Y];                                             //  // This handles the frame of timestep k
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
    float time = 0.0f;                                                              //  // Time of sim init
    

    // CLEARS TERMINAL OF OTHER INFO
    system("cls");                                                                  //  // Clears screen


    // LOOP SECTION
    while(time < FINAL_TIME) 
    {                                                                               //  // This is the while loop
        // INFORMATION
        printf("\n\nIteration: %d\tTime: %4.3f\n", frameCount, time);               //  // Makes sure that the loop is working
        if (SHOW_BLOCK_VISUALS)
        {
            printBlockArray(a_newField);
        }
        else if (!SHOW_RAW_CONDITIONS)
        {
            printArray(a_newField);                                                 //  // Prints the array

        }
        else
        {
            printRawArray(a_newField);                                              //  // Prints the array
        }


        // ENERGY SANITY CHECK
        float energy = sumArray(a_newField);                                        //  // Prints the total energy of the system
        showArrayStats(energy, initialEnergy);
        

        // INITIAL CONDITIONS PRINT
        if (SHOW_STARTING_CONDITIONS)
        {
            printf("\n======================\n");
            printf("Starting Conditions:\n");
            printArray(a_startField);                                                   //  // Display of the initial conditions
        }
        

        // COPIES THE ARRAY TO NEW FIELD
        memcpy(&a_curField, &a_newField, sizeof(int)*NODES_X*NODES_Y);              //  // Updates the working field
        

        // BEHIND-THE-SCENE STUFF
        frameCount++;                                                               //  // Increments the counter
        Sleep(SIM_TIME_DELAY_MILLI);                                                //  // Delay between refreshes
        system("cls");                                                              //  // Clears screen
        time += DELTA_TIME;                                                         //  // Increments the time frame


        // CALCULATION LOOP
        for (int i = 0; i < NODES_Y; i++) 
        {                                                                           //  // Iterates through the array
            for (int j = 0; j < NODES_X; j++) 
            {
                // BOUNDARIES LEFT-RIGHT
                if (j == 0) 
                {                                                                   //  // Left wall
                    aveTempLeft     = a_curField[i][j];                             //  // Ghost node = current node
                    aveTempRight    = a_curField[i][j+1];
                }
                else if (j == (NODES_X-1)) 
                {                                                                   //  // Right wall
                    aveTempLeft     = a_curField[i][j-1];
                    aveTempRight    = a_curField[i][j];                             //  // Ghost node = current node
                }
                else 
                {                                                                   //  // Remaining elements
                    aveTempLeft     = a_curField[i][j-1];
                    aveTempRight    = a_curField[i][j+1];
                }


                //BOUNDARIES UP-DOWN
                if (i == 0) 
                {                                                                   //  // Top wall
                    aveTempAbove    = a_curField[i][j];                             //  // Ghost node = current node
                    aveTempBelow    = a_curField[i+1][j];
                }
                else if (i == (NODES_Y-1)) 
                {                                                                   //  // Bottom wall
                    aveTempAbove    = a_curField[i-1][j];
                    aveTempBelow    = a_curField[i][j];                             //  // Ghost node = current node
                }
                else 
                {                                                                   //  // Remaining elements
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
                dTi_dt = 
                      ( aveTempLeft     + aveTempRight 
                    +   aveTempAbove    + aveTempBelow ) 
                    / ( D_SPACE_SQR ) * ALPHA * DELTA_TIME;                         //  // Driver of the temp
                
                coefficient = 1 - (4*DELTA_TIME*ALPHA/D_SPACE_SQR);                 //  // Handles the current value


                // ARRAY UPDATE
                a_newField[i][j] = coefficient * a_curField[i][j] + dTi_dt;         //  // Last value + temperature difference times the time difference
            }
        }
    }

    return 0;                                                                       //  // Required
}


// FUNCTION DEFINITIONS
void printArray(float field[][NODES_X]) 
{                                                                                   // To print array    
    for (int i = 0; i < NODES_Y; i++) 
    {                                                                               //  // For loop for element
        for (int j = 0; j < NODES_X; j++) 
        {
            // NEGATIVE VALUE HANDLING
            if (field[i][j] < 0.0f) field[i][j] = 0.0f;                             //  // Avoids negative numbers due to small floating point errors
            
            // VERTICAL CHECK
            if (NODES_X == 1) 
            {
                printf("[ %c ]\n", heatVisualsReturn(field[i][j]));
                continue;
            }                                                                       //  // Required if Width is 1

            // DISPLAY DEVELOPMENT
            if (j == 0) 
            {
                printf("[ %c, ", heatVisualsReturn(field[i][j]));
            }                                                                       //  // If first element, include the left bracket
            else if (j == (NODES_X - 1)) 
            {
                printf("%c ]\n", heatVisualsReturn(field[i][j]));
            }                                                                       //  // Else if the element is the last, include the right bracket
            else 
            {
                printf("%c, ", heatVisualsReturn(field[i][j]));
            }                                                                       //  // Otherwise do the normal format
        }
    }
}


void printBlockArray(float field[][NODES_X]) 
{                                                                                   // To print array    
    for (int i = 0; i < NODES_Y; i++) 
    {                                                                               //  // For loop for element
        for (int j = 0; j < NODES_X; j++) 
        {
            // NEGATIVE VALUE HANDLING
            if (field[i][j] < 0.0f) field[i][j] = 0.0f;                             //  // Avoids negative numbers due to small floating point errors
            
            // VERTICAL CHECK
            if (NODES_X == 1) 
            {
                wprintf(L"[ %lc ]\n", heatBlockReturn(field[i][j]));
                continue;
            }                                                                       //  // Required if Width is 1

            // DISPLAY DEVELOPMENT
            if (j == 0) 
            {
                wprintf(L"[ %lc", heatBlockReturn(field[i][j]));
            }                                                                       //  // If first element, include the left bracket
            else if (j == (NODES_X - 1)) 
            {
                wprintf(L"%lc ]\n", heatBlockReturn(field[i][j]));
            }                                                                       //  // Else if the element is the last, include the right bracket
            else 
            {
                wprintf(L"%lc", heatBlockReturn(field[i][j]));
            }                                                                       //  // Otherwise do the normal format
        }
    }
}


void printRawArray(float field[][NODES_X]) 
{                                                                                   // To print array    
    for (int i = 0; i < NODES_Y; i++) 
    {                                                                               //  // For loop for element
        for (int j = 0; j < NODES_X; j++) 
        {
            // NEGATIVE VALUE HANDLING
            if (field[i][j] < 0.0f) field[i][j] = 0.0f;                             //  // Avoids negative numbers due to small floating point errors
            
            // VERTICAL CHECK
            if (NODES_X == 1) 
            {
                printf("[ %-6.1f ]\n", field[i][j]);
                continue;
            }                                                                       //  // Same as normal printArray

            // DISPLAY DEVELOPMENT
            if (j == 0) 
            {
                printf("[ %-6.1f, ", field[i][j]);
            }                                                                       //  // If first element, include the left bracket
            else if (j == (NODES_X - 1)) 
            {
                printf("%-6.1f ]\n", field[i][j]);
            }                                                                       //  // Else if the element is the last, include the right bracket
            else 
            {
                printf("%-6.1f, ", field[i][j]);
            }                                                                       //  // Otherwise do the normal format
        }
    }
}


void showArrayStats(float sum, float initialEnergy) 
{
    float deviation = 0.0f;                                                         //  // Inits the percent deviation
    deviation = fabs((sum - initialEnergy) / initialEnergy);                        //  // Percent difference of energy
    
    float averageEnergyDensity = totalEnergy / (NODES_X*NODES_Y);                   //  // This is for when there's a bit of energy
    
    
    // PRINT
    printf("\nEnergy: %-5f, deviation: %%%6.3f\n", round(sum), deviation*100);      //  // Prints the actual energy
    printf("Intended Energy: %5.4f, Average Energy Density: %5.4f",
        initialEnergy, averageEnergyDensity);                                       //  // Print the theoretical energy
    
    if (SHOW_BLOCK_VISUALS)
    {
        wprintf(L" (%lc)\n", heatBlockReturn(averageEnergyDensity));
    }
    else
    {
        printf(" (%c)\n", heatVisualsReturn(averageEnergyDensity));
    }
}


float sumArray(float field[][NODES_X]) 
{                                                                                   // Gets the total value of an array
    // SUM INIT
    float sum = 0.0f;                                                               //  // Inits the sum variable
    // CUMULATIVE COUNT
    for (int i = 0; i < NODES_Y; i++) 
    {
        for (int j = 0; j < NODES_X; j++) sum += field[i][j];
    }                                                                               //  // Replaces each field
    return sum;
}


char heatVisualsReturn(float value) 
{
    float maxValue = (totalEnergy) / numberOfPoints;                                //  // This can be calculated

    float averageEnergyDensity = totalEnergy / (NODES_X*NODES_Y);                   //  // This is for when there's a bit of energy

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
    float maxValue = (totalEnergy) / numberOfPoints;                                //  // This can be calculated

    float averageEnergyDensity = totalEnergy / (NODES_X*NODES_Y);                   //  // This is for when there's a bit of energy

    // CHECK
    if (value >  (maxValue/2))              {return block_visuals[0];}              //  // I'm aware this looks messy
    if (value >  (maxValue/40))             {return block_visuals[1];}              //  // Switch cases don't work for floats
    if (value >  (maxValue/80))             {return block_visuals[2];}              //  // So had to do this
    
    if (value >  (averageEnergyDensity))    {return '#';}                           //  // End State
    
    if (value >= 0.5f)                      {return block_visuals[3];}              //  // Small case
    if (value >= 0.0f)                      {return '.';}                           //  // Negligible case
    else                                    {return 'E';}                           //  // Error case
}