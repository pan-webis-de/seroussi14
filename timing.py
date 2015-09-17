'''
Created on Sep 15, 2015

@author: thomas
'''

import timeit

if __name__ == '__main__':
#     print(timeit.timeit("import_PAN11()",setup="from main import import_PAN11",number=2))
#     N = 1000
#     print(timeit.timeit("chain.sample(0,5)",setup="from main import PAN11_chain; chain = PAN11_chain()",number=N)/N)
#     print("Look for key in empty dict: {}".format(timeit.timeit("2 in d",setup="d = {}",number = N)/N))
#     print("Sum numpy array with 20996 entries: {}".format(timeit.timeit("sum(a)",setup="import numpy as np; a = np.arange(20996)",number = N)/N))
    print(timeit.timeit("chain.iterate()",setup="from main import PAN11_chain; chain = PAN11_chain()",number=1))