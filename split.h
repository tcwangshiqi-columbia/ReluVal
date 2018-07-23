/*
 ------------------------------------------------------------------
 ** Top contributors (to current version):
 **   Shiqi Wang and Suman Jana
 ** This file is part of the ReluVal project.
 ** Copyright (c) 2018-2019 by the authors listed in the file copyright
 ** and their institutional affiliations.
 ** All rights reserved.
 -----------------------------------------------------------------
 */

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include<signal.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "nnet.h"

#ifndef SPLIT_H
#define SPLIT_H


/* Print detailed progress */
extern int NEED_PRINT;

/* No bisection mode */
extern int NEED_FOR_ONE_RUN;

/* set 1 if a concrete adversarial example is
found */
extern int adv_found;

/* Mode for faster search concrete
adversarial examples */
extern int CHECK_ADV_MODE;
extern int PARTIAL_MODE;

/* Bisection tree info */
extern float max_depth;
extern int leaf_num;

/* Progress record out of 1024 */
extern int progress;

/* Time record */
extern struct timeval start,finish, last_finish;

/* If the input range is less than ADV_THRESHOLD, 
then it will check for concrete adversarial example*/
#define ADV_THRESHOLD  0.00001

/* Thread locker */
pthread_mutex_t lock;

/* Active threads number */
extern int count;


/*
 * Define the argument structure of main function
 * direct_run_check used for parallelization.
 */
struct direct_run_check_args
{
	struct NNet *nnet;
	struct Interval *input;
	struct Interval *output;
	struct Interval *grad; 
	int depth;
	int *feature_range;
	int feature_range_length;
	int split_feature;
	//int avg_depth;
};


/*
 * Check the concrete adversarial examples of 
 * the middle point of given input ranges.
 */
void check_adv(struct NNet *nnet, struct Interval *input);


/*
 * Check the predefined properties given 
 * approximated output ranges.
 */
int check_functions(struct NNet *nnet, struct Interval *output);


/*
 * Check the predefined properties given 
 * concrete output.
 */
int check_functions1(struct NNet *nnet, struct Matrix *output);


/*
 * Function for parallelization.
 */
void *direct_run_check_thread(void *args);


/*
 * Main function to decide whether need to further split input ranges 
 * or terminate according to given approximated output ranges.
 */
int direct_run_check(struct NNet *nnet, struct Interval *input,
					 struct Interval *output, struct Interval *grad, 
					 int depth, int *feature_range, int feature_range_length, 
					 int split_feature);


/*
 * Function for splitting and creating new threads
 * for newly split input ranges.
 */
int split_interval(struct NNet *nnet, struct Interval *input,
				   struct Interval *output, struct Interval *grad, 
				   int depth, int *feature_range, int feature_range_length, 
				   int split_feature);


#endif