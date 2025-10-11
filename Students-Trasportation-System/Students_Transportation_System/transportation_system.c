/* Ioannis Kapetanakis - csd4641 */

#include "transportation_system.h"

struct bus *Bus; 
stop *stopA, *stopB;
UniversityList *UniList;

sem_t bus_sem;   // Semaphore for bus capacity
sem_t stopA_sem; // Semaphore to control access to stop A
sem_t stopB_sem; // Semaphore to control access to stop B
sem_t start_bus; // Semaphore for starting bus function
sem_t uni_sem;  // Semaphore for university capacity

pthread_mutex_t enter_or_leave_stopA; // Mutex for entering or leaving from stop A
pthread_mutex_t enter_or_leave_stopB; // Mutex for entering or leaving from stop B
pthread_mutex_t enter_uni;            // Mutex for entering to the university
pthread_mutex_t leave_uni;            // Mutex for leaving from the university
pthread_mutex_t leave_bus_mutex;      // Mutex for leaving from the bus
pthread_mutex_t University[200];      // Mutex array representing 200 study spaces at the university

pthread_barrier_t barrier_at_stopA;   // Barrier for synchronization at stop A

// Function for students initialization
Student *init_students(size_t N){
    if(pthread_barrier_init(&barrier_at_stopA, NULL, N) != 0){ // Initialize the barrier
        fprintf(stderr, "Barrier initialization error\n");
        exit(-1);
    }
    Student *students = malloc(sizeof(Student)*N);
    for(int i=0; i<N; i++){
        students[i].AM = 1000+i; // Set student's ID
        students[i].Department = rand()%4; // Set student's department
        students[i].study_time = rand()%(15 - 5 + 1) + 5; // Set student's study time, for random value in range [min,max]: rand() % (max - min + 1) + min
        students[i].go_to_uni = 1; // Indication that the student is going to university
        if(sem_init(&students[i].student_sem, 0, 0) != 0){  // Initialization of student's semaphore
            fprintf(stderr, "Semaphore initialization error\n");
            exit(-1);
        }
        if(pthread_create(&students[i].Thread, NULL, student_func, &students[i]) != 0){ // Student's thread creation
            fprintf(stderr, "Thread creation error\n");
            exit(-1);
        }
    }

    for(int i=0; i<N; i++){
        if(pthread_join(students[i].Thread, NULL) != 0){ // Wait for each student's thread to finish
            fprintf(stderr, "Thread join error\n");
            exit(-1);
        }
    }

    return students;
}

void *student_func(void *student){
    Student *created_student = (Student*)student;
    
    // In this mutex and critical region, the creation of the thread and its assignment to stop A is announced
    if(pthread_mutex_lock(&enter_or_leave_stopA) != 0){
        fprintf(stderr, "Mutex lock error\n");
        exit(-1);
    }
    printf("Student %d (%s) created\n\n", created_student->AM, int_to_str(created_student->Department));
    stopA_student_arrive(created_student);
    print_all();
    sleep(1);
    if(pthread_mutex_unlock(&enter_or_leave_stopA) != 0){
        fprintf(stderr, "Mutex unlock error\n");
        exit(-1);
    }
    
    // Wait for all threads to reach stop A
    int barrier_int = pthread_barrier_wait(&barrier_at_stopA);

    if(barrier_int != 0 && barrier_int != PTHREAD_BARRIER_SERIAL_THREAD){
        fprintf(stderr, "Barrier wait error\n");
        exit(-1);
    }

    // The first node of the stop A queue, signals the bus to start
    if(stopA->head->student_Node == created_student){
        if(sem_post(&start_bus) != 0){
            fprintf(stderr, "Post semaphore error\n");
            exit(-1);
        }
    }
    
    // In this while loop, the entrance to the bus is simulated (from stop A)
    while(1){
        if(sem_wait(&created_student->student_sem) != 0){ // Wait for bus signal for boarding (ensures entrance with FIFO)
            fprintf(stderr, "Wait semaphore error\n");
            exit(-1);
        }
        if(sem_wait(&stopA_sem) != 0){ // Ensures that the current student AND the bus are at stop A and the student has exclusive access
            fprintf(stderr, "Wait semaphore error\n");
            exit(-1);
        }
        // Check for available space on bus
        if(dep_space_left(created_student->Department)){
            stopA_student_leave(created_student); // Student leaves stop A
            enter_bus(created_student); // Student enters the bus
            printf("Student %d (%s) boarded to the bus\n\n", created_student->AM, int_to_str(created_student->Department));
            print_all();

            if(sem_post(&stopA_sem) != 0){
                fprintf(stderr, "Post semaphore error\n");
                exit(-1);
            }
            if(sem_wait(&created_student->student_sem) != 0){ // Waiting for the bus to arrive at the university
                fprintf(stderr, "Wait semaphore error\n");
                exit(-1);
            }
            break;
        }
        else{
            printf("Student %d (%s) cannot enter the bus\n\n", created_student->AM, int_to_str(created_student->Department));
        }
        if(sem_post(&stopA_sem) != 0){
            fprintf(stderr, "Post semaphore error\n");
            exit(-1);
        }
    }

    // Mutex and critical region for leaving the bus
    if(pthread_mutex_lock(&leave_bus_mutex) != 0){
        fprintf(stderr, "Mutex lock error\n");
        exit(-1);
    }
    leave_bus(created_student);
    if(pthread_mutex_unlock(&leave_bus_mutex) != 0){
        fprintf(stderr, "Mutex unlock error\n");
        exit(-1);
    }

    // Mutex and critical region for entering stop B
    if(pthread_mutex_lock(&enter_or_leave_stopB) != 0){
        fprintf(stderr, "Mutex lock error\n");
        exit(-1);
    }
    printf("Student %d (%s) got off the bus\n\n", created_student->AM, int_to_str(created_student->Department));
    stopB_student_arrive(created_student);
    print_all();
    if(pthread_mutex_unlock(&enter_or_leave_stopB)){
        fprintf(stderr, "Mutex unlock error\n");
        exit(-1);
    }

    // Student remains for 5 seconds at stop B (for simulation and synchronization purposes)
    sleep(5);

    // Mutex and critical region for leaving stop B
    if(pthread_mutex_lock(&enter_or_leave_stopB) != 0){
        fprintf(stderr, "Mutex lock error\n");
        exit(-1);
    }
    stopB_student_leave(created_student);
    if(pthread_mutex_unlock(&enter_or_leave_stopB)){
        fprintf(stderr, "Mutex unlock error\n");
        exit(-1);
    }

    // Mutex and critical region for entering in the university
    if(pthread_mutex_lock(&enter_uni) != 0){
        fprintf(stderr, "Mutex lock error\n");
        exit(-1);
    }
    uni_enter(created_student);
    printf("Student %d (%s) went to university\n\n", created_student->AM, int_to_str(created_student->Department));
    print_all();
    if(pthread_mutex_unlock(&enter_uni) != 0){
        fprintf(stderr, "Mutex unlock error\n");
        exit(-1);
    }

    sleep(7); // Student remains for 7 seconds in the university before start studyng. I added this sleep, so that there are not 
              // many students in parallel at stop B and so that the bus divides the time better between the 2 stops

    // Semaphores and critical region for studying (semaphores here ensure that no more than 200 students will be studying)
    if(sem_wait(&uni_sem) != 0){
        fprintf(stderr, "Wait semaphore error\n");
        exit(-1);
    }
    uni_study(created_student); // Student study simulation
    if(sem_post(&uni_sem) != 0){
        fprintf(stderr, "Post semaphore error\n");
        exit(-1);
    }

    // Mutex and critical region for leaving university
    if(pthread_mutex_lock(&leave_uni) != 0){
        fprintf(stderr, "Mutex lock error\n");
        exit(-1);
    }
    uni_leave(created_student);
    created_student->go_to_uni=0;
    if(pthread_mutex_unlock(&leave_uni) != 0){
        fprintf(stderr, "Mutex unlock error\n");
        exit(-1);
    }

    // Mutex and critical region for entering stop B
    if(pthread_mutex_lock(&enter_or_leave_stopB) != 0){
        fprintf(stderr, "Mutex lock error\n");
        exit(-1);
    }
    stopB_student_arrive(created_student);
    print_all();
    if(pthread_mutex_unlock(&enter_or_leave_stopB) != 0){
        fprintf(stderr, "Mutex unlock error\n");
        exit(-1);
    }

    // In this while loop, the entrance to the bus is simulated (from stop B)
    while(1){
        if(sem_wait(&created_student->student_sem) != 0){ // Wait for bus signal for boarding (ensures entrance with FIFO)
            fprintf(stderr, "Wait semaphore error\n");
            exit(-1);
        }
        if(sem_wait(&stopB_sem) != 0){ // Ensures that the current student AND the bus are at stop A and the student has exclusive access
            fprintf(stderr, "Wait semaphore error\n");
            exit(-1);
        }

        // Check for available space on bus
        if(dep_space_left(created_student->Department)){
            stopB_student_leave(created_student); // Student leaves stop B
            enter_bus(created_student); // Student enters the bus
            printf("Student %d (%s) boarded to the bus\n\n", created_student->AM, int_to_str(created_student->Department));
            print_all();

            if(sem_post(&stopB_sem) != 0){
                fprintf(stderr, "Post semaphore error\n");
                exit(-1);
            }
            if(sem_wait(&created_student->student_sem) != 0){  // Waiting for the bus to arrive to Chanioporta
                fprintf(stderr, "Wait semaphore error\n");
                exit(-1);
            }
            break;
        }
        else{
            printf("Student %d (%s) cannot enter the bus\n\n", created_student->AM, int_to_str(created_student->Department));
        }
        if(sem_post(&stopB_sem) != 0){
            fprintf(stderr, "Post semaphore error\n");
            exit(-1);
        }
    }
    
    // Mutex and critical region for leaving the bus
    if(pthread_mutex_lock(&leave_bus_mutex) != 0){
        fprintf(stderr, "Mutex lock error\n");
        exit(-1);
    }
    leave_bus(created_student);
    printf("Student %d (%s) went home\n\n", created_student->AM, int_to_str(created_student->Department));
    print_all();
    if(pthread_mutex_unlock(&leave_bus_mutex) != 0){
        fprintf(stderr, "Mutex unlock error\n");
        exit(-1);
    }

    if(sem_destroy(&created_student->student_sem) != 0){
        fprintf(stderr, "Semaphore destroy error\n");
        exit(-1);
    }
    pthread_exit(NULL);
}

// Converts integer to string (used for department names)
char *int_to_str(int dep){
    switch(dep){
        case 0:
            return "Math";
        case 1:
            return "Physics";
        case 2:
            return "Chemistry";
        case 3:
            return "CSD";
        default:
            return NULL;
    }
}

// Checks if there is available space in the bus, for a specific department
int dep_space_left(int dep){
    switch(dep){
        case MATHS:
            if(Bus->maths_dep_counter == (int)(Bus->NumOfSeats/4)){
                return 0;
            }
            return 1;
        case PHYSICS:
            if(Bus->physics_dep_counter == (int)(Bus->NumOfSeats/4)){
                return 0;
            }
            return 1;
        case CHEMI:
            if(Bus->chemi_dep_counter == (int)(Bus->NumOfSeats/4)){
                return 0;
            }
            return 1;
        case CSD:
            if(Bus->csd_dep_counter == (int)(Bus->NumOfSeats/4)){
                return 0;
            }
            return 1;
        default:
            return 0;
    }
}


// Function for bus initialization
void init_bus(int N){
    Bus = malloc(sizeof(struct bus));
    Bus->head = NULL;
    Bus->NumOfSeats = N;
    Bus->seats_taken = 0;
    Bus->maths_dep_counter = 0;
    Bus->physics_dep_counter = 0;
    Bus->chemi_dep_counter = 0;
    Bus->csd_dep_counter = 0;
    if(sem_init(&bus_sem, 0, N) != 0){ // Initialize bus capacity semaphore
        fprintf(stderr, "Semaphore initialization error\n");
        exit(-1);
    }
    if(sem_init(&start_bus, 0, 0) != 0){ // Initialize semaphore for starting the bus
        fprintf(stderr, "Semaphore initialization error\n");
        exit(-1);
    }
    if(pthread_mutex_init(&leave_bus_mutex, NULL) != 0){ // Initialize the mutex for leaving the bus
        fprintf(stderr, "Mutex initialization error\n");
        exit(-1);
    }
    if(pthread_create(&Bus->Thread, NULL, bus_func, NULL) != 0){
        fprintf(stderr, "Thread creation error\n");
        exit(-1);
    }
}

// Function executed by the bus thread
void *bus_func(){

    //Initialize pointers to access the stop A, stop B and bus lists (queues)
    stop_Node *ptr = NULL;
    stop_Node *ptr2 = NULL;
    busNode *ptr_bus = NULL;
    busNode *ptr2_bus = NULL;
    if(sem_wait(&start_bus) != 0){ // Wait, until the first node of stop A list, enables the bus
        fprintf(stderr, "Wait semaphore error\n");
        exit(-1);
    }
    while(1){
        // Bus arrives at stop A and allows students to board
        ptr = stopA->head;
        if(ptr!=NULL && ptr->next_Student!=NULL){
            ptr2=ptr->next_Student;
        }
        if(sem_post(&stopA_sem) != 0){
            fprintf(stderr, "Post semaphore error\n");
            exit(-1);
        }
        while(ptr!=NULL){
            if(sem_post(&ptr->student_Node->student_sem) != 0){ // Signal student to board
                fprintf(stderr, "Post semaphore error\n");
                exit(-1);
            }
            sleep(1);
            ptr = ptr2;
            if(ptr2!=NULL){
                ptr2 = ptr2->next_Student;
            }
        } 
        if(sem_wait(&stopA_sem) != 0){
            fprintf(stderr, "Wait semaphore error\n");
            exit(-1);
        }

        printf("Bus is on the way to University...\n");
        sleep(10); // Travel time to university (T = 10)
        printf("Bus arrived at University...\n\n");

        // Bus arrives at university and allows students to get off
        ptr_bus = Bus->head;
        if(ptr_bus!=NULL && ptr_bus->next_Student!=NULL){
            ptr2_bus=ptr_bus->next_Student;
        }
        while(ptr_bus!=NULL){
            if(sem_post(&ptr_bus->student_Node->student_sem) != 0){ // Signal student to get off
                fprintf(stderr, "Post semaphore error\n");
                exit(-1);
            }
            sleep(1);
            ptr_bus = ptr2_bus;
            if(ptr2_bus!=NULL){
                ptr2_bus = ptr2_bus->next_Student;
            }
        } 

        // Bus allows students to board
        ptr = stopB->head;
        if(ptr!=NULL && ptr->next_Student!=NULL){
            ptr2=ptr->next_Student;
        }
        if(sem_post(&stopB_sem) != 0){
            fprintf(stderr, "Post semaphore error\n");
            exit(-1);
        }
        while(ptr!=NULL){
            if(!ptr->student_Node->go_to_uni){
                if(sem_post(&ptr->student_Node->student_sem) != 0){ // Signal student to board
                    fprintf(stderr, "Post semaphore error\n");
                    exit(-1);
                }
            }
            sleep(1);
            ptr = ptr2;
            if(ptr2!=NULL){
                ptr2 = ptr2->next_Student;
            }
        } 
        if(sem_wait(&stopB_sem) != 0){
            fprintf(stderr, "Wait semaphore error\n");
            exit(-1);
        }

        printf("Bus is heading to stop A...\n");
        sleep(10); // Travel time to university (T = 10)
        printf("Bus arrived to stop A!\n\n");

        // Bus arrives at stop A and allows students to get off
        ptr_bus = Bus->head;
        if(ptr_bus!=NULL && ptr_bus->next_Student!=NULL){
            ptr2_bus=ptr_bus->next_Student;
        }
        while(ptr_bus!=NULL){
            if(sem_post(&ptr_bus->student_Node->student_sem) != 0){ // Signal student to get 
                fprintf(stderr, "Post semaphore error\n");
                exit(-1);
            }
            sleep(1);
            ptr_bus = ptr2_bus;
            if(ptr2_bus!=NULL){
                ptr2_bus = ptr2_bus->next_Student;
            }
        }

        ptr = NULL;
        ptr2 = NULL;
        ptr_bus = NULL;
        ptr2_bus = NULL;

        // If all the students went home (If all the lists are empty), then destroy all the mutexes and semaphores and end the function of bus
        if(stopA->head == NULL && stopB->head == NULL && Bus->head == NULL && UniList->head == NULL){
            if(pthread_barrier_destroy(&barrier_at_stopA) != 0){
                fprintf(stderr, "Barrier destroy error\n");
                exit(-1);
            }

            if(sem_destroy(&bus_sem) != 0){
                fprintf(stderr, "Semaphore destroy error\n");
                exit(-1);
            }
            if(sem_destroy(&stopA_sem) != 0){
                fprintf(stderr, "Semaphore destroy error\n");
                exit(-1);
            }
            if(sem_destroy(&stopB_sem) != 0){
                fprintf(stderr, "Semaphore destroy error\n");
                exit(-1);
            }
            if(sem_destroy(&start_bus) != 0){
                fprintf(stderr, "Semaphore destroy error\n");
                exit(-1);
            }

            if(pthread_mutex_destroy(&enter_or_leave_stopA) != 0){
                fprintf(stderr, "Mutex destroy error\n");
                exit(-1);
            }
            if(pthread_mutex_destroy(&enter_or_leave_stopB) != 0){
                fprintf(stderr, "Mutex destroy error\n");
                exit(-1);
            }
            if(pthread_mutex_destroy(&enter_uni) != 0){
                fprintf(stderr, "Mutex destroy error\n");
                exit(-1);
            }
            if(pthread_mutex_destroy(&leave_uni) != 0){
                fprintf(stderr, "Mutex leave_uni destroy error\n");
                exit(-1);
            }
            if(pthread_mutex_destroy(&leave_bus_mutex) != 0){
                fprintf(stderr, "Mutex destroy error\n");
                exit(-1);
            }

            pthread_exit(NULL);

        }
    }
}

// Adds a student to the bus
void enter_bus(Student *student){
    if(sem_wait(&bus_sem) != 0){ // Reduce capacity in the bus
        fprintf(stderr, "Wait semaphore error\n");
        exit(-1);
    }

    busNode *newNode = malloc(sizeof(busNode));
    busNode *tmp = NULL;
    newNode->student_Node = student;

    // Insert the student at the end of the bus queue
    if(Bus->head == NULL){
        Bus->head = newNode;
        newNode->next_Student = NULL;
    }
    else{
        tmp = Bus->head;
        while(tmp->next_Student != NULL){
            tmp = tmp->next_Student;
        }
        tmp->next_Student = newNode;
        newNode->next_Student = NULL;
    }

    // Update bus seat counters based on student's department
    Bus->seats_taken++;
    switch(student->Department){
        case MATHS:
            Bus->maths_dep_counter++;
            break;
        case PHYSICS:
            Bus->physics_dep_counter++;
            break;
        case CHEMI:
            Bus->chemi_dep_counter++;
            break;
        case CSD:
            Bus->csd_dep_counter++;
            break;
        default:
            break;
    }
}

// Removes a student from the bus (a node in bus queue)
void leave_bus(Student *student){
    if(Bus->seats_taken>0){
        if(sem_post(&bus_sem) != 0){ // Increase capacity in the bus
            fprintf(stderr, "Post semaphore error\n");
            exit(-1);
        }
        busNode *tmp = Bus->head;

        // Search for the student in list
        while(tmp != NULL && tmp->student_Node != student){
            tmp = tmp->next_Student;
        }

        // Remove student (node) for the bus
        if(tmp!=NULL){
            if(Bus->head == tmp){
                if(Bus->head->next_Student == NULL){
                    Bus->head = NULL;
                }
                else{
                    Bus->head = Bus->head->next_Student;
                }
                tmp->next_Student = NULL;
            }
            else if(tmp->next_Student == NULL){
                busNode *tmp2 = Bus->head;
                while(tmp2->next_Student != tmp){
                    tmp2 = tmp2->next_Student;
                }
                tmp2->next_Student = NULL;
            }
            else{
                busNode *tmp2 = Bus->head;
                while(tmp2->next_Student != tmp){
                    tmp2 = tmp2->next_Student;
                }
                tmp2->next_Student = tmp->next_Student;
                tmp->next_Student = NULL;
            }

            // Update department counter
            Bus->seats_taken--;
            switch(student->Department){
                case MATHS:
                    Bus->maths_dep_counter--;
                    break;
                case PHYSICS:
                    Bus->physics_dep_counter--;
                    break;
                case CHEMI:
                    Bus->chemi_dep_counter--;
                    break;
                case CSD:
                    Bus->csd_dep_counter--;
                    break;
                default:
                    break;
            }
        }
    }
}

// Prints the current status of the bus
void bus_print(){
    busNode *ptr = Bus->head;
    printf("Bus: ");
    if(ptr!=NULL){
        while(ptr!=NULL){
            printf("[%d, %s] ", ptr->student_Node->AM, int_to_str(ptr->student_Node->Department));
            ptr = ptr->next_Student;
        }
    }
    printf("\n");
}

// Initialization of stop A (Chanioporta)
void stopA_init(){
    if(sem_init(&stopA_sem, 0, 0) != 0){ // Initialize the semaphore for stop A
        fprintf(stderr, "Semaphore initialization error\n");
        exit(-1);
    }
    if(pthread_mutex_init(&enter_or_leave_stopA, NULL) != 0){ // Initialize the mutex for entering or leaving stop A
        fprintf(stderr, "Mutex initialization error\n");
        exit(-1);
    }
    stopA = malloc(sizeof(stop));
    stopA->head = NULL;
}

// Adds a student at stop A (a node in stop A queue)
void stopA_student_arrive(Student *student){
    stop_Node *newNode = malloc(sizeof(stop_Node));
    newNode->student_Node = student;
    if(stopA->head == NULL){
        stopA->head = newNode;
    }
    else{
       stop_Node *tmp = stopA->head;
       while(tmp->next_Student!=NULL){
            tmp = tmp->next_Student;
       }
       tmp->next_Student = newNode;
    }
    newNode->next_Student = NULL;
}

// Removes a student from stop A (a node in stop A queue)
void stopA_student_leave(Student *student){
    if(stopA->head == NULL){
        return;
    }

    // Search for the student in the queue
    stop_Node *tmp = stopA->head;
    while(tmp != NULL && tmp->student_Node != student){
        tmp = tmp->next_Student;
    }

    // Remove student from the queue
    if(tmp != NULL){
        if(tmp == stopA->head){
            if(stopA->head->next_Student == NULL){
                stopA->head = NULL;
            }
            else{
                stopA->head = stopA->head->next_Student;
            }
            tmp->next_Student = NULL;
        }
        else if(tmp->next_Student == NULL){
            stop_Node *tmp2 = stopA->head;
            while(tmp2->next_Student != tmp){
                tmp2 = tmp2->next_Student;
            }
            tmp2->next_Student = NULL;
        }
        else{
            stop_Node *tmp2 = stopA->head;
            while(tmp2->next_Student != tmp){
                tmp2 = tmp2->next_Student;
            }
            tmp2->next_Student = tmp->next_Student;
            tmp->next_Student = NULL;
        }
    }
}

// Prints the current status of stop A
void stopA_print(){
    stop_Node *ptr = stopA->head;
    printf("StopA: ");
    if(ptr!=NULL){
        while(ptr!=NULL){
            printf("[%d, %s] ", ptr->student_Node->AM, int_to_str(ptr->student_Node->Department));
            ptr = ptr->next_Student;
        }
    }
    printf("\n");
}

// Prints the current status of stop B
void stopB_init(){
    if(sem_init(&stopB_sem, 0, 0) != 0){
        fprintf(stderr, "Semaphore initialization error\n");
        exit(-1);
    }
    if(pthread_mutex_init(&enter_or_leave_stopB, NULL) != 0){
        fprintf(stderr, "Mutex initialization error\n");
        exit(-1);
    }
    stopB = malloc(sizeof(stop));
    stopB->head = NULL;
}

// Adds a student at stop B (a node in stop B queue)
void stopB_student_arrive(Student *student){
    stop_Node *newNode = malloc(sizeof(stop_Node));
    newNode->student_Node = student;

    if(stopB->head == NULL){
        stopB->head = newNode;
    }
    else{
       stop_Node *tmp = stopB->head;
       while(tmp->next_Student!=NULL){
            tmp = tmp->next_Student;
       }
       tmp->next_Student = newNode;
    }
    newNode->next_Student = NULL;
}

// Removes a student from stop B (a node in stop B queue)
void stopB_student_leave(Student *student){
    if(stopB->head == NULL){
        return;
    }

    // Search for the student in the queue
    stop_Node *tmp = stopB->head;
    while(tmp != NULL && tmp->student_Node != student){
        tmp = tmp->next_Student;
    }

    // Remove student from the queue
    if(tmp != NULL){
        if(stopB->head == tmp){
            if(stopB->head->next_Student == NULL){
                stopB->head = NULL;
            }
            else{
                stopB->head = stopB->head->next_Student;
            }
            tmp->next_Student = NULL;
        }
        else if(tmp->next_Student == NULL){
            stop_Node *tmp2 = stopB->head;
            while(tmp2->next_Student != tmp){
                tmp2 = tmp2->next_Student;
            }
            tmp2->next_Student = NULL;
        }
        else{
            stop_Node *tmp2 = stopB->head;
            while(tmp2->next_Student != tmp){
                tmp2 = tmp2->next_Student;
            }
            tmp2->next_Student = tmp->next_Student;
            tmp->next_Student = NULL;
        }
    }
}

// Prints the current status of stop B
void stopB_print(){
    stop_Node *ptr = stopB->head;
    printf("StopB: ");
    if(ptr!=NULL){
        while(ptr!=NULL){
            printf("[%d, %s] ", ptr->student_Node->AM, int_to_str(ptr->student_Node->Department));
            ptr = ptr->next_Student;
        }
    }
    printf("\n");
}

// Initialization of University
void init_university(){
    if(sem_init(&uni_sem, 0, 200) != 0){ // Initialize the semaphore for university capacity
        fprintf(stderr, "Semaphore initialization error");                                       
        exit(-1); 
    }
    if(pthread_mutex_init(&enter_uni, NULL) != 0){ // Initialize the mutex for entering in university
        fprintf(stderr, "Mutex initialization error\n");
        exit(-1);
    }
    if(pthread_mutex_init(&leave_uni, NULL) != 0){ // Initialize the mutex for leaving from university
        fprintf(stderr, "Mutex initialization error\n");
        exit(-1);
    }
    for(int i=0; i<200; i++){
        if(pthread_mutex_init(&University[i], NULL)!=0){                                  
            fprintf(stderr, "University mutex initialization error");                                       
            exit(-1);                                                                    
        }
    }
    UniList = malloc(sizeof(Uni_Node));
    UniList->head = NULL;
}

// Study simulation
void uni_study(Student *student){
    for(int i=0; i<200; i++){
        
        // If the mutex is locked, the thread doesn't block until unlock and searches for other location
        int trylock = pthread_mutex_trylock(&University[i]);
        if(trylock==0){ // if thread succeded to lock
            sleep(student->study_time);                           
            if(pthread_mutex_unlock(&University[i]) != 0){
                fprintf(stderr, "Mutex unlock error\n");
                exit(-1);
            }
            break;
        }
        else if(trylock != EBUSY){ // if mutex trylock returned other value than EBUSY or 0, means there is unexpected problem
            fprintf(stderr, "Mutex trylock error\n");
            exit(-1);
        }

        if(i==199){
            i=-1;
        }
    }
    printf("Student %d (%s) studied for %d seconds and now is heading to stop B\n\n", student->AM, int_to_str(student->Department), student->study_time);
}

// Adds a student into University
void uni_enter(Student *student){
    Uni_Node *newNode = malloc(sizeof(Uni_Node));
    newNode->student_Node = student;
    if(UniList->head == NULL){
        UniList->head = newNode;
    }
    else{
       Uni_Node *tmp = UniList->head;
       while(tmp->next_Student!=NULL){
            tmp = tmp->next_Student;
       }
       tmp->next_Student = newNode;
    }
    newNode->next_Student = NULL;
}

// Removes a student from University
void uni_leave(Student *student){
    if(UniList->head == NULL){
        return;
    }

    Uni_Node *tmp = UniList->head;

    // Search for the student in the queue
    while(tmp != NULL && tmp->student_Node != student){
        tmp = tmp->next_Student;
    }

    // Remove the student from queue
    if(tmp != NULL){
        if(tmp == UniList->head){
            if(UniList->head->next_Student == NULL){
                UniList->head = NULL;
            }
            else{
                UniList->head = UniList->head->next_Student;
            }
            tmp->next_Student = NULL;
        }
        else if(tmp->next_Student == NULL){
            Uni_Node *tmp2 = UniList->head;
            while(tmp2->next_Student != tmp){
                tmp2 = tmp2->next_Student;
            }
            tmp2->next_Student = NULL;
        }
        else{
            Uni_Node *tmp2 = UniList->head;
            while(tmp2->next_Student != tmp){
                tmp2 = tmp2->next_Student;
            }
            tmp2->next_Student = tmp->next_Student;
            tmp->next_Student = NULL;
        }
    }
}

// Prints the current status of university
void uni_print(){
    Uni_Node *ptr = UniList->head;
    printf("University: ");
    if(ptr!=NULL){
        while(ptr!=NULL){
            printf("[%d, %s] ", ptr->student_Node->AM, int_to_str(ptr->student_Node->Department));
            ptr = ptr->next_Student;
        }
    }
    printf("\n");
}

// Prints the current status of, stop A, stop B, bus stop of university and university
void print_all(){
    stopA_print();
    bus_print();
    uni_print();
    stopB_print();
    printf("\n");
}