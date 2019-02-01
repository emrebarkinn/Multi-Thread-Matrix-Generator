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
