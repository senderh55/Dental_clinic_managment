/* this program represent a daily of managing dentist clinic.*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>

#define N 10
#define NUM 3
#define SOFANUM 4

sem_t dentalHygienist, patients, patientPay, mutex, patientOutOfClinic,
		patientInTreat, dGetPaid,waitForPay;
int numOfpatients = 0, sofaPlace = 0;

//define linked list that will represent the the patients that sitting on the sofa
typedef struct node {
	int val;
	struct node * next;
} sofa;

sofa*head = NULL;

void Error(char*msg);
void*patientsThread(void*ptr2);
void * dentalHygienistThread(void*ptr);
int isSofaEmtyp(sofa*val);
void freeAlocation(sofa*head);
void setPatientOnTheSofa(sofa*head, int val);
int getUpFromSofa(sofa**head);

int main() {
	int i, index1[N + 2], index2[NUM], ans1, ans2;

	sem_init(&patientPay, 0, 0);//init the patient payment semaphore
	sem_init(&patients, 0, 4);//init the patient semaphore that represent the patient the sit on the sofa
	sem_init(&mutex, 0, 1);//init mutex semaphore for critical code action
	sem_init(&patientOutOfClinic, 0, 0);//init semaphore for the patient that is waiting outside the clinic
	sem_init(&dentalHygienist, 0, 0);//init the dental Hygienist
	sem_init(&patientInTreat, 0, 3);//init the patient in treatment semaphore it will represent max number of patient that can get treatment
	sem_init(&dGetPaid, 0, 0);//init dental Hygienist semaphore it represent the dental Hygienist waitnit for patinet will pay for his treatment
	sem_init(&waitForPay,0,0);//init waitForPay semaphore it represent the patient waiting for dental Hygienist confirm his payment
	pthread_t dentalHygienistThreads[NUM], patientsThreads[N + 2];

	//init patients threads
	for (i = 0; i < N + 2; i++) {
		index1[i] = i;
		ans1 = pthread_create(&patientsThreads[i], NULL, patientsThread,
				(void*) &index1[i]);
		if (ans1 != 0) {
			Error("thread create failed");
		}
	}
	//init dentalHygienis threads
	for (i = 0; i < NUM; i++) {
		index2[i] = i;
		ans2 = pthread_create(&dentalHygienistThreads[i], NULL,
				dentalHygienistThread, (void*) &index2[i]);
		if (ans2 != 0) {
			Error("thread create failed");
		}
	}

	//main thread wait for all child thread finish run
	for (i = 0; i < N + 2; i++) {
		pthread_join(patientsThreads[i], NULL);
	}
	for (i = 0; i < NUM; i++) {
		pthread_join(dentalHygienistThreads[i], NULL);
	}

	return 1;

}

//this function check if the list of patients that setting on the sofa is empty
int isSofaEmtyp(sofa*val) {

	if (val == NULL) {
		return 1;
	}

	return 0;
}
//function to free allocation of the list
void freeAlocation(sofa*head) {
	sofa*temp = head;
	while (temp->next != NULL) {
		head = head->next;
		free(temp);
		temp = head;

	}
}
//this function is sitting on the end of the sofa the next patient in the queue in the clinic
void setPatientOnTheSofa(sofa*head, int val) {
	sofa*current = head;
	while (current->next != NULL) {
		current = current->next;
	}
	//find the open spot on the sofa
	current->next = (sofa*) malloc(sizeof(sofa));
	if (current->next == NULL) {
		freeAlocation(head);
		Error("memory allocation failed");
	}
	current->next->next = NULL;
	current->next->val = val;

}
//this function represent the getting up from the sofa for getting a treatment
int getUpFromSofa(sofa**head) {
	int retval = -1;
	sofa * next_node = NULL;

	if (*head == NULL) {
		return -1;
	}

	next_node = (*head)->next;
	retval = (*head)->val;
	free(*head);
	*head = next_node;

	return retval;
}
//this function represent the patient thread
//the patient get in the clinic only if there is place in the clinic
//other wise he will wait out side the clinic the queue
void*patientsThread(void*ptr2) {
	int patientNum = *(int*) ptr2;
	int patientAfterTreat = -1;

	while (1) {
		//critical code update the number of patient in the clinic
		sem_wait(&mutex);
		
		//check if there is more place in the clinic for the current patient
		if (numOfpatients > N) {
			fprintf(stdout, "I'm Patient #%d, I'm out of clinic\n", patientNum+1);
			sem_post(&mutex);//finish critical code
			sem_wait(&patientOutOfClinic);

		} else {

			numOfpatients++;
			fprintf(stdout, "I'm Patient #%d, I got into the clinic\n",
					patientNum+1);
			sem_post(&mutex);//finish critical code
			sem_wait(&patients);
			//critical code update the number of patient in the clinic
			sem_wait(&mutex);
			//check if the there is a place on the sofa


				if (isSofaEmtyp(head) == 1) {
					head = (sofa*) malloc(sizeof(sofa));
					//check if the sofa allocation failed
					if (head == NULL) {
						Error("memory allocation failed");
					}

					head->val = patientNum;
					head->next = NULL;
					fprintf(stdout,
							"I'm Patient #%d, I'm sitting on the sofa\n",
							patientNum+1);


				} else {
					setPatientOnTheSofa(head, patientNum);
					fprintf(stdout,
							"I'm Patient #%d, I'm sitting on the sofa\n",
							patientNum+1);
				}
				sem_post(&mutex);

			//patient wait form treatment of there an dentalHygienist that can treat him
			sem_wait(&patientInTreat);
			//patient enter to treatment
			sem_wait(&mutex);
			patientAfterTreat = getUpFromSofa(&head);
			fprintf(stdout, "I'm Patient #%d, I'm getting treatment\n",
					patientAfterTreat+1);
			

			sem_post(&patients);
			sem_post(&mutex);
			sem_post(&dentalHygienist); //set signal to dentalHygienist the she have a patient
			sem_wait(&patientPay); //patient wait for dentalHygienist to finish the treatment
			fprintf(stdout, "I'm Patient #%d, I'm paying now\n",
					patientAfterTreat+1);
			
			sem_post(&dGetPaid);
			sem_wait(&waitForPay);
			
			
			}


	}

}
//this function represent the dental Hygienist it will start work only if there is
//patients on the sofa
void * dentalHygienistThread(void*ptr) {
	int num = *(int*) ptr;
	while (1) {
		sem_wait(&dentalHygienist);//dental Hygienist  wait for patient
		fprintf(stdout, "I'm Dental Hygienist #%d, I'm working now\n", num+1);
		
		//dental Hygienist finish the treatment and wait for patient to pay
		sem_wait(&mutex);
		sem_post(&patientPay);
		sem_wait(&dGetPaid);
		fprintf(stdout, "I'm Dental Hygienist #%d, I'm getting a payment\n",
				num+1);
		numOfpatients--;
		if(numOfpatients<N){
		sem_post(&patientOutOfClinic); //patient paid for treatment and leave the clinic, we can enter new patient to the clinic
		}
		sem_post(&waitForPay);
		sem_post(&patientInTreat);
		sem_post(&mutex);
		
		

	}

}

//this function prints the error message and terminate the all program
void Error(char*msg) {
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

