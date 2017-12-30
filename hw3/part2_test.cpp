#include <unistd.h>
#include "ReceptionHour.h"
#include "tests_macros.h"

#define MAX_STUDENTS 2
pthread_mutex_t mutex;
unsigned int next_id;

void sleep_print_time(int sec) {
    printf("       %d sec of sleep\n",sec);
    for (int i = 1; i <= sec; i++) {
        sleep(1);
        printf("       seconed passed: %d out of: %d\n",i, sec);
    }

}
bool TA_exit_after_door_close_zero_students_TEST() {
    ReceptionHour rh = ReceptionHour(MAX_STUDENTS);
    rh.closeTheDoor();
    return true;
}
bool TA_exit_after_door_close_zero_students_TEST2() {
    /*Same as before but now we wait before closing the door*/
    ReceptionHour rh = ReceptionHour(MAX_STUDENTS);
    printf("\n       Sleeping for 5 sec\n");
    sleep_print_time(5);
    rh.closeTheDoor();
    return true;
}

bool exiting_with_no_finish_student_TEST() {
    ReceptionHour rh = ReceptionHour(MAX_STUDENTS);

    rh.startStudent(1);
    sleep_print_time(15);
    rh.closeTheDoor();
    if (rh.finishStudent(1) != ENTERED) return false;
    return true;
}
bool moreThenMaxStudent() {
    ReceptionHour rh = ReceptionHour(MAX_STUDENTS);
    printf("\n       Creating %d Students and the max number of students is: %d\n", (MAX_STUDENTS + 1), MAX_STUDENTS );
    int num_of_students = 3*MAX_STUDENTS + 0;
    for (int i = 0; i < num_of_students; i++) {
        rh.startStudent(i);
    }
    printf("       Letting students to be created: start sleep for 5 sec\n");
    sleep_print_time(5);
    printf("       Finishing all students if this stack for along time its a problem\n");
    bool result = true;
    int num_of_students_not_enterd = 0;
    int num_of_students_enterd = 0;
    for (int i = 0; i < num_of_students; i++) {
        StudentStatus studentResult = rh.finishStudent(i);
        printf("student %d status is:%d\n",i,studentResult);
        switch(studentResult) {
            case LEFT_BECAUSE_DOOR_CLOSED: {
                result = false;
                printf("       Test failed: a student left because the door was closed and the test didn't close the door yes.\n");
                break;
            }
            case LEFT_BECAUSE_NO_SEAT:
                num_of_students_not_enterd++;
                break;
            case ENTERED:
                num_of_students_enterd++;
                break;
            default: {
                result = false;
                printf("       Test failed Illegal studentStatus = %d \n",studentResult);
            }
        }
    }
    if (num_of_students_not_enterd == 0) {
        printf("       Test failed: %d student didn't entered\n", num_of_students_not_enterd);
        result = false;
    }
    printf("       Letting all students to be finished before door close: start sleep for 5 sec\n");
    sleep_print_time(5);
    rh.closeTheDoor();
    return result;
}
bool startStudentBeforeTA() {
    printf("Make sure you have a long sleep at the beginning of the function that start the TA\n");
    ReceptionHour rh = ReceptionHour(MAX_STUDENTS);
    rh.startStudent(1);

    if (rh.finishStudent(1) != ENTERED) return false;
    sleep_print_time(20);
    rh.closeTheDoor();
    return true;
}
bool start10StudentBeforeTA() {
    printf("Make sure you have a long sleep at the beginning of the function that start the TA\n");
    ReceptionHour rh = ReceptionHour(MAX_STUDENTS);
    for (int i = 0; i < 10; i++) {
        rh.startStudent(i);
    }
    sleep_print_time(30);
    rh.closeTheDoor();
    for (int i = 0; i < 10; i++) {
        rh.finishStudent(i);
    }

    return true;
}
void* start_students_function(void* reception_hour) {
    ReceptionHour* rh = (ReceptionHour*)reception_hour;
    int num_students = rand() % 10;
    unsigned int ids[num_students];
    unsigned int id;
    for (int i = 0; i < num_students; i ++) {
        if (rand() % 5 == 1) {
            sleep_print_time(15);
        }
        pthread_mutex_lock(&mutex);
        id = next_id;
        next_id++;
        pthread_mutex_unlock(&mutex);
        ids[i] = id;
        printf("       Creating student with id = %d\n", id);
        rh->startStudent(id);
    }
    for (int i = 0; i < num_students; i ++) {
        printf("       finishStudent id = %d\n", ids[i]);
        StudentStatus res = rh->finishStudent(ids[i]);
        if (res < 0 || res > 2) {
            printf("       Test failed res = %d\n", res);
        }
    }
    return 0;
}
void* close_door_thread_func(void* reception_hour) {
    ReceptionHour* rh = (ReceptionHour*)reception_hour;
    while(rand() % 5 != 0) {
        printf("       Didn't close the door\n");
        sleep_print_time(3);
    };
    rh->closeTheDoor();
    return 0;
}
bool Test_try_to_create_deadlock() {
    int num_threads = 20;
    unsigned int num_students_in_class = 5;
    next_id = 0;
    pthread_t *threads = new pthread_t [num_threads];
    ReceptionHour rh = ReceptionHour(num_students_in_class);
    for(int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, start_students_function, &rh);
    }
    pthread_t close_door_thread;
    pthread_create(&close_door_thread ,NULL, close_door_thread_func, &rh);
    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_join(close_door_thread, NULL);
    delete threads;
    return true;
}
int main() {
    printf("Part2 Test\n");
    RUN_TEST(TA_exit_after_door_close_zero_students_TEST);
    RUN_TEST(TA_exit_after_door_close_zero_students_TEST2);
    RUN_TEST(exiting_with_no_finish_student_TEST);
    RUN_TEST(moreThenMaxStudent);
    RUN_TEST(startStudentBeforeTA);
    RUN_TEST(start10StudentBeforeTA);
    RUN_TEST(Test_try_to_create_deadlock);
}
