#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include<signal.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "nnet.h"


extern int NEED_PRINT;
extern int NEED_FOR_ONE_RUN;
extern int input_depth;
extern int adv_found;

extern int CHECK_ADV_MODE;
extern int PARTIAL_MODE;

extern int thread_count;
extern float max_depth;
extern int leaf_num;

extern int progress;

extern struct timeval start,finish, last_finish;


#define ADV_THRESHOLD  0.00001

pthread_mutex_t lock;
extern int count;


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


void check_adv(struct NNet *nnet, struct Interval *input);

int check_functions(struct NNet *nnet, struct Interval *output);

int check_functions1(struct NNet *nnet, struct Matrix *output);

int check_not_min(struct NNet *nnet, struct Interval *output);

int check_not_min1(struct NNet *nnet, struct Matrix *output);

int check_not_min_p7(struct NNet *nnet, struct Interval *output);

int check_not_min1_p7(struct NNet *nnet, struct Matrix *output);

void *direct_run_check_thread(void *args);

int direct_run_check(struct NNet *nnet, struct Interval *input,
					 struct Interval *output, struct Interval *grad, 
					 int depth, int *feature_range, int feature_range_length, 
					 int split_feature);

int split_interval(struct NNet *nnet, struct Interval *input,
				   struct Interval *output, struct Interval *grad, 
				   int depth, int *feature_range, int feature_range_length, 
				   int split_feature);
