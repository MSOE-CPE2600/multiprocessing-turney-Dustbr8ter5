/// CPE 2600 Lab 11
//  Vincent Vassallo
//  Section 111
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "jpegrw.h"
#include <sys/wait.h>

// local routines
static int iteration_to_color(int i, int max);
static int iterations_at_point(double x, double y, int max);
static void compute_image(imgRawImage *img, double xmin, double xmax, double ymin, double ymax, int max, int threads);
static void show_help();
// Define
typedef struct imageinfo {
    imgRawImage *img;
    double xmin; 
    double ymin;
    double xmax;
    double ymax;
    int max;
    int threads;
    int thread_id;
} imageinfo;

int main( int argc, char *argv[]) {
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	const char *outfile = "mandel.jpg";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
    int    child = 1;
    int    threads = 1;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:c:t:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
            case 'c':
                child = atoi(optarg);
                break;
            case 't':
                threads = atoi(optarg);
                break;
			case 'h':
				show_help();
				exit(1);
				break;
            
		}
	}
    //printf("%d Child\n", child);
    if(child == 1) {
        char *temp;
        double scale = xscale / 100;
        for(int i = 0; i < 50; i++) {
            temp = malloc(strlen(outfile)*sizeof(char));
            char num[4];
            char extension[4] = ".jpg";
            sprintf(num, "%d", i);
            for(int j = 0; j < strlen(outfile) - 4; j++) {
                temp[j] = outfile[j];
            }
            strcat(temp, num);
            strcat(temp, extension);
            // Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
            if(i == 0) {
	            yscale = xscale / image_width * image_height;
            } else {
                xscale = xscale - scale;
                yscale = xscale / image_width * image_height;
            }
	        // Display the configuration of the image.
	        printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s processes=%d threads=%d\n",xcenter,ycenter,xscale,yscale,max,temp,child,threads);

	        // Create a raw image of the appropriate size.
	        imgRawImage* img = initRawImage(image_width,image_height);

	        // Fill it with a black
	        setImageCOLOR(img,0);

	        // Compute the Mandelbrot image
	        compute_image(img, xcenter - xscale / 2, xcenter + xscale / 2, ycenter - yscale / 2, ycenter + yscale / 2, max, threads);

	        // Save the image in the stated file.
	        storeJpegImageFile(img,temp);

	        // free the mallocs
	        freeRawImage(img);
            free(temp);
        }
    } else { //Fork program amount of desired processes
        double scale = xscale / 100;
        for (int i = 0; i < child; i++) {
            if (fork() == 0) {
                //Set initial image
                int start = i * (50 / child);
                int end;
                //Set ending image
                if (i == child - 1) { //If only 1 image remains
                    end = 50;
                } else {
                    end = start + (50 / child);
                }

			    //Make child process designated images
		        for (int j = start; j < end; j++) {
                    double modscale = xscale - scale * j;
				    // Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
                    if(j == 0) { //
                        yscale = xscale / image_width * image_height;
                    } else {
                        yscale = modscale / image_width * image_height;
                    }
		    	    // Create a raw image of the appropriate size.
	                imgRawImage* img = initRawImage(image_width, image_height);

	                // Fill it with a black
	                setImageCOLOR(img, 0);

	                // Compute the Mandelbrot image
    	            compute_image(img, xcenter - modscale / 2, xcenter + modscale / 2, ycenter - yscale / 2 , ycenter + yscale / 2, max, threads);

	                // Save the image in the stated file.
	        	    char *temp = malloc(strlen(outfile)*sizeof(char));
                    for(int k = 0; k < strlen(outfile) - 4; k++) {
                        temp[k] = outfile[k];
                    }
                    char extension[4] = ".jpg";
                    char num[2];
	        	    sprintf(num, "%d", j);
                    strcat(temp, num);
                    strcat(temp, extension);
	                storeJpegImageFile(img, temp);

    	    	    // Display the configuration of the image.
            	    printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s processes=%d threads=%d\n", xcenter, ycenter, modscale, yscale, max, temp, child, threads);

	                // free the mallocs
	                freeRawImage(img);
                    free(temp);
		        }
			    exit(1);
		    }
	    }
        // Wait for children to finish
	    int status;
	    for (int i = 0; i < child; i++) {
		    wait(&status);
	    }
	    printf("All Images Processed.\n");
    }
	return 0;
}


/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max ) {
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

// For every pixel in the image...
void *compute_thread(void *arg) {

    //Redefine struct after pass through
    imageinfo* image = (imageinfo*) arg;

    //Recreate variables after pass through 
    imgRawImage *img = image->img;
    double xmin = image->xmin; 
    double ymin = image->ymin;
    double xmax = image->xmax;
    double ymax = image->ymax;
    int width = image->img->width;
    int height = image->img->height;
    int max = image->max;
    int threads = image->threads;
    int thread_id = image->thread_id;

    //Determine the portion of the image the thread receives
    int row = height / threads;

    //Determine the starting row for the thread
    int start = thread_id * row;

    //Determine where the thread should end
    int end;
    if (thread_id == threads - 1) {
        end = height;
    } else {
        end = start + row;
    }

    for(int j = start; j < end; j++) {

		for(int i = 0; i < width; i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(img,i,j,iteration_to_color(iters,max));
		}
	}
    return NULL;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int threads) {
    //Define array of threads
    pthread_t thread[threads];
    //Define array of structs
    imageinfo image[threads];
    
    //Create threads depending on user amount
	for(int i = 0; i < threads; i++) {
        //Copy variables into struct
        image[i].img = img;
        image[i].xmin = xmin;
        image[i].xmax = xmax;
        image[i].ymin = ymin;
        image[i].ymax = ymax;
        image[i].max = max;
        image[i].threads = threads;
        image[i].thread_id = i;
        //Create thread to run compute_thread and pass the struct in
        pthread_create(&thread[i], NULL, compute_thread, &image[i]);

    }
    //Rejoin threads
    for(int i = 0; i < threads; i++) {
        pthread_join(thread[i], NULL);
    }
}





/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max ) {
	int color = 0xFEEFDE*iters/(double)max;
	return color;
}


// Show help message
void show_help() {
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}
