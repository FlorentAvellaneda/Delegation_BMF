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
                       
parser.add_argument('--k', nargs='?', help='Rank', type=int, default=3600)

parser.add_argument('--t', nargs='?', help='timeout', type=float, default=3600)


args = parser.parse_args()
#k = args.k  
dataset_path = args.Path

X = pd.read_csv(dataset_path, header=None).to_numpy()
X_out, row_weights, col_weights, idx_rows_reconstruct,\
idx_cols_reconstruct, idx_zero_row, idx_zero_col, idx_rows_unique, idx_cols_unique = preprocess(X)

# Sum of all elements in X and convert to int
sum_X = int(np.nansum(X))

# Number of one in X
nb_ones = sum_X


kInit = min(args.k, X.shape[0]-1, X.shape[1]-1, nb_ones-1)

# boucler de min(X.shape[0], X.shape[1]) à 1
for k in range(kInit, 0, -1):
    bmf = BMF_via_CG(X, k)
    bmf.preprocess_input_matrices()

    t=time.time()-time1
    max_time=max(args.t-t,2)
    #print("solve for ", max_time, "sec...")
    bmf.MLP_solve(max_time=max_time, display=False)
    bmf.MIP_solve(max_time=max_time, display=False)

    bmf.post_process_output_matrices()

    t=time.time()-time1
    print("Result CG on ", os.path.basename(dataset_path) ," with k = ",k,": ", np.nansum(np.abs(X - boolean_matrix_product(bmf.A, bmf.B))), " errors in ", t/60.0, " min")
    if np.nansum(np.abs(X - boolean_matrix_product(bmf.A, bmf.B))) != 0:
        error = np.nansum(np.abs(X - boolean_matrix_product(bmf.A, bmf.B)))
        print("Best for ",os.path.basename(dataset_path)," k = ", k+1, " en ", (time.time()-time1)/60.0, " min avec dual bound = ", bmf.best_dual_bound)
        break



