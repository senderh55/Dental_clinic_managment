# Dental_clinic_managment
Dental_clinic_managment - An assignment for operating systems course.

The goal of the program is to use semaphores for efficient queue management without deadlocks.
Each client and dentist represents wires that activate functions relevant to them.
The program runs in infinity loop while The queues are implemented through linked lists.

## The clinic structure
* There are 3 Dental Hygienist.
* There is a wating room for 10 patients that include sofa where only four of the waiting patients can sit on.
* Every patient who sits on the couch will wait there until his turn.
* The first patient to enter is the one who will be first in line.
* A dental hygienist can treat only one patient
* Only after making a payment is the patient discharged

### Output example

![image](https://user-images.githubusercontent.com/79198595/179356434-b5201102-2b76-43af-8171-8767a8c249e8.png)
