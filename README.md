# COMPSCI 377 LAB: Buffer Caching

## Purpose

This lab is designed to explore caching and eviction, and how they are used/implemented in xv6. Please make sure that all of your answers to questions in these labs come from work done on the Edlab environment – otherwise, there may be inconsistent results and you may lose points.

Please submit your answers to this lab on Gradescope. All answers are due by the time specified on Gradescope. The TA present in your lab will do a brief explanation of the various parts of this lab, but you are expected to answer all questions by yourself. Please raise your hand if you have any questions during the lab section – TAs will be notified you are asking a question. Questions and Parts have a number of points marked next to them to signify their weight in this lab’s final grade. Labs are weighted equally, regardless of their total points.

## Instructions
Once you `ssh` in to Edlab, you can clone this repo using:

```bash
git clone https://raw.githubusercontent.com/brendonky18/377-lab-buffer-caching/
```

Then you can use `cd` to open the directory you just cloned:

```bash
cd 377-lab-buffer-caching
```

*Before doing anything, please make sure to read through all of the instructions first.*

### Part 1: Creating a Test Program (10 Points)

In this lab, you will be working with the file system in xv6. Specifically, you will be working with the buffer cache in order to modify it's eviction policy. Normally, xv6 uses a LRU eviction policy, and in this lab you will be changing it to FIFO.

Different eviction policies are more well-suited to different scenarios, and you will have to design a program in order to benchmark these policies under different kinds of loads. 

The three test cases will be sequential access, random access, and weighted random access of memory. The buffer cache stores 30 buffers, each 512B in size. The worst case scenario for FIFO and LRU is known as a *degenerate pattern*, where a sequential order of accesses results in a buffer being evicted right before it will be read again. For this to happen, the data being read must larger than what the buffer cache can store, >15.6kB in total. 

This means that when running the benchmarks, your test file must be larger than 15.6kB. 

In order to benchmark an average case and worst case scenario, you will read and print the contents of a file byte-by-byte, in random order and in sequential order. 

In order to do this, you will need to use the [`open`](https://man7.org/linux/man-pages/man2/open.2.html), [`read`](https://man7.org/linux/man-pages/man2/read.2.html) and [`close`](https://man7.org/linux/man-pages/man2/close.2.html) unix system calls in order to interact with the files. The man pages for each of the syscalls have been linked.

#### Sequential Access
In the sequential read, you will have to read the entirety of your file twice, once to ensure we are not using a cold cache and a second time to perform the benchmark.  

Unfortunately, xv6 does not implement the `lseek` system call, so in order to read the file back from the start a second time, you must close the file and then open it again. 

#### Random Access
In the random read, you will have to perform the same total number of read operations (the exact number will vary depending on your file size).

#### Weighted-Random Access
In the weighted random distribution, you will do the same as before, however you will be weighting certain elements more. This is meant to simulate locality of reference, which tends to be quite high for an "average" use case. A function which calculates the weighted random indices has already been provided. 

#### Misc.
At the end of each benchmark, you will also need to print the hit rate. Two system calls have been added to assist you in this: `hitrate()` and `resethitrate()`. Respectively, they return a float representing the system's hit rate, and resets the system's hit rate.

A template file has already been created [here](xv6/benchmark.c), which is where you will be implementing your benchmark programs. For the file that the benchmarks will read, you should paste your text in [this](xv6/read_file) file. The `Makefile` has also been updated appropriately, so that both provided file will be included when you run `make`. 

In order to complete this part of the assignment you do not need to modify the `Makefile` or any other file besides the two linked above. All you need to do is implement the code where commented `TODO` in the provided files, and then compile and run the code inside xv6 to confirm that it works.

### Part 2: Modifying the Eviction Policy (10 Points)

For this part of the assignment, you will be working in `bio.c` in order to modify the xv6's eviction policy. In the file, comments have been made indicating what changes need to be made and where to make them. The changes themselves should only be adding or removing no more than 20 lines of code in total in `bio.c`. 

## Part 3: Measuring Performance (5 Points)
*Before* making any changes to the eviction policy in Part 2, run your test program to determine the efficiency of the default LRU policy.

For each of the test cases, make sure to run them several times (at least 3) in order to get a reasonable sample size. 

Make sure to take a screen shot of the results after each test, you will submit these on Gradescope. 

This repo includes a Makefile that allows you to locally compile and run all the sample code listed in this tutorial. You can compile them by running `make`. Feel free to modify the source files yourself, after making changes you can run `make` again to build new binaries from your modified files. You can also use `make clean` to remove all the built files, this command is usually used when something went wrong during the compilation so that you can start fresh. 

To execute the benchmarks, launch xv6 by running `make qemu-nox`, then once it has finished compiling and the emulator is up run the command `./benchmark`, you can optionally specify a file as an argumnet `./benchmark <file>`. If no file is specified, it will default to running the benchmarks on the file `read_file`.

