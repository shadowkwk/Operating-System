//Kam Wai Kin  wkkam9  56328031
//Chan Wai Hoi  whchan435  55713410
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include "generate_frame_vector.c"

double* generate_frame_vector(int length);
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
int s,interval,length=4;
 
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



void *camera_thread(void *args)
{
  Queue* cache = (Queue*)args;
  double* frame_vector;
  do{
    usleep(interval*1000*1000);  // sleep amount of interval 
  } 
  while(cache->isFull());
  
  while(1){
    s=pthread_mutex_lock(&mtx);  //lock before accessing the sharded cache
    frame_vector =generate_frame_vector(length);  //generate one flattened frame
    if(frame_vector==NULL){
      break;
    }
    if(s==0)
    {
      s=pthread_mutex_unlock(&mtx);  //unlock if already locked
    }
    cache->enqueue(frame_vector);  //add camer frame into queue
    do{
      usleep(interval*1000*1000);
    }
    while(cache->isFull());
    
    s=pthread_mutex_unlock(&mtx);  //unlock after data access
  }
  pthread_exit(NULL);
}

void *quantizer_thread(void *args)  //use quantizer thread to quantize, print, delete the frame 
{
    Queue* cache = (Queue*)args;
    double* frame_vector;
    int retries=0;
    
    while(cache->isEmpty())
    {
      usleep(1*1000*1000);  //sleep 1000 ms
      if(++retries>3)  
      {
        pthread_exit(NULL);
      }
    }
    retries=0;  //reset the retry
    
    while(1)
    {
      s=pthread_mutex_lock(&mtx);  //lock before accessing the sharded cache
      double* result = cache->peek();
      for(int i=0;i<length;i++)
      {
        if(result[i]<=0.5)
          result[i]=0;
        if(result[i]>0.5)
          result[i]=1;
        std::cout << result[i] << " ";
      }
      std::cout<<std::endl;
      cache->dequeue();      
      usleep(3*1000*1000);//sleep 3000 ms
      s=pthread_mutex_unlock(&mtx);  //unlock after data access
      
      while(cache->isEmpty()){
        usleep(1*1000*1000);  //sleep 1000 ms
        if(++retries>3)
        {
          pthread_exit(NULL);
        }
      }
    }   
    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    pthread_t thread1,thread2;
    int iret1,iret2;
    if(argc == 2)	// argument more than 2 is not allowed
    {
      interval=atoi(argv[1]);
      if( interval<=0 )  //interval less than 0
        {
          std::cout<<"Input is illegal!"<<std::endl;
          exit(0);
        }
    }
    else
    {
      std::cout<<"Input is illegal!"<<std::endl;  //more than 1 command line argument
      exit(0);
    }
    
    Queue cache(8);
    iret1=pthread_create(&thread1,NULL,camera_thread,&cache);
    iret2=pthread_create(&thread2,NULL,quantizer_thread,&cache);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    exit(0);
}
