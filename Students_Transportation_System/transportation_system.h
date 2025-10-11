/* Ioannis Kapetanakis - csd4641 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

// Define student departments
#define MATHS 0
#define PHYSICS 1
#define CHEMI 2
#define CSD 3

// Student structure
typedef struct{
    int AM; // Student ID
    int Department;
    int study_time; 
    int go_to_uni;     // flag that equals 1 if the student goes to the university (else flag is equal to zero)
    sem_t student_sem; // A binary semaphore, that ensures that the student is properly synchronized at the stops and inside the bus
    pthread_t Thread; // Student's thread
} Student;

// Represents a node in the linked list for students in the bus
typedef struct Node{
    Student *student_Node;
    struct Node *next_Student;
} busNode;

struct bus{
    busNode *head;
    int NumOfSeats; // Total number of seats in the bus
    int seats_taken; // Number of taken seats in the bus
    int maths_dep_counter; // Counter for Maths Students
    int physics_dep_counter; // Counter for Physics Students
    int chemi_dep_counter; // Counter for Chemistry Students
    int csd_dep_counter; // Counter for CSD Students
    pthread_t Thread; // Bus thread
};

// Represents a node in the linked list (queue) for students in the stop
typedef struct Stop_Node{
    Student *student_Node;
    struct Stop_Node *next_Student;
} stop_Node;

typedef struct stop{
   stop_Node *head; // Head of stop's linked list (queue)
} stop;

// Represents a node in the linked list for students in the University
typedef struct Uni_Node{
    Student *student_Node;
    struct Uni_Node *next_Student;
} Uni_Node;

typedef struct Uni_list{
   Uni_Node *head; // Head of Uni's linked list (queue)
} UniversityList;

// Function for students initialization
Student *init_students(size_t N);

// Function executed by student's thread
void *student_func(void *AM);

// Converts integer to string (used for department names)
char *int_to_str(int dep);

// Checks if there is available space in the bus, for a specific department
int dep_space_left(int dep);

// Function for bus initialization
void init_bus(int N);

// Function executed by the bus thread
void *bus_func();

// Adds a student to the bus (a node in bus queue)
void enter_bus(Student *student);

// Removes a student from the bus (a node in bus queue)
void leave_bus(Student *student);

// Prints the current status of the bus
void bus_print();

// Initialization of stop A (Chanioporta)
void stopA_init();

// Adds a student at stop A (a node in stop A queue)
void stopA_student_arrive(Student *student);

// Removes a student from stop A (a node in stop A queue)
void stopA_student_leave(Student *student);

// Initialization of stop B (University bus stop)
void stopB_init();

// Adds a student at stop B (a node in stop B queue)
void stopB_student_arrive(Student *student);

// Removes a student from stop B (a node in stop B queue)
void stopB_student_leave(Student *student);

// Prints the current status of stop A
void stopA_print();

// Prints the current status of stop B
void stopB_print();

// Initialization of University
void init_university();

// Adds a student into University
void uni_enter(Student *student);

// Study simulation
void uni_study(Student *student);

// Removes a student from University
void uni_leave(Student *student);

// Prints the current status of university
void uni_print();

// Prints the current status of, stop A, stop B, bus stop of university and university
void print_all();