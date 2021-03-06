#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <pthread.h>


#define  NOT_READY  -1
#define  FILLED     0
#define  TAKEN      1

struct Memory {
     int  status;
     int  data[4];
     char a;
};
void *deneme(void *arg)
{
     key_t          ShmKEY;
     int            ShmID=-1;
     struct Memory  *ShmPTR;

     ShmKEY = ftok(".", 'q');
         // fprintf(stderr,"%d",(int)ShmKEY);

    while (ShmID < 0) 
     	     ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
     
     printf("   Client has received a shared memory of four integers...\n");

     ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
     if (ShmPTR == (struct Memory *)(-1)) {
          printf("*** shmat error (client) ***\n");
          exit(1);
     }
     printf("   Client has attached the shared memory...\n");

    // while (ShmPTR->status != FILLED) ;
     printf("   Client found the data is ready...\n");
     printf("   Client found %d %d %d %d in shared memory...\n",
                ShmPTR->data[0], ShmPTR->data[1], 
                ShmPTR->data[2], ShmPTR->data[3]);

    // ShmPTR->status = TAKEN;
     printf("   Client has informed server data have been taken...\n");
     shmdt((void *) ShmPTR);
     printf("   Client has detached its shared memory...\n");
     printf("   Client exits...\n");

}
void  main(void)
{
	pthread_t a,b,c;
	     key_t          ShmKEY;
     int            ShmID=-1;
     struct Memory  *ShmPTR;

     ShmKEY = ftok(".", 'q');
         // fprintf(stderr,"%d",(int)ShmKEY);

    while (ShmID < 0) 
     	     ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
     
     printf("   Client has received a shared memory of four integers...\n");

     ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
     if (ShmPTR == (struct Memory *)(-1)) {
          printf("*** shmat error (client) ***\n");
          exit(1);
     }
     printf("   Client has attached the shared memory...\n");

    // while (ShmPTR->status != FILLED) ;
     printf("   Client found the data is ready...\n");
     printf("   Client found %d %d %d %d in shared memory...\n",
                ShmPTR->data[0], ShmPTR->data[1], 
                ShmPTR->data[2], ShmPTR->data[3]);

    // ShmPTR->status = TAKEN;
     printf("   Client has informed server data have been taken...\n");
     shmdt((void *) ShmPTR);
     printf("   Client has detached its shared memory...\n");
     printf("   Client exits...\n");
	pthread_create(&a, NULL, deneme, NULL);
	pthread_create(&b, NULL, deneme, NULL);
	pthread_create(&c, NULL, deneme, NULL);


	pthread_join(a,NULL);
	pthread_join(b,NULL);
	pthread_join(c,NULL);
     exit(0);
}
