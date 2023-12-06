# Dataset processed

This folder contains the processed files with simulated losses recovered using three packet loss concealment methods:
- Silence substitution: missing samples are replaced with silence
- Pattern replication: missing samples are replaced with the previous 128 samples.
- Burg's optimized den sqrt (hybrid denominator): missing samples are recovered using AR models based on Burg's method. For this method, two variants are available, where n is the amout of past history considered, and p is the model order:
  - n=2048, p=64
  - n=2048, p=128

To recreate these files, you need to first configure CMake
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBURG=OPT_DEN_SQRT -DDATA_TYPE=DOUBLE -DSAVE_FILE=on
```

The we build the application
```bash
cmake --build build/ --target burg -j 4
```

Lastly run the app
```bash
./build/burg
```

If you want to run another algorithm, or another data-type, change the CMake options according to the values you can find in the `CMakeLists.txt`

If instead  you want to experiment with a different value of n or p, change the values for the following two lines you can find in `src/main.cpp`

```c++
uint32_t selected_train_size = 2048;
uint32_t selected_lag_value = 128; 
```