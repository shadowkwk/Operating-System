Please read this file first before using the CPP file.

How to Compile the CPP File:
Save the file with a shorter file name such as asg3.cpp
Save it in the same folder as generate_frame_vector.cpp
In the terminal write the following command to compile the file:

g++ generate_frame_vector.cpp asg3.cpp -lpthread -o asg3

How to Run the File:
To run the compiled asg3.cpp, type "./asg3 <interval>" in the terminal. In the place of interval, write any integer.
Now all the test cases can be tried. We have added comments in the CPP file for better understanding of our code.

Description of Files:
The basic difference between files is the way in which the Temporary Frame Recorder(TFR) is implemented. The implementation is described for each file below.
In this file, the TFR is implemented as a FIFO queue of size = 5 just like the cache. So when the threads are created, camera starts to run immediately while the Transformer waits until some element is loaded in the cache. The estimator also waits for some element to be loaded into the TFR. In total there are 4 semaphores which have been used and their function is described below:
1. &cacheEl: This semaphore is used to keep a track of the number of elements in the cache. It also helps the camera to signal the transformer that something has been added to the cache. Initially the value of this semaphore is 0 so the transformer waits on this semaphore until the camera signals. The value of this semaphore is decreased by the estimator after it calculates the MSE to signal the deletion of an element from cache.
2. &TREl: This semaphore is used to keep a track of the number of elements in the TFR. It also helps the transformer to signal the estimator that something has been added to the TFR. Initially the value of this semaphore is 0 so the estimator waits on this semaphore until the transformer signals. The value of this semaphore is decreased by the estimator after it calculates the MSE to signal that the element at the front of the TFR has been processed.
3. &cacheSpace: This semaphore is used to keep a track of the empty spaces in the cache. It also helps the camera to check whether the cache is full or not. Since the size of the cache is 5, the initial value of this semaphore is 5. It is decremented everytime the camera adds an element to the cache and it is incremented whenever the estimator deletes an element from the cache. If the value of this semaphore becomes 0, it means that the cache is Full and the camera waits on this semaphore until the estimator deletes an element from the cache and signals the camera.
4. &TRSpace: This semaphore is used to keep a track of the empty spaces in the TFR. It also helps the transformer to check whether the TFR is full or not. Since the size of the TFR is 5, the initial value of this semaphore is 5. It is decremented everytime the transformer adds an element to the TFR and quantizes it and it is incremented whenever the estimator processes an element from the TFR. If the value of this semaphore becomes 0, it means that the TFR is Full and the transformer waits on this semaphore until the estimator processes an element from the TFR and signals the transformer.
5. &trans: This semaphore is used to make the estimator wait when the transformer is quantizing the frames into the TFR.
6. &est: This semaphore is used to make the transformer wait when the estimator is using the values in the TFR to calculate MSE.
