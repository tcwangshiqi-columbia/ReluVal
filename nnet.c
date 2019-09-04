/*
 ------------------------------------------------------------------
 ** Top contributors:
 **   Shiqi Wang and Suman Jana
 ** This file is part of the ReluVal project.
 ** Copyright (c) 2018-2019 by the authors listed in the file LICENSE
 ** and their institutional affiliations.
 ** All rights reserved.
 -----------------------------------------------------------------
 */


#include "nnet.h"


int PROPERTY = 5;
char *LOG_FILE = "logs/log.txt";
FILE *fp;

/*
 * Load_network is a function modified from Reluplex
 * It takes in a nnet filename with path and load the 
 * network from the file
 * Outputs the NNet instance of loaded network.
 */
struct NNet *load_network(const char* filename, int target)
{

    FILE *fstream = fopen(filename,"r");

    if (fstream == NULL) {
        printf("Wrong network!\n");
        exit(1);
    }

    int bufferSize = 10240;
    char *buffer = (char*)malloc(sizeof(char)*bufferSize);
    char *record, *line;
    int i=0, layer=0, row=0, j=0, param=0;

    struct NNet *nnet = (struct NNet*)malloc(sizeof(struct NNet));

    nnet->target = target;

    line=fgets(buffer,bufferSize,fstream);

    while (strstr(line, "//") != NULL) {
        line = fgets(buffer,bufferSize,fstream); 
    }

    record = strtok(line,",\n");
    nnet->numLayers = atoi(record);
    nnet->inputSize = atoi(strtok(NULL,",\n"));
    nnet->outputSize = atoi(strtok(NULL,",\n"));
    nnet->maxLayerSize = atoi(strtok(NULL,",\n"));

    nnet->layerSizes = (int*)malloc(sizeof(int)*(nnet->numLayers+1));
    line = fgets(buffer,bufferSize,fstream);
    record = strtok(line,",\n");

    for (i = 0;i<((nnet->numLayers)+1);i++) {
        nnet->layerSizes[i] = atoi(record);
        record = strtok(NULL,",\n");
    }

    line = fgets(buffer,bufferSize,fstream);
    record = strtok(line,",\n");
    nnet->symmetric = atoi(record);

    nnet->mins = (float*)malloc(sizeof(float)*nnet->inputSize);
    line = fgets(buffer,bufferSize,fstream);
    record = strtok(line,",\n");

    for (i = 0;i<(nnet->inputSize);i++) {
        nnet->mins[i] = (float)atof(record);
        record = strtok(NULL,",\n");
    }

    nnet->maxes = (float*)malloc(sizeof(float)*nnet->inputSize);
    line = fgets(buffer,bufferSize,fstream);
    record = strtok(line,",\n");

    for (i = 0;i<(nnet->inputSize);i++) {
        nnet->maxes[i] = (float)atof(record);
        record = strtok(NULL,",\n");
    }

    nnet->means = (float*)malloc(sizeof(float)*(nnet->inputSize+1));
    line = fgets(buffer,bufferSize,fstream);
    record = strtok(line,",\n");

    for (i = 0;i<((nnet->inputSize)+1);i++) {
        nnet->means[i] = (float)atof(record);
        record = strtok(NULL,",\n");
    }

    nnet->ranges = (float*)malloc(sizeof(float)*(nnet->inputSize+1));
    line = fgets(buffer,bufferSize,fstream);
    record = strtok(line,",\n");

    for (i = 0;i<((nnet->inputSize)+1);i++) {
        nnet->ranges[i] = (float)atof(record);
        record = strtok(NULL,",\n");
    }

    nnet->matrix = (float****)malloc(sizeof(float *)*nnet->numLayers);

    for (layer = 0;layer<(nnet->numLayers);layer++) {
        nnet->matrix[layer] =\
                (float***)malloc(sizeof(float *)*2);
        nnet->matrix[layer][0] =\
                (float**)malloc(sizeof(float *)*nnet->layerSizes[layer+1]);
        nnet->matrix[layer][1] =\
                (float**)malloc(sizeof(float *)*nnet->layerSizes[layer+1]);

        for (row = 0;row<nnet->layerSizes[layer+1];row++) {
            nnet->matrix[layer][0][row] =\
                    (float*)malloc(sizeof(float)*nnet->layerSizes[layer]);
            nnet->matrix[layer][1][row] = (float*)malloc(sizeof(float));
        }

    }
    
    layer = 0;
    param = 0;
    i=0;
    j=0;

    char *tmpptr=NULL;

    float w = 0.0;

    while ((line = fgets(buffer,bufferSize,fstream)) != NULL) {

        if (i >= nnet->layerSizes[layer+1]) {

            if (param==0) {
                param = 1;
            }
            else {
                param = 0;
                layer++;
            }

            i=0;
            j=0;
        }

        record = strtok_r(line,",\n", &tmpptr);

        while (record != NULL) {   
            w = (float)atof(record);
            nnet->matrix[layer][param][i][j] = w;
            j++;
            record = strtok_r(NULL, ",\n", &tmpptr);
        }

        tmpptr=NULL;
        j=0;
        i++;
    }

    float orig_weights[nnet->maxLayerSize];
    float orig_bias;

    struct Matrix *weights=malloc(nnet->numLayers*sizeof(struct Matrix));
    struct Matrix *bias = malloc(nnet->numLayers*sizeof(struct Matrix));

    for (int layer=0;layer<nnet->numLayers;layer++) {
        weights[layer].row = nnet->layerSizes[layer];
        weights[layer].col = nnet->layerSizes[layer+1];
        weights[layer].data = (float*)malloc(sizeof(float)\
                    * weights[layer].row * weights[layer].col);

        int n=0;

        if (PROPERTY != 1) {

            /* weights in the last layer minus the weights of true label output. */
            if (layer == nnet->numLayers-1) {
                orig_bias = nnet->matrix[layer][1][nnet->target][0];
                memcpy(orig_weights, nnet->matrix[layer][0][nnet->target],\
                            sizeof(float)*nnet->maxLayerSize);

                for (int i=0;i<weights[layer].col;i++) {

                    for (int j=0;j<weights[layer].row;j++) {
                        weights[layer].data[n] =\
                                nnet->matrix[layer][0][i][j]-orig_weights[j];
                        n++;
                    }

                }

                bias[layer].col = nnet->layerSizes[layer+1];
                bias[layer].row = (float)1;
                bias[layer].data = (float*)malloc(sizeof(float)*bias[layer].col);

                for (int i=0;i<bias[layer].col;i++) {
                    bias[layer].data[i] = nnet->matrix[layer][1][i][0]-orig_bias;
                }
            }
            else {

                for (int i=0;i<weights[layer].col;i++) {

                    for (int j=0;j<weights[layer].row;j++) {
                        weights[layer].data[n] = nnet->matrix[layer][0][i][j];
                        n++;
                    }

                }

                bias[layer].col = nnet->layerSizes[layer+1];
                bias[layer].row = (float)1;
                bias[layer].data = (float*)malloc(sizeof(float) *\
                                        bias[layer].col);

                for (int i=0;i<bias[layer].col;i++) {
                    bias[layer].data[i] = nnet->matrix[layer][1][i][0];
                }

            }
        }
        else {

            for (int i=0;i<weights[layer].col;i++) {

                for (int j=0;j<weights[layer].row;j++) {
                    weights[layer].data[n] = nnet->matrix[layer][0][i][j];
                    n++;
                }

            }

            bias[layer].col = nnet->layerSizes[layer+1];
            bias[layer].row = (float)1;
            bias[layer].data = (float*)malloc(sizeof(float)*bias[layer].col);

            for (int i=0;i<bias[layer].col;i++) {
                bias[layer].data[i] = nnet->matrix[layer][1][i][0];
            }

        }

    }

    nnet->weights = weights;
    nnet->bias = bias;

    free(buffer);
    fclose(fstream);

    return nnet;

}


/*
 * destroy_network is a function modified from Reluplex
 * It release all the memory mallocated to the network instance
 * It takes in the instance of nnet
 */
void destroy_network(struct NNet *nnet)
{

    int i=0, row=0;
    if (nnet != NULL) {

        for (i=0;i<(nnet->numLayers);i++) {

            for (row=0;row<nnet->layerSizes[i+1];row++) {
                free(nnet->matrix[i][0][row]);
                free(nnet->matrix[i][1][row]);
            }

            free(nnet->matrix[i][0]);
            free(nnet->matrix[i][1]);
            free(nnet->weights[i].data);
            free(nnet->bias[i].data);
            free(nnet->matrix[i]);
        }

        free(nnet->weights);
        free(nnet->bias);
        free(nnet->layerSizes);
        free(nnet->mins);
        free(nnet->maxes);
        free(nnet->means);
        free(nnet->ranges);
        free(nnet->matrix);
        free(nnet);
    }

}


/*
 * Load the inputs of all the predefined properties
 * It takes in the property and input pointers
 */
void load_inputs(int PROPERTY, int inputSize, float *u, float *l)
{

    if (PROPERTY == 1) {
        float upper[] = {60760,3.141592,3.141592,1200,60};
        float lower[] = {55947.691,-3.141592,-3.141592,1145,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 2) {
        float upper[] = {60760,3.141592,3.141592, 1200, 60};
        float lower[] = {55947.691,-3.141592,-3.141592,1145,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 3) {
        float upper[] = {1800,0.06,3.141592,1200,1200};
        float lower[] = {1500,-0.06,3.10,980,960};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 4) {
        float upper[] = {1800,0.06,0,1200,800};
        float lower[] = {1500,-0.06,0,1000,700};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 5) {
        float upper[] = {400,0.4,-3.1415926+0.005,400,400};
        float lower[] = {250,0.2,-3.1415926,100,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY==16) {
        float upper[] = {62000,-0.7,-3.141592+0.005,200,1200};
        float lower[] = {12000,-3.141592,-3.141592,100,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY==26) {
        float upper[] = {62000,3.141592,-3.141592+0.005,200,1200};
        float lower[] = {12000,0.7,-3.141592,100,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY==7) {
        float upper[] = {60760,3.141592,3.141592,1200,1200};
        float lower[] = {0,-3.141592,-3.141592,100,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY==8) {
        float upper[] = {60760,-3.141592*0.75,0.1,1200,1200};
        float lower[] = {0,-3.141592,-0.1,600,600};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY==9) {
        float upper[] = {7000,-0.14,-3.141592+0.01,150,150};
        float lower[] = {2000,-0.4,-3.141592,100,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY==10) {
        float upper[] = {60760,3.141592,-3.141592+0.01,1200,1200};
        float lower[] = {36000,0.7,-3.141592,900,600};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 11) {
        float upper[] = {400,0.4,-3.1415926+0.005,400,400};
        float lower[] = {250,0.2,-3.1415926,100,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 12) {
        float upper[] = {60760,3.141592,3.141592, 1200, 60};
        float lower[] = {55947.691,-3.141592,-3.141592,1145,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 13) {
        float upper[] = {60760,3.141592,3.141592, 360, 360};
        float lower[] = {60000,-3.141592,-3.141592,0,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 14) {
        float upper[] = {400,0.4,-3.1415926+0.005,400,400};
        float lower[] = {250,0.2,-3.1415926,100,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 15) {
        float upper[] = {400,-0.2,-3.1415926+0.005,400,400};
        float lower[] = {250,-0.4,-3.1415926,100,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 100) {
        float upper[] = {400,0,-3.1415926+0.025,250,200};
        float lower[] = {250,0,-3.1415926+0.025,250,200};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 101) {
        float upper[] = {400,0.4,-3.1415926+0.025,250,200};
        float lower[] = {250,0.2,-3.1415926+0.025,250,200};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 102) {
        float upper[] = {400,0.2,-3.1415926+0.05,0,0};
        float lower[] = {250,0.2,-3.1415926+0.05,0,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 110) {
        float upper[] = {10000,3.141592,-3.141592+0.01,1200,1200};
        float lower[] = {1000,3.141592,-3.141592+0.01,1200,1200};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 111) {
        float upper[] = {1000,3.141592,-3.141592+0.01,1200,1200};
        float lower[] = {1000,3.141592,-3.141592+0.01,0,1200};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

    if (PROPERTY == 112) {
        float upper[] = {1000,3.141592,-3.141592+0.01,1200,1200};
        float lower[] = {1000,3.141592,-3.141592+0.01,1200,0};
        memcpy(u, upper, sizeof(float)*inputSize);
        memcpy(l, lower, sizeof(float)*inputSize);
    }

}


/*
 * Naive interval propagation with openblas.
 * It takes in network and input interval.
 * Outputs the estimated output range. 
 */
int forward_prop_interval(struct NNet *network,\
                        struct Interval *input,\
                        struct Interval *output)
{

    int i,j,layer;

    if (network == NULL) {
        printf("No network can be loaded!\n");
        exit(1);
    }

    struct NNet* nnet = network;
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;
    int maxLayerSize   = nnet->maxLayerSize;

    float z_upper[nnet->maxLayerSize];
    float z_lower[nnet->maxLayerSize];
    float a_upper[nnet->maxLayerSize];
    float a_lower[nnet->maxLayerSize];

    struct Matrix Z_upper = {z_upper, 1, inputSize};
    struct Matrix A_upper = {a_upper, 1, inputSize};
    struct Matrix Z_lower = {z_lower, 1, inputSize};
    struct Matrix A_lower = {a_lower, 1, inputSize};

    for (i=0;i < nnet->inputSize;i++) {
        z_upper[i] = input->upper_matrix.data[i];
        z_lower[i] = input->lower_matrix.data[i];
    }

    memcpy(Z_upper.data, input->upper_matrix.data,\
            nnet->inputSize*sizeof(float));
    memcpy(Z_lower.data, input->lower_matrix.data,\
            nnet->inputSize*sizeof(float));

    float temp_upper[maxLayerSize*maxLayerSize];
    float temp_lower[maxLayerSize*maxLayerSize];
    struct Matrix Temp_upper = {temp_upper,maxLayerSize,maxLayerSize};
    struct Matrix Temp_lower = {temp_upper,maxLayerSize,maxLayerSize};

    for (layer=0;layer<numLayers;layer++) {
        A_upper.row = A_lower.row = nnet->bias[layer].row;
        A_upper.col = A_lower.col = nnet->bias[layer].col;

        memcpy(A_upper.data, nnet->bias[layer].data,\
                A_upper.row*A_upper.col*sizeof(float));
        memcpy(A_lower.data, nnet->bias[layer].data,\
                A_lower.row*A_lower.col*sizeof(float));

        for (i=0;i<nnet->weights[layer].row*nnet->weights[layer].col;i++) {

            if (nnet->weights[layer].data[i] >= 0) {
                Temp_upper.data[i] = Z_upper.data[i%5];
                Temp_lower.data[i] = Z_lower.data[i%5];
            }
            else {
                Temp_upper.data[i] = Z_lower.data[i%5];
                Temp_lower.data[i] = Z_upper.data[i%5];
            }

            Temp_upper.row = Temp_lower.row = nnet->weights[layer].row;
            Temp_upper.col = Temp_lower.col = Z_upper.col;
        }

        matmul_with_bias(&Temp_upper, &nnet->weights[layer], &A_upper);
        matmul_with_bias(&Temp_lower, &nnet->weights[layer], &A_lower);

        Z_upper.row = Z_lower.row = nnet->bias[layer].row;
        Z_upper.col = Z_lower.col = nnet->bias[layer].col;

        for (i=0;i<nnet->bias[layer].col*nnet->bias[layer].row;i++) {
            Z_upper.data[i] = A_upper.data[i*A_upper.col+i];
            Z_lower.data[i] = A_lower.data[i*A_lower.col+i];
        }

        if (layer<numLayers) {
            relu(&Z_upper);
            relu(&Z_lower);
        }

    }

    memcpy(output->upper_matrix.data, Z_upper.data,\
            Z_upper.row*Z_upper.col*sizeof(float));
    memcpy(output->lower_matrix.data, Z_lower.data,\
            Z_lower.row*Z_lower.col*sizeof(float));

    output->upper_matrix.row = output->lower_matrix.row\
                = Z_upper.row;
    output->upper_matrix.col = output->lower_matrix.col\
                = Z_upper.col;

    return 1;

}


/*
 * Following functions denomalize and normalize the concrete inputs
 * and input intervals.
 * They take in concrete inputs or input intervals.
 * Output normalized or denormalized concrete inputs or input intervals.
 */
void denormalize_input(struct NNet *nnet, struct Matrix *input)
{

    for (int i=0; i<nnet->inputSize;i++) {
        input->data[i] = input->data[i]*(nnet->ranges[i]) + nnet->means[i];
    }

}


void denormalize_input_interval(struct NNet *nnet, struct Interval *input)
{

    denormalize_input(nnet, &input->upper_matrix);
    denormalize_input(nnet, &input->lower_matrix);

}


void normalize_input(struct NNet *nnet, struct Matrix *input)
{

    for (int i=0;i<nnet->inputSize;i++) {

        if (input->data[i] > nnet->maxes[i]) {
            input->data[i] = (nnet->maxes[i]-nnet->means[i])/(nnet->ranges[i]);
        }
        else if (input->data[i] < nnet->mins[i]) {
            input->data[i] = (nnet->mins[i]-nnet->means[i])/(nnet->ranges[i]);
        }
        else {
            input->data[i] = (input->data[i]-nnet->means[i])/(nnet->ranges[i]);
        }

    }

}


void normalize_input_interval(struct NNet *nnet, struct Interval *input)
{

    normalize_input(nnet, &input->upper_matrix);
    normalize_input(nnet, &input->lower_matrix);

}


/*
 * Concrete forward propagation with openblas
 * It takes in network and concrete input matrix.
 * Outputs the concrete outputs.
 */
int forward_prop(struct NNet *network, struct Matrix *input, struct Matrix *output)
{

    int i,j,layer;

    struct NNet* nnet = network;
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;

    float z[nnet->maxLayerSize];
    float a[nnet->maxLayerSize];
    struct Matrix Z = {z, 1, inputSize};
    struct Matrix A = {a, 1, inputSize};

    memcpy(Z.data, input->data, nnet->inputSize*sizeof(float));

    for(layer=0;layer<numLayers;layer++){
        A.row = nnet->bias[layer].row;
        A.col = nnet->bias[layer].col;
        memcpy(A.data, nnet->bias[layer].data, A.row*A.col*sizeof(float));

        matmul_with_bias(&Z, &nnet->weights[layer], &A);
        if(layer<numLayers-1){
            relu(&A);
        }
        memcpy(Z.data, A.data, A.row*A.col*sizeof(float));
        Z.row = A.row;
        Z.col = A.col;
        
    }

    memcpy(output->data, A.data, A.row*A.col*sizeof(float));
    output->row = A.row;
    output->col = A.col;

    return 1;
}


/*
 * Concrete forward propagation with for loops
 * It takes in network and concrete input matrix.
 * Outputs the concrete outputs.
 */
int evaluate(struct NNet *network, struct Matrix *input, struct Matrix *output)
{

    int i,j,layer;

    struct NNet* nnet = network;
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;

    float ****matrix = nnet->matrix;

    float tempVal;
    float z[nnet->maxLayerSize];
    float a[nnet->maxLayerSize];

    for (i=0;i<nnet->inputSize;i++) {
        z[i] = input->data[i];
    }

    for (layer = 0;layer<numLayers;layer++) {

        for (i=0;i<nnet->layerSizes[layer+1];i++) {
            float **weights = matrix[layer][0];
            float **biases  = matrix[layer][1];
            tempVal = 0.0;

            for (j=0;j<nnet->layerSizes[layer];j++) {
                tempVal += z[j]*weights[i][j];

            }

            tempVal += biases[i][0];

            //Perform ReLU
            if (tempVal < 0.0 && layer < (numLayers - 1)) {
                // printf( "doing RELU on layer %u\n", layer );
                tempVal = 0.0;
            }

            a[i]=tempVal;
        }

        for (j=0;j<nnet->maxLayerSize;j++) {
            z[j] = a[j];
        }

    }

    for (i=0; i<outputSize; i++) {
        output->data[i] = a[i];
    }
    
    return 1;

}


/*
 * Naive interval propagation with for loops.
 * It takes in network and input interval.
 * Outputs the estimated output range. 
 */
int evaluate_interval(struct NNet *network,\
                    struct Interval *input,\
                    struct Interval *output)
{

    int i,j,layer;

    struct NNet* nnet = network;
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;

    float ****matrix = nnet->matrix;

    float tempVal_upper, tempVal_lower;
    float z_upper[nnet->maxLayerSize];
    float z_lower[nnet->maxLayerSize];
    float a_upper[nnet->maxLayerSize];
    float a_lower[nnet->maxLayerSize];

    for (i=0;i < nnet->inputSize;i++) {
        z_upper[i] = input->upper_matrix.data[i];
        z_lower[i] = input->lower_matrix.data[i];
    }

    for (layer = 0;layer<(numLayers);layer++) {

        for (i=0;i<nnet->layerSizes[layer+1];i++) {
            float **weights = matrix[layer][0];
            float **biases  = matrix[layer][1];
            tempVal_upper = tempVal_lower = 0.0;

            for (j=0;j<nnet->layerSizes[layer];j++) {

                if (weights[i][j] >= 0) {
                    tempVal_upper += z_upper[j]*weights[i][j];
                    tempVal_lower += z_lower[j]*weights[i][j];
                }
                else {
                    tempVal_upper += z_lower[j]*weights[i][j];
                    tempVal_lower += z_upper[j]*weights[i][j];
                }

            }

            tempVal_lower += biases[i][0];
            tempVal_upper += biases[i][0];

            if (layer < (numLayers - 1)) {

                if (tempVal_lower < 0.0){
                    tempVal_lower = 0.0;
                }

                if (tempVal_upper < 0.0){ 
                    tempVal_upper = 0.0;
                }

            }

            a_upper[i] = tempVal_upper;
            a_lower[i] = tempVal_lower;

        }

        for (j=0;j<nnet->maxLayerSize;j++) {
            z_upper[j] = a_upper[j];
            z_lower[j] = a_lower[j];
        }

    }

    for (i=0;i<outputSize;i++) {
        output->upper_matrix.data[i] = a_upper[i];
        output->lower_matrix.data[i] = a_lower[i];
    }

    return 1;
}


/*
 * Symbolic interval propagation with for loops.
 * It takes in network and input interval.
 * Outputs the estimated output range. 
 */
int evaluate_interval_equation(struct NNet *network,\
                            struct Interval *input,\
                            struct Interval *output)
{

    int i,j,k,layer;

    struct NNet* nnet = network;
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;
    int maxLayerSize   = nnet->maxLayerSize;

    float ****matrix = nnet->matrix;

    // equation is the temp equation for each layer
    float equation_upper[maxLayerSize][inputSize+1];
    float equation_lower[maxLayerSize][inputSize+1];
    float new_equation_upper[maxLayerSize][inputSize+1];
    float new_equation_lower[maxLayerSize][inputSize+1];

    memset(equation_upper,0,sizeof(float)*\
                        (inputSize+1)*maxLayerSize);
    memset(equation_lower,0,sizeof(float)*\
                        (inputSize+1)*maxLayerSize);

    float tempVal_upper, tempVal_lower;

    for (i=0;i<nnet->inputSize;i++) {
        equation_lower[i][i] = 1;
        equation_upper[i][i] = 1;
    }

    for (layer=0;layer<(numLayers);layer++) {

        for (i=0;i<maxLayerSize;i++) {
            memset(new_equation_upper, 0, sizeof(float)*\
                                (inputSize+1)*maxLayerSize);
            memset(new_equation_lower, 0, sizeof(float)*\
                                (inputSize+1)*maxLayerSize);
        }

        for (i=0;i<nnet->layerSizes[layer+1];i++) {
            float **weights = matrix[layer][0];
            float **biases  = matrix[layer][1];

            tempVal_upper = tempVal_lower = 0.0;

            for (j=0;j<nnet->layerSizes[layer];j++) {

                for (k=0;k<inputSize+1;k++) {

                    if (weights[i][j] >= 0) {
                        new_equation_upper[i][k] += equation_upper[j][k]*\
                                            weights[i][j];
                        new_equation_lower[i][k] += equation_lower[j][k]*\
                                            weights[i][j];
                    }
                    else {
                        new_equation_upper[i][k] += equation_lower[j][k]*\
                                            weights[i][j];
                        new_equation_lower[i][k] += equation_upper[j][k]*\
                                            weights[i][j];
                    }

                }

            }

            for (k=0;k<inputSize;k++) {

                if (new_equation_lower[i][k] >= 0) {
                    tempVal_lower += new_equation_lower[i][k] *\
                                            input->lower_matrix.data[k];
                }
                else {
                    tempVal_lower += new_equation_lower[i][k] *\
                                            input->upper_matrix.data[k];
                }

                if (new_equation_upper[i][k] >= 0) {
                    tempVal_upper += new_equation_upper[i][k] *\
                                            input->upper_matrix.data[k];
                }
                else {
                    tempVal_upper += new_equation_upper[i][k] *\
                                            input->lower_matrix.data[k];
                }

            }

            new_equation_lower[i][inputSize] += biases[i][0];
            new_equation_upper[i][inputSize] += biases[i][0];

            tempVal_lower += new_equation_lower[i][inputSize];
            tempVal_upper += new_equation_upper[i][inputSize];

            /* Perform ReLU */
            if (layer < (numLayers - 1)) {

                if (tempVal_lower < 0.0) {
                    tempVal_lower = 0.0;

                    memset(new_equation_upper[i], 0,\
                                sizeof(float)*(inputSize+1));
                    memset(new_equation_lower[i], 0,\
                                sizeof(float)*(inputSize+1));

                    new_equation_upper[i][inputSize] = tempVal_upper;
                }

                if (tempVal_upper < 0.0){
                    tempVal_upper = 0.0;

                    memset(new_equation_upper[i], 0,\
                                sizeof(float)*(inputSize+1));
                    memset(new_equation_lower[i], 0,\
                                sizeof(float)*(inputSize+1));
                }
            }
            else {
                output->upper_matrix.data[i] = tempVal_upper;
                output->lower_matrix.data[i] = tempVal_lower;
            }

        }

        memcpy(equation_upper, new_equation_upper, sizeof(float)*\
                                (inputSize+1)*maxLayerSize);
        memcpy(equation_lower, new_equation_lower, sizeof(float)*\
                                (inputSize+1)*maxLayerSize);

    }

    return 1;

}


/*
 * Backward propagation to calculate the gradient ranges of inputs.
 * Takes in network and gradient masks.
 * Outputs input gradient ranges.
 */
void backward_prop(struct NNet *nnet,\
                struct Interval *grad,\
                int R[][nnet->maxLayerSize])
{

    int i, j, layer;
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;
    int maxLayerSize   = nnet->maxLayerSize;

    float grad_upper[maxLayerSize];
    float grad_lower[maxLayerSize];
    float grad1_upper[maxLayerSize];
    float grad1_lower[maxLayerSize];

    memcpy(grad_upper, nnet->matrix[numLayers-1][0][nnet->target],\
                    sizeof(float)*nnet->layerSizes[numLayers-1]);
    memcpy(grad_lower, nnet->matrix[numLayers-1][0][nnet->target],\
                    sizeof(float)*nnet->layerSizes[numLayers-1]);

    for (layer=numLayers-2;layer>-1;layer--){ 
        float **weights = nnet->matrix[layer][0];
        memset(grad1_upper, 0, sizeof(float)*nnet->layerSizes[layer]);
        memset(grad1_lower, 0, sizeof(float)*nnet->layerSizes[layer]);

        for (j=0;j<nnet->layerSizes[layer+1];j++) {

            if(R[layer][j] == 0){
                grad_upper[j] = grad_lower[j] = 0;
            }
            else if (R[layer][j] == 1) {
                grad_upper[j] = (grad_upper[j]>0)?grad_upper[j]:0;
                grad_lower[j] = (grad_lower[j]<0)?grad_lower[j]:0;
            }

            for (i=0;i<nnet->layerSizes[layer];i++) {
                
                if (weights[j][i] >= 0) {
                    grad1_upper[i] += weights[j][i]*grad_upper[j];
                    grad1_lower[i] += weights[j][i]*grad_lower[j];
                }
                else {
                    grad1_upper[i] += weights[j][i]*grad_lower[j];
                    grad1_lower[i] += weights[j][i]*grad_upper[j];
                }
            }
        }

        if (layer != 0) {
            memcpy(grad_upper,grad1_upper,sizeof(float)*nnet->layerSizes[layer]);
            memcpy(grad_lower,grad1_lower,sizeof(float)*nnet->layerSizes[layer]);
        }
        else {
            memcpy(grad->lower_matrix.data, grad1_lower,\
                                        sizeof(float)*inputSize);
            memcpy(grad->upper_matrix.data, grad1_upper,\
                                        sizeof(float)*inputSize);
        }
    }

}


/*
 * Symbolic interval propagation with for openblas.
 * Takes in network and input interval.
 * Outputs the estimated output range. 
 */
int forward_prop_interval_equation(struct NNet *network,\
                                struct Interval *input,\
                                struct Interval *output,\
                                struct Interval *grad)
{

    int i,j,k,layer;

    struct NNet* nnet = network;
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;
    int maxLayerSize   = nnet->maxLayerSize;

    int R[numLayers][maxLayerSize];
    memset(R, 0, sizeof(float)*numLayers*maxLayerSize);

    /* equation is the temp equation for each layer */
    float equation_upper[(inputSize+1)*maxLayerSize];
    float equation_lower[(inputSize+1)*maxLayerSize];
    float new_equation_upper[(inputSize+1)*maxLayerSize];
    float new_equation_lower[(inputSize+1)*maxLayerSize];

    memset(equation_upper,0,sizeof(float)*\
                        (inputSize+1)*maxLayerSize);
    memset(equation_lower,0,sizeof(float)*\
                        (inputSize+1)*maxLayerSize);

    struct Interval equation_inteval = {
            (struct Matrix){(float*)equation_lower,\
            inputSize+1, inputSize},
            (struct Matrix){(float*)equation_upper,\
            inputSize+1, inputSize}
        };

    struct Interval new_equation_inteval = {
            (struct Matrix){(float*)new_equation_lower,\
            inputSize+1, maxLayerSize},\
            (struct Matrix){(float*)new_equation_upper,\
            inputSize+1, maxLayerSize}
        };                                       

    float tempVal_upper=0.0, tempVal_lower=0.0;
    float upper_s_lower=0.0;

    for (i=0;i < nnet->inputSize;i++) {
        equation_lower[i*(inputSize+1)+i] = 1;
        equation_upper[i*(inputSize+1)+i] = 1;
    }

    for (layer=0;layer<(numLayers);layer++) {
        
        memset(new_equation_upper, 0, sizeof(float)*(inputSize+1)*maxLayerSize);
        memset(new_equation_lower, 0, sizeof(float)*(inputSize+1)*maxLayerSize);
        
        struct Matrix weights = nnet->weights[layer];
        struct Matrix bias = nnet->bias[layer];

        float p[weights.col*weights.row];
        float n[weights.col*weights.row];

        memset(p, 0, sizeof(float)*weights.col*weights.row);
        memset(n, 0, sizeof(float)*weights.col*weights.row);

        struct Matrix pos_weights = {p, weights.row, weights.col};
        struct Matrix neg_weights = {n, weights.row, weights.col};

        for (i=0;i<weights.row*weights.col;i++) {

            if(weights.data[i] >= 0){
                p[i] = weights.data[i];
            }
            else{
                n[i] = weights.data[i];
            }

        }

        matmul(&equation_inteval.upper_matrix, &pos_weights,\
                        &new_equation_inteval.upper_matrix);
        matmul_with_bias(&equation_inteval.lower_matrix, &neg_weights,\
                        &new_equation_inteval.upper_matrix);

        matmul(&equation_inteval.lower_matrix, &pos_weights,\
                        &new_equation_inteval.lower_matrix);
        matmul_with_bias(&equation_inteval.upper_matrix, &neg_weights,\
                        &new_equation_inteval.lower_matrix);
        
        for (i=0; i < nnet->layerSizes[layer+1]; i++)
        {
            tempVal_upper = tempVal_lower = 0.0;

            if (NEED_OUTWARD_ROUND) {

                for(k=0;k<inputSize;k++){

                    if (new_equation_lower[k+i*(inputSize+1)] >= 0) {
                        tempVal_lower += new_equation_lower[k+i*(inputSize+1)] *\
                                    input->lower_matrix.data[k] - OUTWARD_ROUND;
                    }
                    else {
                        tempVal_lower += new_equation_lower[k+i*(inputSize+1)] *\
                                    input->upper_matrix.data[k] - OUTWARD_ROUND;
                    }

                    if (new_equation_upper[k+i*(inputSize+1)] >= 0) {
                        tempVal_upper += new_equation_upper[k+i*(inputSize+1)] *\
                                    input->upper_matrix.data[k] + OUTWARD_ROUND;
                        upper_s_lower += new_equation_upper[k+i*(inputSize+1)] *\
                                    input->lower_matrix.data[k] + OUTWARD_ROUND;
                    }
                    else {
                        tempVal_upper += new_equation_upper[k+i*(inputSize+1)] *\
                                    input->lower_matrix.data[k] + OUTWARD_ROUND;
                        upper_s_lower += new_equation_upper[k+i*(inputSize+1)] *\
                                    input->upper_matrix.data[k] + OUTWARD_ROUND;
                    }

                }

            }
            else {

                for (k=0;k<inputSize;k++) {

                    if (new_equation_lower[k+i*(inputSize+1)] >= 0) {
                        tempVal_lower += new_equation_lower[k+i*(inputSize+1)] *\
                                    input->lower_matrix.data[k];
                    }
                    else {
                        tempVal_lower += new_equation_lower[k+i*(inputSize+1)] *\
                                    input->upper_matrix.data[k];
                    }

                    if (new_equation_upper[k+i*(inputSize+1)] >= 0) {
                        tempVal_upper += new_equation_upper[k+i*(inputSize+1)] *\
                                    input->upper_matrix.data[k];
                    }
                    else {
                        tempVal_upper += new_equation_upper[k+i*(inputSize+1)] *\
                                    input->lower_matrix.data[k];
                    }

                }

            }
            
            new_equation_lower[inputSize+i*(inputSize+1)] += bias.data[i];
            new_equation_upper[inputSize+i*(inputSize+1)] += bias.data[i];

            tempVal_lower += new_equation_lower[inputSize+i*(inputSize+1)];
            tempVal_upper += new_equation_upper[inputSize+i*(inputSize+1)];
            upper_s_lower += new_equation_upper[inputSize+i*(inputSize+1)];

            /* Perform ReLU */
            if (layer < (numLayers - 1)) {

                if (tempVal_upper <= 0.0) {
                    tempVal_upper = 0.0;

                    for(k=0;k<inputSize+1;k++){
                        new_equation_upper[k+i*(inputSize+1)] = 0;
                        new_equation_lower[k+i*(inputSize+1)] = 0;
                    }

                    R[layer][i] = 0;
                }
                else if (tempVal_lower >= 0.0) {
                    R[layer][i] = 2;
                }
                else {
                    tempVal_lower = 0.0;
                    //printf("wrong node: ");

                    if (upper_s_lower >= 0.0) {
                        //printf("upper's lower >= 0\n");

                        for (k=0;k<inputSize+1;k++) {
                            new_equation_lower[k+i*(inputSize+1)] = 0;
                        }

                    }
                    else {
                        //printf("upper's lower <= 0\n");

                        for (k=0;k<inputSize+1;k++) {
                            new_equation_lower[k+i*(inputSize+1)] = 0;
                            new_equation_upper[k+i*(inputSize+1)] = 0;
                        }

                        new_equation_upper[inputSize+i*(inputSize+1)] =\
                                    tempVal_upper;
                        //new_equation_upper[inputSize+i*(inputSize+1)] -= upper_s_lower;
                    }

                    R[layer][i] = 1;
                }

            }
            else {
                output->upper_matrix.data[i] = tempVal_upper;
                output->lower_matrix.data[i] = tempVal_lower;
            }

        }

        //printf("\n");
        memcpy(equation_upper, new_equation_upper,\
                sizeof(float)*(inputSize+1)*maxLayerSize);
        memcpy(equation_lower, new_equation_lower,\
                sizeof(float)*(inputSize+1)*maxLayerSize);

        equation_inteval.lower_matrix.row = equation_inteval.upper_matrix.row\
                    =  new_equation_inteval.lower_matrix.row;
        equation_inteval.lower_matrix.col = equation_inteval.upper_matrix.col\
                    = new_equation_inteval.lower_matrix.col;
    }

    backward_prop(nnet, grad, R);

    return 1;

}


// new propagation method which can make the verification much faster! 
int forward_prop_interval_equation_linear2(struct NNet *network,\
            struct Interval *input, struct Interval *output,\
            struct Interval *grad)
{
    int i,j,k,layer;

    struct NNet* nnet = network;
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;
    int maxLayerSize   = nnet->maxLayerSize;

    int R[numLayers][maxLayerSize];
    memset(R, 0, sizeof(float)*numLayers*maxLayerSize);

    // equation is the temp equation for each layer
    float equation_upper[(inputSize+1)*maxLayerSize];
    float equation_lower[(inputSize+1)*maxLayerSize];
    float new_equation_upper[(inputSize+1)*maxLayerSize];
    float new_equation_lower[(inputSize+1)*maxLayerSize];
    memset(equation_upper,0,sizeof(float)*(inputSize+1)*maxLayerSize);
    memset(equation_lower,0,sizeof(float)*(inputSize+1)*maxLayerSize);

    struct Interval equation_inteval = {
                (struct Matrix){(float*)equation_lower, inputSize+1, inputSize},
                (struct Matrix){(float*)equation_upper, inputSize+1, inputSize}
            };
    struct Interval new_equation_inteval = {
                (struct Matrix){(float*)new_equation_lower, inputSize+1, maxLayerSize},
                (struct Matrix){(float*)new_equation_upper, inputSize+1, maxLayerSize}
            };                                       

    float tempVal_upper=0.0, tempVal_lower=0.0;
    float upper_s_lower=0.0, lower_s_upper=0.0;
    for (i=0; i < nnet->inputSize; i++)
    {
        equation_lower[i*(inputSize+1)+i] = 1;
        equation_upper[i*(inputSize+1)+i] = 1;
    }

    for (layer = 0; layer<(numLayers); layer++)
    {

        memset(new_equation_upper, 0, sizeof(float)*(inputSize+1)*maxLayerSize);
        memset(new_equation_lower, 0, sizeof(float)*(inputSize+1)*maxLayerSize);
        
        struct Matrix weights = nnet->weights[layer];
        struct Matrix bias = nnet->bias[layer];
        float p[weights.col*weights.row];
        float n[weights.col*weights.row];
        memset(p, 0, sizeof(float)*weights.col*weights.row);
        memset(n, 0, sizeof(float)*weights.col*weights.row);
        struct Matrix pos_weights = {p, weights.row, weights.col};
        struct Matrix neg_weights = {n, weights.row, weights.col};
        for(i=0;i<weights.row*weights.col;i++){
            if(weights.data[i]>=0){
                p[i] = weights.data[i];
            }
            else{
                n[i] = weights.data[i];
            }
        }

        matmul(&equation_inteval.upper_matrix, &pos_weights, &new_equation_inteval.upper_matrix);
        matmul_with_bias(&equation_inteval.lower_matrix, &neg_weights, &new_equation_inteval.upper_matrix);

        matmul(&equation_inteval.lower_matrix, &pos_weights, &new_equation_inteval.lower_matrix);
        matmul_with_bias(&equation_inteval.upper_matrix, &neg_weights, &new_equation_inteval.lower_matrix);
        
        for (i=0; i < nnet->layerSizes[layer+1]; i++)
        {
            tempVal_upper = tempVal_lower = 0.0;
            lower_s_upper = upper_s_lower = 0.0;

            if(NEED_OUTWARD_ROUND){
                for(k=0;k<inputSize;k++){
                    if(new_equation_lower[k+i*(inputSize+1)]>=0){
                        tempVal_lower += new_equation_lower[k+i*(inputSize+1)] * input->lower_matrix.data[k]-OUTWARD_ROUND;
                        lower_s_upper += new_equation_lower[k+i*(inputSize+1)] * input->upper_matrix.data[k]-OUTWARD_ROUND;
                    }
                    else{
                        tempVal_lower += new_equation_lower[k+i*(inputSize+1)] * input->upper_matrix.data[k]-OUTWARD_ROUND;
                        lower_s_upper += new_equation_lower[k+i*(inputSize+1)] * input->lower_matrix.data[k]-OUTWARD_ROUND;
                    }
                    if(new_equation_upper[k+i*(inputSize+1)]>=0){
                        tempVal_upper += new_equation_upper[k+i*(inputSize+1)] * input->upper_matrix.data[k]+OUTWARD_ROUND;
                        upper_s_lower += new_equation_upper[k+i*(inputSize+1)] * input->lower_matrix.data[k]+OUTWARD_ROUND;
                    }
                    else{
                        tempVal_upper += new_equation_upper[k+i*(inputSize+1)] * input->lower_matrix.data[k]+OUTWARD_ROUND;
                        upper_s_lower += new_equation_upper[k+i*(inputSize+1)] * input->upper_matrix.data[k]+OUTWARD_ROUND;
                    }  
                }
            }
            else{
                for(k=0;k<inputSize;k++){
                    if(layer==0){
                        if(new_equation_lower[k+i*(inputSize+1)]!=new_equation_upper[k+i*(inputSize+1)]){
                            printf("wrong!\n");
                        }
                    }
                    if(new_equation_lower[k+i*(inputSize+1)]>=0){
                        tempVal_lower += new_equation_lower[k+i*(inputSize+1)] * input->lower_matrix.data[k];
                        lower_s_upper += new_equation_lower[k+i*(inputSize+1)] * input->upper_matrix.data[k];
                    }
                    else{
                        tempVal_lower += new_equation_lower[k+i*(inputSize+1)] * input->upper_matrix.data[k];
                        lower_s_upper += new_equation_lower[k+i*(inputSize+1)] * input->lower_matrix.data[k];
                    }
                    if(new_equation_upper[k+i*(inputSize+1)]>=0){
                        tempVal_upper += new_equation_upper[k+i*(inputSize+1)] * input->upper_matrix.data[k];
                        upper_s_lower += new_equation_upper[k+i*(inputSize+1)] * input->lower_matrix.data[k];
                    }
                    else{
                        tempVal_upper += new_equation_upper[k+i*(inputSize+1)] * input->lower_matrix.data[k];
                        upper_s_lower += new_equation_upper[k+i*(inputSize+1)] * input->upper_matrix.data[k];
                    }  
                }
            }
            
            new_equation_lower[inputSize+i*(inputSize+1)] += bias.data[i];
            new_equation_upper[inputSize+i*(inputSize+1)] += bias.data[i];
            tempVal_lower += new_equation_lower[inputSize+i*(inputSize+1)];
            lower_s_upper += new_equation_lower[inputSize+i*(inputSize+1)];
            tempVal_upper += new_equation_upper[inputSize+i*(inputSize+1)];
            upper_s_lower += new_equation_upper[inputSize+i*(inputSize+1)];

            //Perform ReLU
            if(layer<(numLayers-1)){
                if (tempVal_upper<=0.0){
                    tempVal_upper = 0.0;
                    for(k=0;k<inputSize+1;k++){
                        new_equation_upper[k+i*(inputSize+1)] = 0;
                        new_equation_lower[k+i*(inputSize+1)] = 0;
                    }
                    R[layer][i] = 0;
                }
                else if(tempVal_lower>=0.0){
                    R[layer][i] = 2;
                }
                else{
                    if(lower_s_upper>0 || upper_s_lower<0){
                        //printf("%d,%d:%f, %f, %f, %f\n",layer, i, tempVal_lower, lower_s_upper, upper_s_lower, tempVal_upper );
                    }
                    //printf("wrong node: ");
                    if(upper_s_lower<0.0){
                        for(k=0;k<inputSize+1;k++){
                            new_equation_upper[k+i*(inputSize+1)] =\
                                                    new_equation_upper[k+i*(inputSize+1)]*\
                                                    tempVal_upper / (tempVal_upper-upper_s_lower);
                        }
                        new_equation_upper[inputSize+i*(inputSize+1)] -= tempVal_upper*upper_s_lower/\
                                                            (tempVal_upper-upper_s_lower);
                    }

                    if(lower_s_upper<0.0){
                        for(k=0;k<inputSize+1;k++){
                            new_equation_lower[k+i*(inputSize+1)] = 0;
                        }
                    }
                    else{
                        /*
                        if(lower_s_upper<-tempVal_lower){
                            for(k=0;k<inputSize+1;k++){
                                new_equation_lower[k+i*(inputSize+1)] = 0;
                            }
                        }  
                        */
                        for(k=0;k<inputSize+1;k++){
                            new_equation_lower[k+i*(inputSize+1)] =\
                                                    new_equation_lower[k+i*(inputSize+1)]*\
                                                    lower_s_upper / (lower_s_upper- tempVal_lower);
                        }
                        

                    }
                    R[layer][i] = 1;
                }
            }
            else{
                output->upper_matrix.data[i] = tempVal_upper;
                output->lower_matrix.data[i] = tempVal_lower;
            }
        }
        //printf("\n");
        memcpy(equation_upper, new_equation_upper, sizeof(float)*(inputSize+1)*maxLayerSize);
        memcpy(equation_lower, new_equation_lower, sizeof(float)*(inputSize+1)*maxLayerSize);
        equation_inteval.lower_matrix.row = equation_inteval.upper_matrix.row =\
                                                         new_equation_inteval.lower_matrix.row;
        equation_inteval.lower_matrix.col = equation_inteval.upper_matrix.col =\
                                                         new_equation_inteval.lower_matrix.col;
    }

    backward_prop(nnet, grad, R);

    return 1;
}
