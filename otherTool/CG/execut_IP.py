from BMF import *
import time
import pandas as pd
import numpy as np
import argparse
import sys
import os
import ntpath


time1=time.time()

parser = argparse.ArgumentParser(description='File to ...')

                       
parser.add_argument('Path',
                       metavar='path',
                       type=str,
                       help='path to the dataset')
                       
parser.add_argument('--k', nargs='?', help='Rank', type=int, default=0)

parser.add_argument('--t', nargs='?', help='timeout', type=float, default=3600)
                 
args = parser.parse_args()
#k = args.k  
dataset_path = args.Path

X = pd.read_csv(dataset_path, header=None).to_numpy()
X_out, row_weights, col_weights, idx_rows_reconstruct,\
idx_cols_reconstruct, idx_zero_row, idx_zero_col, idx_rows_unique, idx_cols_unique = preprocess(X)




for k in range(args.k, min(X.shape[0], X.shape[1]), 1):
    bmf = BMF_via_compact_IP(X, k)
    bmf.preprocess_input_matrices()
    
    if bmf.CIP_solve(max_time=args.t, maximumObjectiveValue=0) == True:
        bmf.post_process_output_matrices()
        print("Result IP on ", os.path.basename(dataset_path) ," with k = ",k,": ", np.nansum(np.abs(X - boolean_matrix_product(bmf.A, bmf.B))), " errors")
        break
        

t=time.time()-time1
print("Temps = ", t, " sec")

