//Kam Wai Kin  wkkam9  56328031
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <iomanip>
#include <vector>
#include <iomanip>
#include <string.h>
double* generate_frame_vector(int length);
double* compression(double* frame, int length);

int interval,genCount=0;
bool allDone; //all frames genarated
double* tempFrame=(double*)malloc(8 * sizeof(double));  // temporary frame recorder

sem_t cacheElement;  //frame in cache
sem_t cacheSpace; //empty space in cache
sem_t estimator;
sem_t transformer;

class Queue{    
  std::vector<double*> vector;  // vector to store queue elements
  int capacity;   // maximum capacity of the queue
 
  public:
    Queue(int size);     // constructor
    ~Queue();                   // destructor
 
    double* dequeue();
    void enqueue(double*);
    double* peek();
    int size();
    bool isEmpty();
    bool isFull();
};
 
Queue::Queue(int size){  // Constructor to initialize a queue
  capacity = size;
}
 
Queue::~Queue() {  // Destructor to free memory allocated to the queue
  vector.clear();
}
 
double* Queue::dequeue(){  // Utility function to dequeue the front element
  if (isEmpty())  // check for queue underflow
  {
   std::cout << "Underflow\nProgram Terminated\n";
   exit(EXIT_FAILURE);
  }
  double* x = vector.front();
  vector.erase(vector.begin());
  return x;
}

void Queue::enqueue(double* item){  // Utility function to add an item to the queue
  if (isFull())  // check for queue overflow
  {
      std::cout << "Overflow\nProgram Terminated\n";
      exit(EXIT_FAILURE);
  }
  vector.push_back(item);
}
 
double* Queue::peek(){  // Utility function to return the front element of the queue
  if (isEmpty())
  {
   std::cout << "Underflow\nProgram Terminated\n";
   exit(EXIT_FAILURE);
  }
  return vector.front();
}
 
int Queue::size() {  // Utility function to return the size of the queue
    return vector.size();
}
 
bool Queue::isEmpty() {  // Utility function to check if the queue is empty or not
    return (vector.size() == 0);
}
 
bool Queue::isFull() {  // Utility function to check if the queue is full or not
  return (vector.size() == capacity);
}


void* camera(void* args)
{
  Queue* cache = (Queue*)args;
  while (true)
  {
    sem_wait(&cacheSpace);
    double* frame_vector=generate_frame_vector(8);
    if(frame_vector==NULL)
    {
      allDone=true;
      break;
    }
    else
    {
      genCount++;
      cache->enqueue(frame_vector);
      usleep(interval*1000*1000);
      sem_post(&cacheElement);      
    }
  }
  pthread_exit(NULL);
}


void* trans(void* args)
{
  int transCount=0;
  Queue* cache = (Queue*)args;
  double* temp;
  while(1)
  {
    sem_wait(&cacheElement);
    sem_wait(&estimator);
    temp=cache->peek();
    //(double*)malloc(length * sizeof(double));
    memcpy(tempFrame,temp,8*sizeof(double));
    /*for(int i=0;i<8;i++)
    {
      std::cout<<" fk "<<std::fixed <<std::setprecision(6)<<temp[i]<<std::endl;
      //temp[i]=0.888888;
    }*/
    transCount++;
    compression(tempFrame,8);
    usleep(3*1000*1000);
    sem_post(&transformer);
    if(allDone && transCount==genCount)
      break;
  }
  pthread_exit(NULL);
}

void* esti(void* args)
{
  int estiCount=0;
  Queue* cache = (Queue*)args;
  while(1)
  {
    double MSE=0;
    sem_wait(&transformer);
    double* tempFrame2=cache->peek();
    for(int i=0;i<8;i++)
    {
      //std::cout<<" tempFrame2 "<<std::fixed <<std::setprecision(6)<<tempFrame2[i]<<std::endl;
      //std::cout<<" tempFrame "<<std::fixed <<std::setprecision(6)<<tempFrame[i]<<std::endl;
      double result = tempFrame2[i]-tempFrame[i];
      result=result*result;
      MSE+=result;
    }
    MSE/=8;
    estiCount++;
    std::cout<<"MSE="<<std::fixed <<std::setprecision(6)<<MSE<<std::endl;
    cache->dequeue();
    sem_post(&estimator);
    sem_post(&cacheSpace);
    if(allDone && genCount==estiCount)
      break;
    
  }
  pthread_exit(NULL);
}

int main(int argc, char** argv)
{
  interval=0;
  int iret1,iret2,iret3;
  pthread_t thread1,thread2,thread3;
  Queue cache(5);
  if(argc==2)  //only 2 arguments
  {
    interval=atoi(argv[1]);
    sem_init(&cacheElement,0,0);
    sem_init(&cacheSpace,0,5);
    sem_init(&transformer,0,0);
    sem_init(&estimator,0,1);
    iret1=pthread_create(&thread1, NULL, camera, &cache);
    iret2=pthread_create(&thread2, NULL, trans, &cache);
    iret3=pthread_create(&thread3, NULL, esti, &cache);
    if(iret1)
    {
      std::cout<<"Cannot create camera thread!"<<std::endl;
      exit(-1);
    }
    if(iret2)
    {
      std::cout<<"Cannot create transformer thread!"<<std::endl;
      exit(-1);
    }
    if(iret3)
    {
      std::cout<<"Cannot create estimator thread!"<<std::endl;
      exit(-1);
    }
    pthread_join(thread1,NULL);  //wait camera thread to done
    pthread_join(thread2,NULL);  //wait transformer thread to done
    pthread_join(thread3,NULL);  //wait estimator thread to done
    pthread_exit(NULL);
  }
  std::cout<<"Input is illegal!"<<std::endl;
  exit(0);
}