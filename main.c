#include <stdio.h>                                                                  // Needed for printing
#include <windows.h>                                                                // Needed for sleep command on Windows
#include <conio.h>                                                                  // Needed for terminal commands on Windows

#define SIM_TIME_DELAY_MILLI    10
#define NODES                   5                                                   // Manual array size

#define ALPHA                   0.0001f                                             // The thermal conductivity K (or alpha)
#define BAR_LENGTH              0.1f                                                // Bar Length in metres
#define DELTA_X                 BAR_LENGTH / NODES                                  // Distance between nodes

#define BAR_L_START_TEMP        100.0f                                              // Temp at N=0
#define BAR_AMBIENT_TEMP        40.0f                                               // Temp between N=1 and N=N_end-1
#define BAR_L_END_TEMP          10.0f                                               // Temp at N=N_end

#define DELTA_TIME              0.1f                                                // Time step
#define FINAL_TIME              600.0f                                              // Final time


void printArray(float field[]);                                                     // This prints the array
void copyArray(float copyField[], float pasteField[]);                              // This copies the array from one to another
void sumArray(float field[]);                                                       // This totals the values of the field, used to calculate the total energy of the system (for error checking)


int main()                                                                          // Script start
{
    // INIT SECTION
    printf("Hello world!\n");                                                       //  // Sanity test
    int frameCount = 1;                                                             //  // Frame counter init

    // INITIAL CONDITIONS ARRAY
    float a_startField[NODES];                                                      
    for(int i = 0 ; i < NODES; i++) {a_startField[i]=BAR_AMBIENT_TEMP;};
    a_startField[0]             = BAR_L_START_TEMP;                                 
    a_startField[NODES - 1]     = BAR_L_END_TEMP;                                   

    // NEW FIELD
    float a_newField[NODES];                                                        //  // Inits the current field, the current field is the field presented
    for(int i = 0 ; i < NODES; i++) {a_newField[i]=BAR_AMBIENT_TEMP;};
    a_newField[0]               = BAR_L_START_TEMP;                                 //  // Sets the initial conditions
    a_newField[NODES - 1]       = BAR_L_END_TEMP;                                   //  // Bar end temp

    // CURRENT FIELD
    float a_curField[NODES];                                                        //  // Inits the working field, basically holds the values used for calculations, while the a_currentField gets updated
    for(int i = 0 ; i < NODES; i++) {a_curField[i]=BAR_AMBIENT_TEMP;};
    a_curField[0]               = BAR_L_START_TEMP;                                 //  // Sets the initial conditions, currently written manually despite copyArray() function because it's not worth it atm
    a_curField[NODES - 1]       = BAR_L_END_TEMP;                                   //  // Bar end temp

    // DIFFERENTIATION VARIABLES
    float dTi_dt;
    float dx2 = DELTA_X * DELTA_X;

    // TEMPERATURE AVERAGES
    float aveTempI;                                                                 //  // Overall average value
    float aveTempLeft;                                                              //  // Average between the current node and the one to the left
    float aveTempRight;                                                             //  // Average between the current node and the one to the right
    
    // TIME COUNTER
    float time = 0.0f;                                                              //  // Time of sim init
    
    // CLEARS TERMINAL OF OTHER INFO
    system("cls");                                                                  //  // Clears screen

    // LOOP SECTION
    while(time < FINAL_TIME)                                                        //  // This is the while loop
    {
        // INFORMATION
        printf("Iteration: %d\tTime: %4.3f\n", frameCount, time);                   //  // Makes sure that the loop is working
        printArray(a_newField);                                                     //  // Prints the array

        // ENERGY SANITY CHECK
        sumArray(a_newField);                                                       //  // Prints the total energy of the system

        // INITIAL CONDITIONS PRINT
        printf("Starting Conditions:");
        printArray(a_startField);                                                   //  // Display of the initial conditions, this is to given an idea of the setup
        
        // COPIES THE ARRAY TO NEW FIELD
        copyArray(a_newField, a_curField);                                          //  // Updates the working field
        
        // BEHIND-THE-SCENE STUFF
        frameCount++;                                                               //  // Increments the counter
        Sleep(SIM_TIME_DELAY_MILLI);                                                //  // Delay between refreshes
        system("cls");                                                              //  // Clears screen
        time += DELTA_TIME;                                                         //  // Increments the time frame

        // CALCULATION LOOP
        for (int i = 0; i < NODES; i++)                                             //  // Iterates through the array
        {
            // BOUNDARIES
            if (i == 0)                                                             //  // If the value is at L = 0 (beginning)
            {
                aveTempLeft     = 0;                                                //  // No node to the left
                aveTempRight    =   ( a_curField[i + 1] - a_curField[i] );
            }
            else if (i == (NODES-1))                                                //  // If the value is at L = L (ending)
            {
                aveTempLeft     = - ( a_curField[i] - a_curField[i - 1] );
                aveTempRight    = 0;                                                //  // No node to the right
            }
            else                                                                    //  // Remaining elements
            {
                aveTempLeft     = - ( a_curField[i] - a_curField[i - 1] );
                aveTempRight    =   ( a_curField[i + 1] - a_curField[i] );
            }

            // CALCULATIONS
            aveTempI    = (aveTempLeft + aveTempRight) / dx2;                       //  // Average temp
            dTi_dt      = ALPHA * aveTempI;                                         //  // Average temp times the scaler (thermal conductivity)

            // ARRAY UPDATE
            a_newField[i] =  a_curField[i] + dTi_dt * DELTA_TIME;                   //  // Last value + temperature difference times the time difference       
        }
    }

    return 0;                                                                       //  // Required
}


void printArray(float field[])                                                      // To print array
{
    for (int i = 0; i < NODES; i++)                                                 //  // For loop for elements
    {
        // NEGATIVE VALUE HANDLING
        if (field[i] < 0) {field[i] = 0.0f;}                                        //  // Avoids negative numbers due to small floating point errors

        // DISPLAY DEVELOPMENT
        if (i == 0) {printf("[ %5.2f, ", field[i]);}                                //  // If first element, include the left bracket
        else if (i == (NODES - 1)) {printf("%5.2f ]\n", field[i]);}                 //  // Else if the element is the last, include the right bracket
        else {printf("%5.2f, ", field[i]);}                                         //  // Otherwise do the normal format
    }
}


void copyArray(float copyField[], float pasteField[])                               // To copy arrays of the same dimension size
{
    for (int i = 0; i < NODES; i++) {pasteField[i] = copyField[i];}                 //  // Replaces each field
}


void sumArray(float field[])                                                        // Gets the total value of an array
{
    // SUM INIT
    float sum = 0;                                                                  //  // Inits the sum variable

    // CUMULATIVE COUNT
    for (int i = 0; i < NODES; i++) {sum += field[i];}                              //  // Cumulative value

    // SANITY CHECK
    float initialEnergy =                                                           //  // This just makes sure the the energy is keeping to the initial energy
        BAR_L_START_TEMP 
        + (NODES - 2) * BAR_AMBIENT_TEMP
        + BAR_L_END_TEMP;

    // PRINT
    printf("\nEnergy: %5.4f\nIntended Energy: %5.4f\n", sum, initialEnergy);        //  // Print the total

}