#include "matrix.h"
#include <string.h>
#include "interval.h"
typedef int bool;
enum { false, true };

#define NEED_OUTWARD_ROUND 1
#define OUTWARD_ROUND 0.00000005

extern int PROPERTY;
extern char *LOG_FILE;
extern FILE *fp;


/*
 * Network instance modified from Reluplex
 * malloc all the memory needed for network
 */
struct NNet 
{
    int symmetric;     //1 if network is symmetric, 0 otherwise
    int numLayers;     //Number of layers in the network
    int inputSize;     //Number of inputs to the network
    int outputSize;    //Number of outputs to the network
    int maxLayerSize;  //Maximum size dimension of a layer in the network
    int *layerSizes;   //Array of the dimensions of the layers in the network

    float *mins;      //Minimum value of inputs
    float *maxes;     //Maximum value of inputs
    float *means;     //Array of the means used to scale the inputs and outputs
    float *ranges;    //Array of the ranges used to scale the inputs and outputs
    float ****matrix; //4D jagged array that stores the weights and biases
                       //the neural network.
    struct Matrix* weights;
    struct Matrix* bias;

    int target;
    int *feature_range;
    int feature_range_length;
    int split_feature;
};

//Functions Implemented
struct NNet *load_network(const char *filename, int target);

void load_inputs(int PROPERTY, int inputSize, float *u, float *l);

/*  
 * Uses for loop to calculate the output
 * 0.00002607 sec for one run with one core
*/
int evaluate(struct NNet *network, struct Matrix *input, struct Matrix *output);
/*  
 * Uses for loop to calculate the interval output
 * 0.000091 sec for one run with one core
*/
int evaluate_interval(struct NNet *network, struct Interval *input, struct Interval *output);
/*  
 * Uses for loop with equation to calculate the interval output
 * 0.000229 sec for one run with one core
*/
int evaluate_interval_equation(struct NNet *network, struct Interval *input, struct Interval *output);

/*  
 * Uses sgemm to calculate the output
 * 0.00001359 sec for one run with one core
*/
int forward_prop(struct NNet *network, struct Matrix *input, struct Matrix *output);

int forward_prop_interval(struct NNet *network, struct Interval *input, struct Interval *output);
/*  
 * Uses sgemm with equation to calculate the interval output
 * 0.000185 sec for one run with one core
*/
int forward_prop_interval_equation(struct NNet *network, struct Interval *input, struct Interval *output, struct Interval *grad);


int forward_prop_interval_equation_new2(struct NNet *network, struct Interval *input, struct Interval *output, struct Interval *grad);

int forward_prop_interval_equation_new(struct NNet *network, struct Interval *input, struct Interval *output, struct Interval *grad);

void destroy_network(struct NNet *network);

void denormalize_input(struct NNet *nnet, struct Matrix *input);

void denormalize_input_interval(struct NNet *nnet, struct Interval *input);

void normalize_input(struct NNet *nnet, struct Matrix *input);

void normalize_input_interval(struct NNet *nnet, struct Interval *input);
/*
 * The back prop to calculate the gradient
 * 0.000249 sec for one run with one core
*/
void backward_prop(struct NNet *nnet, struct Interval *grad, int R[][nnet->maxLayerSize]);