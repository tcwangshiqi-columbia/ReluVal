# ReluVal(Usenix Security'18)
ReluVal is a system for formally analyzing self-defined properties on given neural networks by leveraging symbolic interval analysis and iterative refinement.

You can find detailed description of ReluVal in paper [Formal Security Analysis of Neural Networks using Symbolic Intervals](https://arxiv.org/pdf/1804.10829.pdf).

This repository contains the implementation of ReluVal and the evalutions on ACAS Xu described in the paper.


## Prerequisite


### OpenBLAS Installation
OpenBLAS library is used for matrix multiplication speedup. So please make sure you have successfully installed [OpenBLAS](https://www.openblas.net/). You can follow following commands to install openblas or find the quick installation guide at [OpenBLAS's Installation Guide](https://github.com/xianyi/OpenBLAS/wiki/Installation-Guide).

```
wget http://github.com/xianyi/OpenBLAS/archive/v0.2.20.tar.gz
tar -xzf SOpenBLAS-0.2.20.tar.gz
cd OpenBLAS-0.2.20
make
make PREFIX=/path/to/your/installation install
```

### Downloading

```
git clone https://github.com/tcwangshiqi-columbia/ReluVal
```

### Compiling:
Please make sure the path of OpenBLAS is the same as the one in MakeFile. Then you can compile ReluVal with following command:

```
cd ACAS
make
```

## Running 

The main function is in network_test.c. To run the function, you can call the binary ./network_test. It expects at least three arguments. Here is the argument list:

property: the saftety property want to verify

network: the network want to test with

target: Wanted label of the property

need to print = 0: whether need to print the detailed information of each split. 0 is not and 1 is yes. Default value is 0.

test for one run = 0: whether need to estimate the output range without split refinement. 0 is no, 1 is yes. Default value is 0.

check mode = 0: Normal split mode is 0. Check adv mode is 1. Check adv mode will prevent further splits as long as the depth goes upper than 20 so as to locate concrete adversarial examples faster. Default value is 0.

The program will terminate in two ways: (1) a concrete adversarial is found, and (2) the property is verified as safe.

### Example

Here is an example for running ReluVal:

```
./network_test 5 ./nnet/ACASXU_run2a_1_1_batch_2000.nnet 4
```

### ACAS Xu experiments:

The evaluations on ACAS Xu can be easily ran with pre-written scripts in folder "scripts". Here is an example:

```
./scripts/run_property5.sh
```


## Citing ReluVal

```
@inproceedings {Shiqi18,
	author = {Shiqi Wang and Kexin Pei and Justin Whitehouse and Junfeng Yang and Suman Jana},
	title = {Formal Security Analysis of Neural Networks using Symbolic Intervals},
	booktitle = {27th {USENIX} Security Symposium ({USENIX} Security 18)},
	year = {2018},
	address = {Baltimore, MD},
	url = {https://www.usenix.org/conference/usenixsecurity18/presentation/wang-shiqi},
	publisher = {{USENIX} Association},
}
```


## Contributors

* [Shiqi Wang](https://sites.google.com/view/tcwangshiqi) - tcwangshiqi@cs.columbia.edu
* [Kexin Pei](https://sites.google.com/site/kexinpeisite/) - kpei@cs.columbia.edu
* Justin Whitehouse 
* [Junfeng Yang](http://www.cs.columbia.edu/~junfeng/) - junfeng@cs.columbia.edu
* [Suman Jana](http://www.cs.columbia.edu/~suman/) - suman@cs.columbia.edu

