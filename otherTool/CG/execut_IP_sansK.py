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
                       
parser.add_argument('--k', nargs='?', help='Rank', type=int, default=10000)

parser.add_argument('--t', nargs='?', help='timeout', type=float, default=3600)
                 
args = parser.parse_args()
#k = args.k  
dataset_path = args.Path

X = pd.read_csv(dataset_path, header=None).to_numpy()
X_out, row_weights, col_weights, idx_rows_reconstruct,\
idx_cols_reconstruct, idx_zero_row, idx_zero_col, idx_rows_unique, idx_cols_unique = preprocess(X)

kInit = min(args.k*2, X.shape[0]-1, X.shape[1]-1)


# # boucler de min(X.shape[0], X.shape[1]) à 1
# #for k in range(min(X.shape[0], X.shape[1])-1, 0, -1):
# for k in range(kInit, 0, -1):

        

#     bmf = BMF_via_compact_IP(X, k)
#     bmf.preprocess_input_matrices()
#     bmf.CIP_solve(max_time=3600)
#     bmf.post_process_output_matrices()

#     t=time.time()-time1
#     print("Result IP on ", os.path.basename(dataset_path) ," with k = ",k,": ", np.nansum(np.abs(X - boolean_matrix_product(bmf.A, bmf.B))), " errors in ", t, " sec")
#     if np.nansum(np.abs(X - boolean_matrix_product(bmf.A, bmf.B))) != 0:
#         print("Best k = ", k+1)
#         break


debut = 1
fin = kInit
resultat = kInit+1

while debut <= fin:
    milieu = (debut + fin) // 2

    print("k = ", milieu, " ...")

    bmf = BMF_via_compact_IP(X, milieu)
    bmf.preprocess_input_matrices()
    if bmf.CIP_solve(max_time=args.t, maximumObjectiveValue=0) == False:
        debut = milieu + 1

        print("Result IP on ", os.path.basename(dataset_path) ," with k = ",milieu," : nb_error > 0")
    else:

        bmf.post_process_output_matrices()
        print("Result IP on ", os.path.basename(dataset_path) ," with k = ",milieu,": ", np.nansum(np.abs(X - boolean_matrix_product(bmf.A, bmf.B))), " errors")

        resultat = milieu
        fin = milieu - 1


t=time.time()-time1
print("Best k = ", resultat, " in ", t, " sec")
