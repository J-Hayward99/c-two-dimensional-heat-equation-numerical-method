#include <stdio.h>                                                                  // Needed for printing
#include <math.h>                                                                   // Used for the abs() function
#include <windows.h>                                                                // Needed for sleep command on Windows
#include <conio.h>                                                                  // Needed for terminal commands on Windows

// CONSTANTS
const int SIM_TIME_DELAY_MILLI  = 20;
const int NODES_X               = 15;                                               // Manual array size
const int NODES_Y               = 15;                                               // Manual array size

const float ALPHA               = 0.00001f;                                         // The thermal conductivity K (or alpha)
const float DELTA_SPACE         = 0.01f;                                            // Distance between nodes in the width coord
const float D_SPACE_SQR         = DELTA_SPACE * DELTA_SPACE;


const float BAR_L_START_TEMP    = 0.0f;                                             // Temp at N=0
const float BAR_AMBIENT_TEMP    = 0.0f;                                             // Temp between N=1 and N=N_end-1
const float BAR_L_END_TEMP      = 0.0f;                                             // Temp at N=N_end

const float BAR_POINT_ONE       = 100.0f;
const int   BAR_P_ONE_LOC[]     = {NODES_Y/2, 3};
const float BAR_POINT_TWO       = 100.0f;
const int   BAR_P_TWO_LOC[]     = {NODES_Y/2, (NODES_X - 4)};

const float DELTA_TIME          = 0.040f;                                           // Time step
const float FINAL_TIME          = 600.0f;                                           // Final time

// LOOK UP TABLES
const char heat_visuals[]       = {'7', '6', '5', '4', '3', '2', '1'};

// PROTOTYPE FUNCTIONS
void    printArray(float field[][NODES_X]);                                         // This prints the array
void    printRawArray(float field[][NODES_X]);
void    copyArray(float copyField[][NODES_X], float pasteField[][NODES_X]);         // This copies the array from one to another
void    showArrayStats(float sum, float initialEnergy);

float   sumArray(float field[][NODES_X]);                                           // This totals the values of the field, used to calculate the total energy of the system (for error checking)

char    heatVisualsReturn(float value);                                             // This returns the LUT values



int main()                                                                          // Script start
{
    // INIT SECTION
    printf("Hello world!\n");                                                       //  // Sanity test
    int frameCount = 1;                                                             //  // Frame counter init
    char stability = DELTA_TIME < (D_SPACE_SQR/(4*ALPHA));
    if (stability == 0){
        printf("UNSTABLE SIMULATION");
        return 1;
    }


    // INITIAL CONDITIONS ARRAY
    float a_startField[NODES_Y][NODES_X];                                           //  // Initiates the field
    
    for (int i = 0; i < NODES_Y; i++) {
        for (int j = 0; j < NODES_X; j++) {
                if (NODES_X == 1) {
                    a_startField[i][j]  = BAR_L_START_TEMP;
                    continue; 
                }

                if (NODES_Y == 1) {
                    a_startField[i][j]  = BAR_L_START_TEMP;
                    continue;
                }


            if (j == 0) {
                a_startField[i][j]  = BAR_L_START_TEMP;                     
            }
            else if (j == (NODES_X - 1)) {
                a_startField[i][j]  = BAR_L_END_TEMP;
            }  
            else {
                a_startField[i][j]  = BAR_AMBIENT_TEMP;
            }

        }
    }
    
    a_startField[BAR_P_ONE_LOC[0]][BAR_P_ONE_LOC[1]] = BAR_POINT_ONE;
    a_startField[BAR_P_TWO_LOC[0]][BAR_P_TWO_LOC[1]] = BAR_POINT_TWO;
    
    printArray(a_startField);
    float initialEnergy = sumArray(a_startField);

    // NEW FIELD
    float a_newField[NODES_X][NODES_Y];
    copyArray(a_startField, a_newField);
    

    // CURRENT FIELD
    float a_curField[NODES_X][NODES_Y];
    copyArray(a_startField, a_newField);
    
    
    // DIFFERENTIATION VARIABLES
    
    float dTi_dt;
    float coefficient;

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
    while(time < FINAL_TIME) {                                                      //  // This is the while loop
        // INFORMATION
        printf("\n\nIteration: %d\tTime: %4.3f\n", frameCount, time);               //  // Makes sure that the loop is working
        printArray(a_newField);                                                     //  // Prints the array

        // ENERGY SANITY CHECK
        float energy = sumArray(a_newField);                                        //  // Prints the total energy of the system
        showArrayStats(energy, initialEnergy);
        
        // DEBUG
        printf("\n======================\n");
        printf("h2: %f, h: %f\n",
            D_SPACE_SQR, DELTA_SPACE
            );
        // printf("RAW NUMBERS:\n");
        // printRawArray(a_newField);


        // INITIAL CONDITIONS PRINT
        // printf("\n======================\n");
        // printf("Starting Conditions:\n");
        // printArray(a_startField);                                                   //  // Display of the initial conditions, this is to given an idea of the setup
        
        // COPIES THE ARRAY TO NEW FIELD
        copyArray(a_newField, a_curField);                                          //  // Updates the working field
        
        // BEHIND-THE-SCENE STUFF
        frameCount++;                                                               //  // Increments the counter
        Sleep(SIM_TIME_DELAY_MILLI);                                                //  // Delay between refreshes
        system("cls");                                                              //  // Clears screen
        time += DELTA_TIME;                                                         //  // Increments the time frame

        // CALCULATION LOOP
        for (int i = 0; i < NODES_Y; i++) {                                         //  // Iterates through the array
            for (int j = 0; j < NODES_X; j++) {

                // BOUNDARIES LEFT-RIGHT
                if (j == 0) {                                                       //  // If the value is at L = 0 (beginning)
                    aveTempLeft     = a_curField[i][j];                                            //  // No node to the left
                    aveTempRight    = a_curField[i][j+1];
                }
                
                else if (j == (NODES_X-1)) {                                        //  // If the value is at L = L (ending)
                    aveTempLeft     = a_curField[i][j-1];
                    aveTempRight    = a_curField[i][j];                                            //  // No node to the right
                }

                else {                                                              //  // Remaining elements
                    aveTempLeft     = a_curField[i][j-1];
                    aveTempRight    = a_curField[i][j+1];
                }

                //BOUNDARIES UP-DOWN
                if (i == 0) {                                                       //  // If the value is at L = 0 (beginning)
                    aveTempAbove    = a_curField[i][j];                                            //  // No node to the left
                    aveTempBelow    = a_curField[i+1][j];
                }
                
                else if (i == (NODES_Y-1)) {                                        //  // If the value is at L = L (ending)
                    aveTempAbove    = a_curField[i-1][j];
                    aveTempBelow    = a_curField[i][j];                                            //  // No node to the right
                }

                else {                                                              //  // Remaining elements
                    aveTempAbove    = a_curField[i-1][j];
                    aveTempBelow    = a_curField[i+1][j];
                }

                // LIMIT CONDITIONS
                if (NODES_X == 1) {
                    aveTempLeft     = 0;
                    aveTempRight    = 0;  
                }

                if (NODES_Y == 1) {
                    aveTempAbove    = 0;
                    aveTempBelow    = 0;  
                }

                // CALCULATIONS
                dTi_dt = 
                      ( aveTempLeft     + aveTempRight 
                    +   aveTempAbove    + aveTempBelow ) 
                    / ( D_SPACE_SQR ) * ALPHA * DELTA_TIME;                                                //  // Average temp
                
                coefficient = 1 - (4*DELTA_TIME*ALPHA/D_SPACE_SQR);

                // ARRAY UPDATE
                a_newField[i][j] = 
                    coefficient * a_curField[i][j] + dTi_dt;                                                       //  // Last value + temperature difference times the time difference
            }
        }
    }

    return 0;                                                                       //  // Required
}


void printArray(float field[][NODES_X]) {                                           // To print array    
    for (int i = 0; i < NODES_Y; i++) {                                             //  // For loop for element
        for (int j = 0; j < NODES_X; j++) {
            // NEGATIVE VALUE HANDLING
            if (field[i][j] < 0.0f) {field[i][j] = 0.0f;}                           //  // Avoids negative numbers due to small floating point errors
            
            // VERTICAL CHECK
            if (NODES_X == 1) {
                printf("[ %c ]\n", heatVisualsReturn(field[i][j]));
                continue;
            }

            // DISPLAY DEVELOPMENT
            if (j == 0) {
                printf("[ %c, ", heatVisualsReturn(field[i][j]));
            }                                                                       //  // If first element, include the left bracket
            
            else if (j == (NODES_X - 1)) {
                printf("%c ]\n", heatVisualsReturn(field[i][j]));
            }                                                                       //  // Else if the element is the last, include the right bracket
            
            else {
                printf("%c, ", heatVisualsReturn(field[i][j]));
            }                                                                       //  // Otherwise do the normal format
        }
    }
}

void printRawArray(float field[][NODES_X]) {                                             // To print array    
    for (int i = 0; i < NODES_Y; i++) {                                             //  // For loop for element
        for (int j = 0; j < NODES_X; j++) {
            // NEGATIVE VALUE HANDLING
            if (field[i][j] < 0.0f) {field[i][j] = 0.0f;}                           //  // Avoids negative numbers due to small floating point errors
            
            // VERTICAL CHECK
            if (NODES_X == 1) {
                printf("[ %-6.2f ]\n", field[i][j]);
                continue;
            }

            // DISPLAY DEVELOPMENT
            if (j == 0) {
                printf("[ %-6.2f, ", field[i][j]);
            }                                                                       //  // If first element, include the left bracket
            
            else if (j == (NODES_X - 1)) {
                printf("%-6.2f ]\n", field[i][j]);
            }                                                                       //  // Else if the element is the last, include the right bracket
            
            else {
                printf("%-6.2f, ", field[i][j]);
            }                                                                       //  // Otherwise do the normal format
        }
    }
}

void copyArray(float copyField[][NODES_X], float pasteField[][NODES_X]) {            // To copy arrays of the same dimension size
    for (int i = 0; i < NODES_Y; i++) {
        for (int j = 0; j < NODES_X; j++) {
            pasteField[i][j] = copyField[i][j];
        }
    }                                                                               //  // Replaces each field
}

void showArrayStats(float sum, float initialEnergy) {
    float deviation = 0.0f;                                                         //  // Inits the percent deviation
    deviation = fabs((sum - initialEnergy) / initialEnergy);                         //  // Percent difference of energy

    // PRINT
    printf(
        "\nEnergy: %-5f, deviation: %%%6.3f\n",
        round(sum), deviation*100
        );                                                                          //  // Prints the actual energy
    printf("Intended Energy: %5.4f\n", initialEnergy);                              //  // Print the theoretical energy

}


float sumArray(float field[][NODES_X]) {                                            // Gets the total value of an array
    // SUM INIT
    float sum = 0.0f;                                                               //  // Inits the sum variable
    // CUMULATIVE COUNT
    for (int i = 0; i < NODES_Y; i++) {
        for (int j = 0; j < NODES_X; j++) {
            sum += field[i][j];        
            }
    }                                                                               //  // Replaces each field
    return sum;
}

char heatVisualsReturn(float value) {
    float maxValue = (BAR_POINT_ONE + BAR_POINT_TWO) / 2;

    float averageEnergyDensity = 
        (BAR_POINT_ONE + BAR_POINT_TWO) 
        / (NODES_X*NODES_Y);

    if (value >  (maxValue/2))              {return heat_visuals[0];}
    if (value >  (maxValue/20))             {return heat_visuals[1];}
    if (value >  (maxValue/40))             {return heat_visuals[2];}
    if (value >  (maxValue/60))             {return heat_visuals[3];}
    if (value >  (maxValue/80))             {return heat_visuals[4];}
    if (value >  (maxValue/90))             {return heat_visuals[5];}
    if (value >  (maxValue/95))             {return heat_visuals[6];}
    if (value >  (averageEnergyDensity))    {return '#';}
    if (value >= 1.0f)                      {return '0';}
    if (value >= 0.5f)                      {return 'x';}
    if (value >= 0.0f)                      {return '.';}
    else                                    {return 'E';}
}