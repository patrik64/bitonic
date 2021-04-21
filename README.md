[![Build status](https://ci.appveyor.com/api/projects/status/s3wo2rl736iekakp?svg=true)](https://ci.appveyor.com/project/Patrik64/bitonic)
[![Build Status](https://travis-ci.org/Patrik64/Bitonic-cpp.svg?branch=master)](https://travis-ci.org/Patrik64/Bitonic-cpp)


bitonic application

Building and running the application  
The actor implementation uses several new features from the latest c++ implementation   
(e.g. auto keyword and lambda(s)), 
and it uses the boost library.  

to build the application, you'll need the following:  
- cmake  
- latest boost library   
- latest gcc compiler  

to build:  

```
mkdir build
cd build
cmake ..
make
```

that should create the bitonic executable
to run, type the following:

```./bitonic```
