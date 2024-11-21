# System Programming Lab 11 Multiprocessing
## Overview
Single process was completed using a for loop to create all 50 images with slight decrements in scale between them.

Multiple processes used forks nested in a for loop in order to create all the necessary processes to create images. Using the ratio of images/processes, each process was given a specific image to create and the necessary decrement to subtract from the starting xscale and to create the y scale.

In order to name the files properly, a temporary string was created in order to copy from outfile excluding the file extension. As the program iterates it concatenates the number of the image and the missing extension back on the temp and sends it off to be saved.

### Runtime Results
![image](./Graph.png)

### Discussion
**All testing was done using the same parameters to create the same set of 50 images**

The results make seem accurate. As more processes are added, the work is divided up more and more across the processor cores and the runtimes reduce. However as the number of processes surpass the number of cores in my machine, it slows substantially.