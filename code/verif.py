import numpy as np
import argparse
import pandas as pd


parser = argparse.ArgumentParser(description='count the number of reconstruction error A o B on X')



parser.add_argument('A', metavar='A', type=str, help='path to the matrix A')

parser.add_argument('B', metavar='B', type=str, help='path to the matrix B')

parser.add_argument('X', metavar='X', type=str, help='path to the input matrix X')


args = parser.parse_args()


A = pd.read_csv(args.A, header=None).to_numpy()
B = pd.read_csv(args.B, header=None).to_numpy()
X = pd.read_csv(args.X, header=None).to_numpy()



error = np.nansum(np.abs(X - (np.dot(A, B) > 0) ))

print("k =", A.shape[1])
print("Reconstruction error =", error)


