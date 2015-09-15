'''
Created on Sep 15, 2015

@author: thomas
'''

import timeit

if __name__ == '__main__':
#     print(timeit.timeit("import_PAN11()",setup="from main import import_PAN11",number=2))
    print(timeit.timeit("chain.iterate()",setup="from main import PAN11_chain; chain = PAN11_chain()",number=2))