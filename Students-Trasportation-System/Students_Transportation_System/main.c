/* Ioannis Kapetanakis - csd4641 */

#include "transportation_system.h"

int main(){
    srand(time(0));

    int NumOfStudents; // Variable that stores the number of students
    int NumOfSeats; // Variables that stores the number of seats in the bus

    printf("Enter the number of students: ");
    scanf("%d", &NumOfStudents);

    printf("Enter the number of seats in the bus: ");
    scanf("%d", &NumOfSeats);

    printf("\n");

    // Initialize university
    init_university();

    // Initialize stop A (Chanioporta)
    stopA_init();

    // Initialize stop B (University bus stop)
    stopB_init();

    // Initialize bus (and run its thread)
    init_bus(NumOfSeats);

    // Initialize students (and run their threads)
    Student* students = init_students(NumOfStudents);

    printf("All students from stop A went to University and came back!\n");
    
    free(students);

    return 0;
}