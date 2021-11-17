# Multithreaded Relaxation Technique Program
This code emulates the relaxation technique of solving differential equations.

## Usage
The C code takes a square 2d array of doubles and performs a series of iterations on it. In each iteration the
boundary values stay constant but all other values are rewritten to be the average of the four
adjacent values. Iterations are performed until all values are unchanging within a given error
margin (such as 0.001).

The program takes an array, the size of an array, an error margin and a number of threads to
use in the computation. Then performs iterations on the technique until a final array is produced,
which the code outputs.

## Approach to Parallelism
The program initialises two blocks of memory to work on and loads the given array into the first
block. Then the program divides up the given array into sections of consecutive rows of the array
and gives each of the threads one of the sections to work on. During each iteration each of the
threads reads data on it’s section from the first block, performs the averaging calculation and then
writes the results to the second block of memory.

After the iteration the threads pause for the main thread to check that all of the values are
within the error margin. If they are then the process is complete. If not the threads swap the
memory blocks that they were working on and performing another iteration; reading data from
the second block of memory and writing to the first.

Swapping the blocks of memory like this is fast as large amounts of data to not need to be
copied between arrays. And prevents clashes in threads editing and accessing data at the same
time, as at any given time one block of memory is exclusively being read from while the other is
exclusively being written to.

## Synchronisation and Avoiding Race Conditions
Synchronisation of threads is achieved using barriers and mutexes. Barriers are placed after the
iterations to insure all threads have finished an iteration before the main thread checks if the
arrays are within error. And mutexes are created to ’pause’ the iterating threads while the main
thread performs this check,. This is done by the main thread locking a mutex for each of the
threads and then the threads trying to access the mutex. The threads are stuck waiting for access,
and are granted access and allowed to continue when the main thread releases the lock.
