#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "split.h"

//extern int thread_tot_cnt;

void sig_handler(int signo)
{
    if (signo == SIGQUIT){
        printf("progress: %d/1024\n", progress);

    }
}

int main( int argc, char *argv[]){
    //char *FULL_NET_PATH = "nnet/ACASXU_run2a_1_1_batch_2000.nnet";
    char *FULL_NET_PATH;
    int target = 0;
    if(argc>8 || argc<4) {
        printf("please specify a network\n");
        printf("./network_test [property] [network] [target] [print] [test for one run] [check mode]\n");
        exit(1);
    }
    for(int i=1;i<argc;i++){
        if(i==1){
            PROPERTY = atoi(argv[i]); 
            if(PROPERTY<0){
                printf("Wrong input depth");
                exit(1);
            } 
        }
        if(i==2){
            FULL_NET_PATH = argv[i];
        }
        if(i==3){
            target = atoi(argv[i]);
        }
        if(i==4){
            NEED_PRINT = atoi(argv[i]);
            if(NEED_PRINT != 0 && NEED_PRINT!=1){
                printf("Wrong print");
                exit(1);
            }
        }
        if(i==5){
            NEED_FOR_ONE_RUN = atoi(argv[i]); 
            if(NEED_FOR_ONE_RUN != 0 && NEED_FOR_ONE_RUN!=1){
                printf("Wrong test for one run");
                exit(1);
            }
        }
        if(i==6){
            if(atoi(argv[i])==0){
                CHECK_ADV_MODE = 0;
                PARTIAL_MODE = 0;
            }
            if(atoi(argv[i])==1){
                CHECK_ADV_MODE = 1;
                PARTIAL_MODE = 0;
            }
            if(atoi(argv[i])==2){
                CHECK_ADV_MODE = 0;
                PARTIAL_MODE = 1;
            }
        }
    }

    //fp = fopen(LOG_FILE, "a");

    openblas_set_num_threads(1);

    //clock_t start, end;
    srand((unsigned)time(NULL));
    double time_spent;
    int i,j,layer;

    struct NNet* nnet = load_network(FULL_NET_PATH, target);
    
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;
    int maxLayerSize = nnet->maxLayerSize;

    float u[inputSize], l[inputSize], input_prev[inputSize];
    struct Matrix input_prev_matrix = {input_prev, 1, inputSize};

    struct Matrix input_upper = {u,1,nnet->inputSize};
    struct Matrix input_lower = {l,1,nnet->inputSize};
    struct Interval input_interval = {input_lower, input_upper};

    initialize_input_interval(PROPERTY, inputSize, input_prev, u, l);

    normalize_input(nnet, &input_prev_matrix);
    normalize_input_interval(nnet, &input_interval);  

    float grad_upper[inputSize], grad_lower[inputSize];
    struct Interval grad_interval = {(struct Matrix){grad_upper, 1, inputSize},
                                     (struct Matrix){grad_lower, 1, inputSize}};

    float o[nnet->outputSize];
    struct Matrix output = {o, outputSize, 1};
    
    float o_upper[nnet->outputSize], o_lower[nnet->outputSize];
    struct Interval output_interval = {(struct Matrix){o_lower, outputSize, 1},
                                       (struct Matrix){o_upper, outputSize, 1}};

    int n = 0;
    int feature_range_length = 0;
    int split_feature = -1;
    int depth = 0;
    printf("running image %d with network %s\n", PROPERTY, FULL_NET_PATH);
    printf("Infinite Norm: %d\n", INF);
    //printMatrix(&input_upper);
    //printMatrix(&input_lower);
    for(int i=0;i<inputSize;i++){
        if(input_interval.upper_matrix.data[i]<input_interval.lower_matrix.data[i]){
            printf("wrong input!\n");
            exit(0);
        }
        if(input_interval.upper_matrix.data[i]!=input_interval.lower_matrix.data[i]){
            n++;
        }
    }
    feature_range_length = n;
    int *feature_range = (int*)malloc(n*sizeof(int));
    for(int i=0, n=0;i<nnet->inputSize;i++){
        if(input_interval.upper_matrix.data[i]!=input_interval.lower_matrix.data[i]){
            feature_range[n] = i;
            n++;
        }
    }

    float *equation_upper = (float*)malloc(sizeof(float)*(inputSize+1)*maxLayerSize);
    float *equation_lower = (float*)malloc(sizeof(float)*(inputSize+1)*maxLayerSize);
    float *new_equation_upper = (float*)malloc(sizeof(float)*(inputSize+1)*maxLayerSize);
    float *new_equation_lower = (float*)malloc(sizeof(float)*(inputSize+1)*maxLayerSize);
    
    //forward_prop(nnet, &input_prev_matrix, &output);
    //evaluate(nnet, &input_prev_matrix, &output);
    //printMatrix(&output);
    evaluate_interval(nnet, &input_interval,&output_interval);
    //printMatrix(&output);
    printMatrix(&output_interval.upper_matrix);
    printMatrix(&output_interval.lower_matrix);

    
    gettimeofday(&start, NULL);
    int isOverlap = 0;

    for(int i=0;i<1;i++){
        forward_prop_interval_equation(nnet, &input_interval, &output_interval, &grad_interval, equation_upper, equation_lower, new_equation_upper, new_equation_lower);
        //evaluate_interval_equation(nnet, &input_interval, &output_interval);
    }
    //forward_prop_interval_equation(nnet, &input_interval, &output_interval, &grad_interval, equation_upper, equation_lower, new_equation_upper, new_equation_lower);
    //evaluate_interval_equation(nnet, &input_interval, &output_interval);
    printMatrix(&output_interval.upper_matrix);
    printMatrix(&output_interval.lower_matrix);

    /*
    if(CHECK_ADV_MODE){
        printf("CHECK_ADV_MODE\n");
        isOverlap = direct_run_check(nnet, &input_interval, &output_interval, 
                               &grad_interval, depth, feature_range, 
                               feature_range_length, split_feature);
    
    }
    else{
        printf("No CHECK_ADV_MODE\n");
        for(int i=0;i<1;i++){
            //forward_prop_interval_equation(nnet, &input_interval, &output_interval, &grad_interval);
            isOverlap = direct_run_check(nnet, &input_interval, &output_interval, 
                               &grad_interval, depth, feature_range, 
                               feature_range_length, split_feature);
        } 
    }
    */
    gettimeofday(&finish, NULL);
    time_spent = ((float)(finish.tv_sec-start.tv_sec)*1000000 +\
                 (float)(finish.tv_usec-start.tv_usec)) / 1000000;
    if(isOverlap==0 && adv_found == 0){
        printf("No adv!\n");
    }

    
    printf("time: %f \n", time_spent);
    destroy_network(nnet);
    free(feature_range);

    free(equation_upper);
    free(equation_lower);
    free(new_equation_upper);
    free(new_equation_lower);
}
