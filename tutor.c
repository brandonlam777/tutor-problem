#define MAX 20
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void *student(void *param);
void *tutor(void *param);
void pthread_exit(void *status);

sem_t chairs_mutex;
sem_t sem_student;
sem_t sem_tutor;
int num_chairs;
int max_chairs;
int helpTimes;
int tutors;
int students;
int studentsHelped = 0;
int tutorsLeft = 0;
pthread_t tutorids[MAX];
pthread_t studentids[MAX];
int main(int argc, char *argv[]) {
	printf("Main thread beginning\n");
   // Get command line arguments 
   int i;
   if (argc != 5){
	   printf("Please enter 4 arguments: <Number of Students> <Number of tutors>\n");
	   printf("<Number of chairs> <Number of times students looks for help>\n");
	   exit(0);
   } 
   students = atoi(argv[1]);
   tutors = atoi(argv[2]);
   num_chairs = atoi(argv[3]);
   helpTimes = atoi(argv[4]);
   max_chairs = num_chairs;
   printf("Number of students is: %u\n", students);
   printf("Number of tutors is: %u\n", tutors);
   printf("Number of chairs is: %u\n", num_chairs);
   //  Initialize semaphores 
   sem_init(&chairs_mutex,0,1);
   sem_init(&sem_student,0,0);
   sem_init(&sem_tutor,0,0);
   //  Create tutor threads
   for (i = 0; i < tutors; i++){
	   pthread_create(&tutorids[i], NULL, tutor, NULL);
	   printf("Creating tutor thread with id %lu\n",tutorids[i]);
   }
   //  Create student threads
   for (i = 0; i < students; i++){
	   pthread_create(&studentids[i], NULL, student, NULL);
	   printf("Creating student thread with id %lu\n",studentids[i]);
   }
   
   //Check if any students are left, if not terminate tutor threads
   while(1)
   {
	   if(studentsHelped == students)
	   {
		   for(i=0; i < tutors; i++)
		   {
			   printf("Tutor %u: Leaving\n",i);
			   pthread_cancel(tutorids[i]);
		   }
	   break;
	   }
   }
   // Exit.
   printf("Main thread exiting\n");
   exit(0);
}

void *tutor(void *param) {
   int worktime;
   int tutorN;
   int i;
   int waitingStudents;
   for(i = 0; i < tutors; i++)
   {
	   if((unsigned int)pthread_self() == (unsigned int)tutorids[i])
	   {
		   tutorN = i + 1;
	   }
   }
   while(1) {
      // wait for a student to become available (sem_student)
	  sem_wait(&sem_student);
      // wait for mutex to access chair count (chair_mutex)
	  sem_wait(&chairs_mutex);
      // increment number of chairs available
	  num_chairs += 1;
	  waitingStudents = max_chairs - num_chairs;
	  printf("Tutor %u: Taking a student. Waiting Students = %d\n",tutorN, waitingStudents);
      // signal to student that tutor is ready (sem_tutor) 
	  sem_post(&sem_tutor);
      // give up lock on chair count 
	  sem_post(&chairs_mutex);
      // generate random number, worktime, from 1-3 seconds for length of haircut.  
	  worktime = (rand() % 3) + 1;
      // sleep 
	  printf("Tutor %u: helping student for %d seconds\n",tutorN, worktime);
	  sleep(worktime);
    } 
	return(0);
}

void *student(void *param) {
    int waittime;
	int studentN;
	int i;
	int timesHelped = 0;
	int waitingStudents;
	int ret = 100;
	sleep(1);
	for(i = 0; i < students; i++)
    {
	   if((unsigned int)pthread_self() == (unsigned int)studentids[i])
	   {
		   studentN = (i + 1);
	   }
    }
   while(1) {
	  
      // wait for mutex to access chair count
	  sem_wait(&chairs_mutex);
      // if there are no chairs 
	  if(num_chairs <= 0){
           //free mutex lock on chair count 
		   printf("Student %u: found no empty chair will try again later\n", studentN);
		   sem_post(&chairs_mutex);
	  }
      // else if there are chairs 
	  else{
           //decrement number of chairs available 
		   num_chairs -= 1;
		   waitingStudents = max_chairs - num_chairs;
		   printf("Student %u: takes a seat. Waiting students = %d\n",studentN,waitingStudents);
           // signal that a student is ready
		   sem_post(&sem_student);
           // free mutex lock on chair count
		   sem_post(&chairs_mutex);
           // wait for tutor (sem_tutor)
		   sem_wait(&sem_tutor);
           // get help
		   printf("Student %u: getting help\n",studentN);
		   timesHelped++;
	  }
	  if(timesHelped == helpTimes) //terminate student thread if times helped reached
	  {
		  studentsHelped++;
		  printf("Student %u: Leaving\n",studentN);
		  pthread_exit(&ret);
		  break;
	  }
	  else
	  {
		// generate random number, waittime, for length of wait.
		waittime = (rand() % 5) + 1;
		//sleep for waittime seconds 
		printf("Student %u: waiting %d seconds before looking for help again\n",studentN,waittime);
		sleep(waittime);
	  }
	  
     }
	 return(0);
}
