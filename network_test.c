/*
 ------------------------------------------------------------------
 ** Top contributors:
 **   Shiqi Wang and Suman Jana
 ** This file is part of the ReluVal project.
 ** Copyright (c) 2018-2019 by the authors listed in the file LICENSE
 ** and their institutional affiliations.
 ** All rights reserved.
 -----------------------------------------------------------------
 *
 * This is the main file of ReluVal, here is the usage:
 * ./network_test [property] [network] [target] 
 *      [need to print=0] [test for one run=0] [check mode=0]
 *
 * [property]: the saftety property want to verify
 *
 * [network]: the network want to test with
 *
 * [target]: Wanted label of the property
 *
 * [need to print]: whether need to print the detailed info of each split.
 * 0 is not and 1 is yes. Default value is 0.
 *
 * [test for one run]: whether need to estimate the output range without
 * split. 0 is no, 1 is yes. Default value is 0.
 *
 * [check mode]: normal split mode is 0. Check adv mode is 1.
 * Check adv mode will prevent further splits as long as the depth goes
 * upper than 20 so as to locate the concrete adversarial examples faster.
 * Default value is 0.
 * 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "split.h"

//extern int thread_tot_cnt;

/* print the progress if getting SIGQUIT */
void sig_handler(int signo)
{

    if (signo == SIGQUIT) {
        printf("progress: %d/1024\n", progress);
    }

}



int main( int argc, char *argv[])
{

    //char *FULL_NET_PATH =\
            "nnet/ACASXU_run2a_1_1_batch_2000.nnet";
    char *FULL_NET_PATH;

    int target = 0;

    if (argc > 9 || argc < 4) {
        printf("please specify a network\n");
        printf("./network_test [property] [network] "
            "[target] [print] "
            "[test for one run] [check mode]\n");
        exit(1);
    }

    for (int i=1;i<argc;i++) {

        if (i == 1) {
            PROPERTY = atoi(argv[i]); 
            if(PROPERTY<0){
                printf("No such property defined");
                exit(1);
            } 
        }

        if (i == 2) {
            FULL_NET_PATH = argv[i];
        }

        if (i == 3) {
            target = atoi(argv[i]);
        }

        if (i == 4) {
            NEED_PRINT = atoi(argv[i]);
            if(NEED_PRINT != 0 && NEED_PRINT!=1){
                printf("Wrong print");
                exit(1);
            }
        }

        if (i == 5) {
            NEED_FOR_ONE_RUN = atoi(argv[i]);

            if (NEED_FOR_ONE_RUN != 0 && NEED_FOR_ONE_RUN != 1) {
                printf("Wrong test for one run");
                exit(1);
            }

        }

        if (i == 6) {

            if (atoi(argv[i]) == 0) {
                CHECK_ADV_MODE = 0;
                PARTIAL_MODE = 0;
            }

            if (atoi(argv[i]) == 1) {
                CHECK_ADV_MODE = 1;
                PARTIAL_MODE = 0;
            }

            if (atoi(argv[i]) == 2) {
                CHECK_ADV_MODE = 0;
                PARTIAL_MODE = 1;
            }

        }

    }

    openblas_set_num_threads(1);

    //clock_t start, end;
    srand((unsigned)time(NULL));
    double time_spent;
    int i,j,layer;

    struct NNet* nnet = load_network(FULL_NET_PATH, target);
    
    int numLayers    = nnet->numLayers;
    int inputSize    = nnet->inputSize;
    int outputSize   = nnet->outputSize;
    
    float input_test[] = {-0.324484, -0.453125,\
                -0.492187, 0.390625, -0.257812};
    //float input_test[] = {59261.709517, -0.046875,\
                1.857353,1145.000000,8.654796};

    struct Matrix input_t = {input_test, 1, 5};
    float u[inputSize], l[inputSize];
    load_inputs(PROPERTY, inputSize, u, l);

    struct Matrix input_upper = {u,1,nnet->inputSize};
    struct Matrix input_lower = {l,1,nnet->inputSize};

    struct Interval input_interval = {input_lower, input_upper};

    float grad_upper[inputSize], grad_lower[inputSize];
    struct Interval grad_interval = {
                (struct Matrix){grad_upper, 1, inputSize},
                (struct Matrix){grad_lower, 1, inputSize}
            };

    //normalize_input(nnet, &input_t);
    normalize_input_interval(nnet, &input_interval);

    float o[nnet->outputSize];
    struct Matrix output = {o, outputSize, 1};

    float o_upper[nnet->outputSize], o_lower[nnet->outputSize];
    struct Interval output_interval = {
                (struct Matrix){o_lower, outputSize, 1},
                (struct Matrix){o_upper, outputSize, 1}
            };

    //if (signal(SIGQUIT, sig_handler) == SIG_ERR)
        //printf("\ncan't catch SIGQUIT\n");

    int n = 0;
    int feature_range_length = 0;
    int split_feature = -1;
    int depth = 0;

    printf("running property %d with network %s\n",\
                PROPERTY, FULL_NET_PATH);
    printf("input ranges:\n");

    printMatrix(&input_upper);
    printMatrix(&input_lower);

    for (int i=0;i<inputSize;i++) {

        if (input_interval.upper_matrix.data[i] <\
                input_interval.lower_matrix.data[i]) {
            printf("wrong input!\n");
            exit(0);
        }

        if(input_interval.upper_matrix.data[i] !=\
                input_interval.lower_matrix.data[i]){
            n++;
        }

    }

    feature_range_length = n;
    int *feature_range = (int*)malloc(n*sizeof(int));

    for (int i=0, n=0;i<nnet->inputSize;i++) {
        if(input_interval.upper_matrix.data[i] !=\
                input_interval.lower_matrix.data[i]){
            feature_range[n] = i;
            n++;
        }
    }

    //evaluate(nnet, &input_t, &output);
    //forward_prop(nnet, &input_t,&output);
    //printMatrix(&output);
    
    gettimeofday(&start, NULL);
    int isOverlap = 0;
    float avg[100] = {0};

    if (CHECK_ADV_MODE) {
        printf("check mode: CHECK_ADV_MODE\n");
        isOverlap = direct_run_check(nnet,\
                &input_interval, &output_interval,\
                &grad_interval, depth, feature_range,\
                feature_range_length, split_feature);
    
    }
    else {
        printf("check mode: NORMAL_CHECK_MODE\n");

        for (int i=0;i<1;i++) {
            //forward_prop_interval_equation(nnet,\
                    &input_interval, &output_interval,\
                    &grad_interval);
            isOverlap = direct_run_check(nnet,\
                    &input_interval, &output_interval,\
                    &grad_interval, depth, feature_range,\
                    feature_range_length, split_feature);
        }

    }
    
    gettimeofday(&finish, NULL);
    time_spent = ((float)(finish.tv_sec - start.tv_sec) *\
            1000000 + (float)(finish.tv_usec - start.tv_usec)) /\
            1000000;

    if (isOverlap == 0 && adv_found == 0) {
        printf("\nNo adv!\n");
    }

    
    printf("time: %f \n", time_spent);
    printf("max simultaneous threads: %d\n", thread_simul_cnt);
    printf("\n\n");

    destroy_network(nnet);
    free(feature_range);

}
