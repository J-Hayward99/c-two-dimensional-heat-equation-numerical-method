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
const int SIM_TIME_DELAY_MILLI      = 1;                                            // How fast the program runs
const int NODES_X                   = 20;                                           // Manual array size
const int NODES_Y                   = 20;                                           // Manual array size

const int SHOW_STARTING_CONDITIONS  = FALSE;
const int VISUAL_SETTING            = VISUAL_HEATMAP;

// Thermal Parameters
const float ALPHA               = 0.00001f;                                         // The thermal conductivity K (or alpha)
const float DELTA_SPACE         = 0.01f;                                            // Space step "h"
const float D_SPACE_SQR         = DELTA_SPACE * DELTA_SPACE;                        // Used as h^2

// Ambient Temperatures
const float BAR_L_START_TEMP    = 100.0f;                                           // Temp at N=0
const float BAR_AMBIENT_TEMP    = 40.0f;                                            // Temp between N=1 and N=N_end-1
const float BAR_L_END_TEMP      = 100.0f;                                           // Temp at N=N_end

// Points of Temperature
struct Point
{
    const int   COORDS[2];
    const float TEMPERATURE_KELVIN;
};

// Time Limits
const float DELTA_TIME          = 0.040f;                                           // Time step
const float FINAL_TIME          = 600.0f;                                           // Final time


// DERIVED CONSTANTS
const int TOP_WALL              = 0;
const int BOTTOM_WALL           = NODES_Y - 1;
const int LEFT_WALL             = 0;
const int RIGHT_WALL            = NODES_X - 1;


// LOOK UP TABLES
const float     percentDistribution[]   = {
    0.90f, 0.75f, 0.50f, 0.25f, 0.20f, 
    0.15f, 0.10f, 0.05f, 0.02f, 0.01f
};

const char      heatmapVisuals[]        = {
    '7', '6', '5', '4', '3', '2', '1', '0', '#', 'x'
};

const wchar_t   blockVisuals[]          = 
{
    L'\u2588', L'\u2593', 
    L'\u2592', L'\u2591'
};



// GLOBALS
float   totalEnergy     = 0.0f;
float   smallestEnergy  = 9999999999.0f;
float   biggestEnergy   = 0.0f;
int     nonZeroNodes    = 0;


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
    
    for (int index_y = 0; index_y < NODES_Y; index_y++) 
    {
        for (int index_x = 0; index_x < NODES_X; index_x++) 
        {
            if (index_x == LEFT_WALL) 
                a_startField[index_y][LEFT_WALL]    = BAR_L_START_TEMP;                     
            
            else if (index_x == RIGHT_WALL) 
                a_startField[index_y][RIGHT_WALL]   = BAR_L_END_TEMP;
            
            else 
                a_startField[index_y][index_x]      = BAR_AMBIENT_TEMP;
            
        }
    }
    totalEnergy += BAR_L_START_TEMP + BAR_AMBIENT_TEMP + BAR_L_END_TEMP;
    
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
    int numberOfPoints  = sizeof(a_Points)/sizeof(a_Points[0]);
    
    for (int i = 0; i < numberOfPoints; i++)
    {
        int point_x                     = a_Points[i].COORDS[0];
        int point_y                     = a_Points[i].COORDS[1];
        float temperature               = a_Points[i].TEMPERATURE_KELVIN;
        
        a_startField[point_y][point_x]  = temperature;
        totalEnergy                     += temperature;
    }

    // Count Non-Zero Nodes & Find Temperature Range
    for (int index_y = 0; index_y < NODES_Y; index_y++) 
    {
        for (int index_x = 0; index_x < NODES_X; index_x++)
        {
            // Count Non-Zero Nodes
            float currValue = a_startField[index_y][index_x];
            if (currValue != 0.0f) nonZeroNodes++;
            
            // Temperature Check
            if (currValue < smallestEnergy) smallestEnergy  = currValue;
            if (currValue > biggestEnergy)  biggestEnergy   = currValue;
        }
    } 

    
    // FIELDS
    printArray(&a_startField[0][0]);
    float initialEnergy = sumArray(&a_startField[0][0]);


    // New Field
    float a_newField[NODES_Y][NODES_X];                                             //  // Frame of timestep k+1
    memcpy(&a_newField, &a_startField, sizeof(int)*NODES_X*NODES_Y);

    // Current Field
    float a_curField[NODES_Y][NODES_X];                                             //  // Frame of timestep k
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
        for (int index_y = 0; index_y < NODES_Y; index_y++) 
        {
            for (int index_x = 0; index_x < NODES_X; index_x++) 
            {
                float currElementValue = a_curField[index_y][index_x];
                
                aveTempLeft     = a_curField[index_y][index_x - 1];
                aveTempRight    = a_curField[index_y][index_x + 1];
                
                aveTempAbove    = a_curField[index_y - 1][index_x];
                aveTempBelow    = a_curField[index_y + 1][index_x];
                
                // BOUNDARIES (Ghost node = current node)
                if (index_x == LEFT_WALL)   aveTempLeft     = currElementValue;
                if (index_x == RIGHT_WALL)  aveTempRight    = currElementValue;
                
                if (index_y == TOP_WALL)    aveTempAbove    = currElementValue;
                if (index_y == BOTTOM_WALL) aveTempBelow    = currElementValue;

                // CALCULATIONS
                // Temperature Driver
                dTi_dt = 
                      ( aveTempLeft     + aveTempRight 
                    +   aveTempAbove    + aveTempBelow ) 
                    / ( D_SPACE_SQR ) * ALPHA * DELTA_TIME;
                
                // Coefficient
                coefficient = 1 - (4 * DELTA_TIME*ALPHA / D_SPACE_SQR);


                // ARRAY UPDATE
                float temp_diff = coefficient * a_curField[index_y][index_x];
                a_newField[index_y][index_x] = temp_diff + dTi_dt;                  //  // Last value + (temperature difference * the time difference)
            }
        }
    }

    return 0;
}


// FUNCTION DEFINITIONS
void printArray(float* a_field) 
{
    for (int index_y = 0; index_y < NODES_Y; index_y++) 
    {
        for (int index_x = 0; index_x < NODES_X; index_x++) 
        {
            float value = *(a_field + index_y * NODES_X + index_x);
            
            // NEGATIVE VALUE HANDLING
            if (value < 0.0f) value = 0.0f;                                         //  // Avoids negative numbers due to small floating point errors
            
            // VERTICAL CHECK
            if (NODES_X == 1) 
            {
                printf("[ %c ]\n", heatVisualsReturn(value));
                continue;
            }

            // DISPLAY DEVELOPMENT
            if (index_x == 0) 
                printf("[ %c, ", heatVisualsReturn(value));
            
            else if (index_x == (NODES_X - 1)) 
                printf("%c ]\n", heatVisualsReturn(value));
            
            else 
                printf("%c, ", heatVisualsReturn(value));
        }
    }
}


void printBlockArray(float* a_field) 
{
    for (int index_y = 0; index_y < NODES_Y; index_y++) 
    {
        for (int index_x = 0; index_x < NODES_X; index_x++) 
        {
            float value = *(a_field + index_y * NODES_X + index_x);

            // NEGATIVE VALUE HANDLING
            if (value < 0.0f) value = 0.0f;

            // VERTICAL CHECK
            if (NODES_X == 1) 
            {
                wprintf(L"[ %lc ]\n", heatBlockReturn(value));
                continue;
            }

            // DISPLAY DEVELOPMENT
            if (index_x == 0) 
                wprintf(L"[ %lc", heatBlockReturn(value));
            
            else if (index_x == (NODES_X - 1)) 
                wprintf(L"%lc ]\n", heatBlockReturn(value));
            
            else 
                wprintf(L"%lc", heatBlockReturn(value));
        }
    }
}


void printRawArray(float* a_field) 
{
    for (int index_y = 0; index_y < NODES_Y; index_y++) 
    {
        for (int index_x = 0; index_x < NODES_X; index_x++) 
        {
            float value = *(a_field + index_y * NODES_X + index_x);

            // NEGATIVE VALUE HANDLING
            if (value < 0.0f) value = 0.0f;
            
            // VERTICAL CHECK
            if (NODES_X == 1) 
            {
                printf("[ %-6.1f ]\n", value);
                continue;
            }

            // DISPLAY DEVELOPMENT
            if (index_x == 0) 
                printf("[ %-6.1f, ", value);
            
            else if (index_x == (NODES_X - 1)) 
                printf("%-6.1f ]\n", value);
            
            else 
                printf("%-6.1f, ", value);
        }
    }
}


void showArrayStats(float sum, float initialEnergy) 
{
    float deviation             = fabs((sum - initialEnergy) / initialEnergy);      //  // Percent difference of energy
    float averageEnergyDensity  = totalEnergy / (NODES_X*NODES_Y);                  //  // This is for when there's a bit of energy
    
    
    // PRINT
    // Main
    printf("\nEnergy: %-5f, deviation: %6.3f%%\n", round(sum), deviation*100);      //  // Prints the actual energy
    printf(
        "Intended Energy: %5.4f, Average Energy Density: %5.4f",
        initialEnergy, averageEnergyDensity
    );                                                                              //  // Print the theoretical energy
    
    // Respective Symbol
    switch (VISUAL_SETTING)
    {
        case VISUAL_BLOCKS:
            wprintf(L" (%lc)", heatBlockReturn(averageEnergyDensity));
            break;
        case VISUAL_HEATMAP:
            printf(" (%c)", heatVisualsReturn(averageEnergyDensity));
            break;
        default:
            printf(" (%6.1f)", averageEnergyDensity);
    }
    // Heatmap Stats
    printf("\nMax Energy: %5.4f, Min Energy: %5.4f\n",
        biggestEnergy, smallestEnergy
    );
    
    printf("Energy Range: %5.4f, Initially Active Nodes: %d", 
    (biggestEnergy - smallestEnergy), nonZeroNodes
    );
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
    float distributedValue      = (
        (value - smallestEnergy) / 
        (biggestEnergy - smallestEnergy)
    );

    // CHECK
    if (distributedValue > percentDistribution[0])  return heatmapVisuals[0];       //  // I'm aware this looks messy
    if (distributedValue > percentDistribution[1])  return heatmapVisuals[1];       //  // Switch cases don't work for floats
    if (distributedValue > percentDistribution[2])  return heatmapVisuals[2];       //  // So had to do this
    if (distributedValue > percentDistribution[3])  return heatmapVisuals[3];
    if (distributedValue > percentDistribution[4])  return heatmapVisuals[4];
    if (distributedValue > percentDistribution[5])  return heatmapVisuals[5];
    if (distributedValue > percentDistribution[6])  return heatmapVisuals[6];
    
    if (distributedValue > percentDistribution[7])  return heatmapVisuals[7];
    if (distributedValue > percentDistribution[8])  return heatmapVisuals[8];       //  // Small case
    if (distributedValue > percentDistribution[9])  return heatmapVisuals[9];       //  // Small case
    if (value >= 0.0f)                              return '.';                     //  // Negligible case
    else                                            return 'e';                     //  // Error case
}

wchar_t heatBlockReturn(float value) 
{
    float distributedValue      = (
        (value - smallestEnergy) / 
        (biggestEnergy - smallestEnergy)
    );

    // CHECK
    if (distributedValue > percentDistribution[0])  return heatmapVisuals[0];       //  // I'm aware this looks messy
    if (distributedValue > percentDistribution[2])  return heatmapVisuals[1];       //  // Switch cases don't work for floats
    if (distributedValue > percentDistribution[4])  return heatmapVisuals[2];       //  // So had to do this
    if (distributedValue > percentDistribution[6])  return heatmapVisuals[3];

    if (distributedValue > percentDistribution[7])   return '#';
    if (distributedValue > percentDistribution[8])   return 'x';                    //  // Small case
    if (distributedValue > percentDistribution[9])   return '0';                    //  // Small case
    if (value >= 0.0f)                               return '.';                    //  // Negligible case
    else                                             return 'E';                    //  // Error case
}