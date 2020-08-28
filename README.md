# DSPatch

Implementation of the [DSPatch](https://people.inf.ethz.ch/omutlu/pub/DSPatch_prefetcher_micro19.pdf) Prefetcher developed by Intel Labs and ETH Zürich, on a light weight Cache Simulator found [here](https://github.com/Shihao-Song/Computer-Architecture-Teaching/tree/master/C621/Cache_Policy).

## Build
To build this either use the command:

```bash
make
```
or 
```
g++ -std=c++11 -g -o Main Main.cpp Trace.cpp Cache.cpp DSPatch.cpp
```

## Usage

To use this simulator trace files must be space separated ie.
```
CORE_ID PROGRAM_COUNTER MEMORY_ADDRESS LOAD_OR_STORE
```

Then run using the following command:
```
./Main <trace_file>
```
ie.
```
./Main sample.mem_trace
```