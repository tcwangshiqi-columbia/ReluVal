#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "nnet.h"


const char *FULL_NET_PATH = "./nnet/ACASXU_run2a_1_1_batch_2000.nnet";
int main( int argc, char **argv ){

    NNet *nnet = new NNet();
    printf("begin\n");
    nnet = load_network("ACASXU_run2a_1_1_batch_2000.nnet");

    double input[5] = {250,0.2,-3.1415926,100,400};
    struct Matrix in = {input, 1, 5};
    double output[5];

    int i,j,layer;

    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;
    int symmetric    = nnet->symmetric;
    
    struct Matrix weights[7];
    struct Matrix bias[7];
    
    for(int layer=0;layer<nnet->numLayers;layer++){
        weights[layer].row = nnet->layerSizes[layer];
        weights[layer].col = nnet->layerSizes[layer+1];
        weights[layer].matrix = (double*)malloc(sizeof(double)*weights[layer].row * weights[layer].col);
        int n=0;
        for(int i=0;i<weights[layer].col;i++){
            for(int j=0;j<weights[layer].row;j++){
                weights[layer].matrix[n] = nnet->matrix[layer][0][i][j];
                n++;
            }
        }
        bias[layer].row = nnet->layerSizes[layer+1];
        bias[layer].col = double(1);
        bias[layer].matrix = (double*)malloc(sizeof(double)*bias[layer].row);
        for(int i=0;i<bias[layer].row;i++){
            bias[layer].matrix[i] = nnet->matrix[layer][1][i][0];
        }
    } 
    
    double ****matrix = nnet->matrix;
    printf("%lf\n", weights[0].matrix[7]);
    //Normalize inputs

    for (i=0; i<inputSize;i++)
    {
        if (input[i]>nnet->maxes[i])
        {
            nnet->inputs[i] = (nnet->maxes[i]-nnet->means[i])/(nnet->ranges[i]);
        }
        else if (input[i]<nnet->mins[i])
        {
            nnet->inputs[i] = (nnet->mins[i]-nnet->means[i])/(nnet->ranges[i]);
        }
        else
        {
            nnet->inputs[i] = (input[i]-nnet->means[i])/(nnet->ranges[i]);
        }
    }
    if (symmetric==1 && nnet->inputs[2]<0)
    {
        nnet->inputs[2] = -nnet->inputs[2]; //Make psi positive
        nnet->inputs[1] = -nnet->inputs[1]; //Flip across x-axis
    } else {
        symmetric = 0;
    }
 

    double tempVal;
    // printf("number of layers = %u\n", numLayers);

    for (layer = 0; layer<(numLayers); layer++)
    {
        for (i=0; i < nnet->layerSizes[layer+1]; i++)
        {
            double **weights = matrix[layer][0];
            double **biases  = matrix[layer][1];
            tempVal = 0.0;

            //Perform weighted summation of inputs
            for (j=0; j<nnet->layerSizes[layer]; j++)
            {
                tempVal += nnet->inputs[j]*weights[i][j];

            }

            //Add bias to weighted sum
            tempVal += biases[i][0];

            //Perform ReLU
            if (tempVal<0.0 && layer<(numLayers-1))
            {
                // printf( "doing RELU on layer %u\n", layer );
                tempVal = 0.0;
            }
            nnet->temp[i]=tempVal;
        }

        //Output of one layer is the input to the next layer
        for (i=0; i < nnet->layerSizes[layer+1]; i++)
        {
            nnet->inputs[i] = nnet->temp[i];
        }
    }

    //Write the final output value to the allocated spot in memory
    for (i=0; i<outputSize; i++)
    {
        if ( false )
            output[i] = nnet->inputs[i]*nnet->ranges[nnet->inputSize]+nnet->means[nnet->inputSize];
        else
            output[i] = nnet->inputs[i];
    }
    printf("%f\n", output[4]);

    printf("end\n");
    /*
    for(int layer=0;layer<numLayers;layer++){
        free(weights[layer].matrix);
        free(bias[layer].matrix);
    }
    */ 
}