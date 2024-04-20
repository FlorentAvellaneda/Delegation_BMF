# Delegation-Relegation for Boolean Matrix Factorization (Appendix)

The provided source code is designed for compilation and execution on GNU/Linux systems.

## File organization
- `script/`: Contains scripts for re-running benchmarks
- `build/`: Contains the executable of our tool
- `code/`: Contains the source code of our tool
- `data/Real/`: Original, unsimplified datasets
- `data/Real/SimplExi/`: Datasets with existential simplification
- `data/Real/SimplUni/`: Datasets with universal simplification
- `data/Real/dat/`: Datasets in *.dat format
- `data/Synthetic/`: Synthetic datasets
- `otherTools/`: Contains Iteress, UndercoverBMF, and BMF_via_CG

## Dependencies

### Not included
- g++
- cmake
- python3 (optional: for external solution validation)

### Included
- Cadical
- EvalMaxSAT
- CLI11

## Other tools included
- Iteress : https://fcalgs.sourceforge.net/
- UndercoverBMF : https://github.com/FlorentAvellaneda/UndercoverBMF
- BMF_via_CG : https://github.com/kovacsrekaagnes/rank_k_Binary_Matrix_FactorisationIncluded

## Build
The code is already compiled and the binaries are located in the 'build' folder. If you wish to recompile, follow these instructions:
```bash
cd build
cmake ../code/
make
```


## Usage

**Usage:** `./build/Simpli [OPTIONS] CSV_file`  

**Options:**  
  -h,--help           &emsp; *Print this help message and exit*    
  --exi                      &emsp; *Simplification Existentially*  
  --uni                      &emsp; *Simplification Universally*  
  --seed UINT           &emsp; *Seed (0 means random seed)*  
  --OptiBlock           &emsp; *Activate the OptiBlock strategy*  
  -k UINT               &emsp; *k (default = +infinity)*  
  -S TEXT                    &emsp; *Save the simplified matrix*  
  -v UINT                    &emsp; *verbosity (default: 1)*  
  -o TEXT                    &emsp; *output file for A and B*  
  -O TEXT                    &emsp; *output file for A o B*  


## Benchmark from the paper (folder `script`)  
- `./simplifyAndFastBMF.sh` Prints the size, the number of 1s, and the remaining ones after simplification with SimpliExists and SimpliForAll (**Table 1**)
- `./benchIteressSimplification.sh` Prints the remaining 1s after simplification with Iteress (**Table 1**)
- `./BMF_with_CG.sh 10800` Performs BMF with CG on original datasets with a timeout of 10800 sec (**Table 1**)
- `./existentialBMF_with_CG.sh 10800` Performs BMF with CG on existentially simplified datasets with a timeout of 10800 sec  (**Table 1**)
- `./universalBMF_with_CG.sh 10800` Performs BMF with CG on universally simplified datasets with a timeout of 10800 sec  (**Table 1**)
- `./IteressBMF_with_CG.sh 10800` Performs BMF with CG on matrices simplified with Iteress with a timeout of 10800 sec  (**Table 1**)
- `./universalBMF_with_Optiblock.sh 10800` Performs BMF with OptiBlock on universally simplified datasets with a timeout of 10800 sec (**Table 2**)
- `./existentialBMF_with_Optiblock.sh 10800` Performs BMF with OptiBlock on existentially simplified datasets with a timeout of 10800 sec (**Table 2**)
- `./BMF_with_Optiblock.sh 10800` Performs BMF with OptiBlock on original datasets with a timeout of 10800 sec (**Table 2**)
- `./IteressBMF_with_Optiblock.sh 10800` Performs BMF with OptiBlock on matrices simplfied with Iteress with a timeout of 10800 sec (**Table 2**)
- `./benchOptiOnSyntetic.sh`: Finds Optimal BMF on synthetic matrices with SAT (**Table 3**)
- `./benchOptiOnSynteticSimplified.sh`: Finds Optimal BMF with SAT on synthetic matrices simplified with universal simplification (**Table 3**)
- `./simplifyAndFastBMFOnSyntetic.sh` Prints the remaining 1s after simplification with SimpliExists and SimpliForAll on synthetic datasets  (**Table 3**)
- `./benchIteressSimplificationOnSynthetic.sh` Prints the remaining 1s after simplification with Iteress on synthetic matrices (**Table 3**)
- `./generateSimplExi.sh` Generates existentially simplified datasets
- `./generateSimplUni.sh` Generates universally simplified datasets



