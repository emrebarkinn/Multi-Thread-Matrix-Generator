#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

int SIZE;
int **matrix=NULL;

//generete_function global variables
int genf_cond=0;
int logf_cond=0;
int modf_cond=0;

int global_sum=0;

pthread_mutex_t global_sum_mutex;


pthread_mutex_t gen_mutex1;
pthread_mutex_t gen_mutex2;

pthread_mutex_t log_mutex1;
pthread_mutex_t log_mutex2;

pthread_mutex_t mod_mutex1;
pthread_mutex_t mod_mutex2;

pthread_mutex_t sum_mutex1;


void calculate_mod(int in_matrix[5][5], int new_matrix[5][5]);

struct inner_matrix{  //we create struct for inner matrix because in this way it more easy for copy or pass by referance;
  int in_matrix[5][5];
};



struct inner_matrix* create_inner_matrix(){
  struct inner_matrix* temp=(struct inner_matrix*)malloc(sizeof(struct inner_matrix));

  for (size_t j = 0; j < 5; j++) {
    for (size_t i = 0; i < 5; i++) {
      if(i==0&&j==0){
        temp->in_matrix[j][i]=(int) rand()%99 +1; //first element can not equal to 0 because
      }else                                       //it will cause floating point error (when we use it for mod calculations)
      {
        temp->in_matrix[j][i]=(int) rand()%100;
      }

    }
  }


  return temp;

}
struct inner_matrix* create_mod_inner_matrix(int in_matrix[5][5]){
  struct inner_matrix* temp=(struct inner_matrix*)malloc(sizeof(struct inner_matrix));

  calculate_mod(in_matrix,temp->in_matrix);//generates mode for all elements from given matrix(in _matrix)
  return temp; //returns new created matrix struct.

}
struct QNode
{
    int id;
    struct inner_matrix* node;
    int gen_status;
    int log_status;
    int mod_status;
    int sum_status;
    struct QNode *next;
};
struct Queue
{
    struct QNode *front, *rear;
};
struct QNode* newNode(int k)
{
    struct QNode *temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->id=k;
    temp->node=NULL;
    temp->next = NULL;
    temp->gen_status =0; // f1 means its created
    temp->log_status=0;
    temp->sum_status=0;

    return temp;
}
struct Queue *createQueue()
{
    struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}


struct Queue *queue1=NULL;              /////////////////////////////////////////////////////////////////7777777777
struct Queue *queue2=NULL;


void enQueue(struct Queue *q, int k)
{
    // Create a new LL node
    struct QNode *temp = newNode(k);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
       q->front = q->rear = temp;
       return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove a key from given queue q
struct QNode *deQueue(struct Queue *q)
{
    if (q->front == NULL)
       return NULL;

    // Store previous front and move front one node ahead
    struct QNode *temp = q->front;
    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
       q->rear = NULL;
    return temp;
}


void initialize_matrix(){
  matrix = (int **)malloc(sizeof(int)*SIZE*SIZE);
  for (size_t j = 0; j < SIZE; j++) {
    matrix[j]=(int *)malloc(sizeof(int)*SIZE);
  }

}
void print_matrix(){
  for (size_t j = 0; j < SIZE; j++) {
    if(j!=0 && j%5==0)
      printf("\n");
    for (size_t i = 0; i < SIZE; i++) {
      if(i!=0 && i%5==0)
        printf("  ");
      printf("%3d",matrix[j][i]);

    }
    printf("\n");

  }
}
void print_in_matrix(char output[200],int in_matrix[5][5]){
  char temp_s[200];
  temp_s[0]='\0';
  strcpy(output,temp_s);
  for (size_t j = 0; j < 5; j++) {
    for (size_t i = 0; i < 5; i++) {
      sprintf(temp_s,"%3d",in_matrix[j][i]);
      strcat(output,temp_s);
    }
    sprintf(temp_s,"\n");
    strcat(output,temp_s);
  }
}
int log_into_matrix(int loc, int in_matrix[5][5]){

  int y_start=(loc/(SIZE/5))*5;
  int x_start=(loc%(SIZE/5))*5;


  for (size_t j = y_start; j < y_start+5; j++) {
    for (size_t i = x_start; i < x_start+5; i++) {
      matrix[j][i]=in_matrix[j-y_start][i-x_start];
    }
  }

  return 1;
}
void calculate_mod(int in_matrix[5][5], int new_matrix[5][5]){

  for (size_t j=0 ; j < 5; j++) {
    for (size_t i=0 ; i < 5; i++) {
      new_matrix[j][i]=in_matrix[j][i]%in_matrix[0][0];
    }
  }

}
int calculate_sum(int in_matrix[5][5]){
  int sum=0;
  for (size_t j=0 ; j < 5; j++) {
    for (size_t i=0 ; i < 5; i++) {
      sum += in_matrix[j][i];
    }
  }
  return sum;

}
void* generete_function(void* args){
  char output[200];
  char temp_string[200];
  output[0]='\0';
  temp_string[0]='\0';
  int count=0;
  int thread_id=*((int*)args);

  if(genf_cond==0){//we create this global cond for checking queue is created or not
    //mutex1
    pthread_mutex_lock(&gen_mutex1);
    if(queue1 == NULL){
      queue1=createQueue();
      for (size_t i = 0; i<(SIZE/5)*(SIZE/5); i++)
        enQueue(queue1,i);

      genf_cond=1;//that means queue is created.
    }
    pthread_mutex_unlock(&gen_mutex1);
    //close mutex1
  }
  struct QNode* temp;
  int cond_temp=0;
  while(count!=(SIZE/5)*(SIZE/5)){

//in this way more thread will be in creating session .


        temp=queue1->front;
        count=0;

        while(temp !=NULL){
          //mutex2


          pthread_mutex_lock(&gen_mutex2);
          if(temp->gen_status==0){//when we find a node which need to be created (inner_matrix)
            temp->gen_status=1;//we mark that node as in generating state so other threads can not try to generate that matrix
            cond_temp=1;//with this local variable thread will generate matrix outside of mutex range
          }
          //mutex2
          pthread_mutex_unlock(&gen_mutex2);


          if(cond_temp==1){
            temp->node=create_inner_matrix();
            temp->gen_status=2;// this means the inner matrix is created.

            //for preventing conflicts in printing outputs
            // we firstly write them into single string and we print that string only.

            sprintf(output,"Generator_%d generated following matrix\n", thread_id);
            print_in_matrix(temp_string,temp->node->in_matrix);
            strcat(output,temp_string);
            sprintf(temp_string,"This matrix is [%d,%d] submatrix\n",(temp->id/(SIZE/5)),(temp->id%(SIZE/5))  );
            strcat(output,temp_string);
            printf("%s\n",output );

            cond_temp=0;
          }
          if(temp->gen_status > 0)
            count ++;

          if(count==(SIZE/2)*(SIZE/2)){ //if count equal to total inner matrix size this thread work is done.
            break;
          }
          temp=temp->next;
        }


  }
  //

  pthread_exit((void*)0);
}
void* log_function(void* args){
  char output[200];
  char temp_string[200];
  output[0]='\0';
  temp_string[0]='\0';

  while(genf_cond==0){ //if queue is not created it will wait until it is created

  }
  if(logf_cond==0){
    //mutex_log_1
    pthread_mutex_lock(&log_mutex1);
    if(matrix==NULL){
      initialize_matrix();//creates main matrix
      logf_cond=1;//preventing threads to enter this mutex again
    }
    pthread_mutex_unlock(&log_mutex1);
    //mutex_log_1
  }

  int count=0;

  int thread_id=*((int*)args);
  struct QNode* temp;
  int cond_temp=0;
  while(count!=(SIZE/5)*(SIZE/5)){




        temp=queue1->front;

        count=0;
        while(temp !=NULL){
          //mutex_log_2
          pthread_mutex_lock(&log_mutex2);
          if(temp->gen_status==2){//we can only log a matris if it is in created state
            temp->gen_status=3;//this state means it created and logged.
            cond_temp=1;
          }
          pthread_mutex_unlock(&log_mutex2);
          //mutex_log_2


          if(cond_temp==1){

            log_into_matrix(temp->id,temp->node->in_matrix);

            //for preventing conflicts in printing outputs
            // we firstly write them into single string and we print that string only.

            sprintf(output,"Log_%d put following submatrix into bigger matrix\n", thread_id);
            print_in_matrix(temp_string,temp->node->in_matrix);
            strcat(output,temp_string);
            sprintf(temp_string,"This matrix is [%d,%d] submatrix\n",(temp->id/(SIZE/5)),(temp->id%(SIZE/5)) );
            strcat(output,temp_string);
            printf("%s\n",output );

            cond_temp=0;
          }

          if(temp->gen_status==3)//we count matrices which are created.
            count++;
          if(count==(SIZE/2)*(SIZE/2)){//if count equal to total inner matrix number thread will exit the loop
            break;
          }


          temp=temp->next;
        }


  }

  pthread_exit((void*)0);

}
void* mod_function(void* args){
  char output[200];
  char temp_string[200];
  output[0]='\0';
  temp_string[0]='\0';
  int count=0;
 int thread_id=*((int*)args);
 while(genf_cond==0){ //it waits for queue1 to be created.

 }
  if(modf_cond==0){ //it will create queue2 if it is not created.
    //mutex1
    pthread_mutex_lock(&mod_mutex1);
    if(queue2 == NULL){
      queue2=createQueue();
      for (size_t i = 0; i<(SIZE/5)*(SIZE/5); i++)
        enQueue(queue2,i);//it add all elements into queue but their inside
      modf_cond=1;
    }
    pthread_mutex_unlock(&mod_mutex1);
    //close mutex1
  }
  struct QNode* temp;
  struct QNode* temp2;
  int cond_temp=0;

  while(count!=(SIZE/5)*(SIZE/5)){

        temp=queue1->front;
        temp2=queue2->front;
        count=0;

        while(temp !=NULL){
          //mutex2
          pthread_mutex_lock(&mod_mutex2);
          if(temp->gen_status>=2&&temp->mod_status==0){
            temp->mod_status=1;
            cond_temp=1;
          }
          //mutex2
          pthread_mutex_unlock(&mod_mutex2);


          if(cond_temp==1){

            while(temp2!=NULL){
              if(temp2->id==temp->id)
                break;

              temp2=temp2->next;
            }
            if(temp2!=NULL){
              temp2->node=create_mod_inner_matrix(temp->node->in_matrix);
              temp->mod_status=2;
              temp2->mod_status=2;
              log_into_matrix(temp->id,temp->node->in_matrix);

              sprintf(output,"Mod_%d genereted following submatrix into bigger matrix\n", thread_id);
              print_in_matrix(temp_string,temp2->node->in_matrix);
              strcat(output,temp_string);
              sprintf(temp_string,"From  [%d,%d] submatrix\n",(temp->id/(SIZE/5)),(temp->id%(SIZE/5))  );
              strcat(output,temp_string);
              printf("%s\n",output );

            }

            cond_temp=0;
          }
          if(temp->mod_status > 0)
            count ++;

          if(count==(SIZE/2)*(SIZE/2)){
            break;
          }
          temp=temp->next;
        }


  }
  //

  pthread_exit((void*)0);


}
void* sum_function(void* args){
  char output[200];
  char temp_string[200];
  output[0]='\0';
  temp_string[0]='\0';
  int local_sum=0;
  int count=0;
 int thread_id=*((int*)args);
 while(modf_cond==0){

 }
  struct QNode* temp;
  int cond_temp=0;
  while(count!=(SIZE/5)*(SIZE/5)){

        temp=queue2->front;

        count=0;

        while(temp !=NULL){

          pthread_mutex_lock(&sum_mutex1);
          if(temp->mod_status==2&&temp->sum_status==0){
            temp->sum_status=1;
            cond_temp=1;
          }
          pthread_mutex_unlock(&sum_mutex1);


          if(cond_temp==1){


            temp->sum_status=2;
            local_sum=calculate_sum(temp->node->in_matrix);
            pthread_mutex_lock(&global_sum_mutex);
            int temp_global_sum=global_sum;
            global_sum+=local_sum;
            printf("Add_%d has local sum :%d by [%d,%d] submatrix, global sum before/after update:%d/%d\n", thread_id,local_sum,(temp->id/(SIZE/5)),(temp->id%(SIZE/5)),temp_global_sum,global_sum);

            //sprintf(output,"Add_%d has local sum :%d by [%d,%d] submatrix, global sum before/after update:%d/%d\n", thread_id,local_sum,(temp->id%(SIZE/5)),(temp->id/(SIZE/5)),temp_global_sum,global_sum);
            pthread_mutex_unlock(&global_sum_mutex);
            //printf("%s\n",output );

            cond_temp=0;
          }
          if(temp->sum_status > 0)
            count ++;

          if(count==(SIZE/2)*(SIZE/2)){
            break;
          }
          temp=temp->next;
        }


  }
  pthread_exit((void*)0);


}
int main(int argc, char *argv[]){


  if(argc!=8){
    perror("Invalid number of argument ");
  }
  printf("%d",0%2);
  //(int) strtol(str, (char **)NULL, 10)
  SIZE=atoi(argv[2]);
  if(SIZE%5 !=0){
    perror("Please enter a size which is divisible by 5");
    return 1;
  }
  int gen_thread_size=atoi(argv[4]);
  //initialize_matrix();  //creates bigger marix
  int log_thread_size=atoi(argv[5]);
  int mod_thread_size=atoi(argv[6]);
  int sum_thread_size=atoi(argv[7]);

  pthread_mutex_init(&global_sum_mutex,NULL);

  pthread_mutex_init(&gen_mutex1,NULL);
  pthread_mutex_init(&gen_mutex2,NULL);

  pthread_mutex_init(&log_mutex1,NULL);
  pthread_mutex_init(&log_mutex2,NULL);

  pthread_mutex_init(&mod_mutex1,NULL);
  pthread_mutex_init(&mod_mutex2,NULL);

  pthread_mutex_init(&sum_mutex1,NULL);

  pthread_t generate_thread[gen_thread_size];
  pthread_t log_thread[log_thread_size];
  pthread_t mod_thread[mod_thread_size];
  pthread_t sum_thread[sum_thread_size];

  for (int i = 0; i < gen_thread_size
    ; i++) {
    pthread_create (&generate_thread[i],NULL,&generete_function,(void*)&i);
  }
  for (int i = 0; i < log_thread_size; i++) {
    pthread_create (&log_thread[i],NULL,&log_function,(void*)&i);
  }
  for (int i = 0; i < mod_thread_size; i++) {
    pthread_create (&mod_thread[i],NULL,&mod_function,(void*)&i);
  }
  for (int i = 0; i < sum_thread_size; i++) {
    pthread_create (&sum_thread[i],NULL,&sum_function,(void*)&i);
  }



  for (size_t i = 0; i < gen_thread_size; i++) {
    pthread_join(generate_thread[i],NULL);
  }
  for (size_t i = 0; i < log_thread_size; i++) {
    pthread_join(log_thread[i],NULL);
  }
  for (size_t i = 0; i < mod_thread_size; i++) {
    pthread_join(mod_thread[i],NULL);
  }
  for (size_t i = 0; i < sum_thread_size; i++) {
    pthread_join(sum_thread[i],NULL);
  }

  print_matrix();

  FILE *fptr;
  fptr=fopen("output_file.txt","w");
  fprintf(fptr, "The matrix is  \n");

  for (size_t j = 0; j < SIZE; j++) {
    if(j!=0 && j%5==0)
      fprintf(fptr,"\n");
    for (size_t i = 0; i < SIZE; i++) {
      if(i!=0 && i%5==0)
        fprintf(fptr,"  ");
      fprintf(fptr,"%3d",matrix[j][i]);

    }
    fprintf(fptr,"\n");

  }
  fprintf(fptr, "The global sum is :%d\n",global_sum );

  fclose(fptr);

  pthread_mutex_destroy(&gen_mutex1);
  pthread_mutex_destroy(&gen_mutex2);
  pthread_mutex_destroy(&log_mutex1);
  pthread_mutex_destroy(&log_mutex2);
  pthread_mutex_destroy(&mod_mutex1);
  pthread_mutex_destroy(&mod_mutex2);
  pthread_mutex_destroy(&sum_mutex1);
  pthread_mutex_destroy(&global_sum_mutex);
  pthread_exit(NULL);
  return 1;
}
