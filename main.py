'''
Created on Sep 14, 2015

@author: thomas
'''

import PAN11Corpus
import model
import timeit
import pickle

def main():        
    Corp = import_PAN11()
    Corp.save_to_hdf5()
    
#     pickle.dump( chain, open( "chain_initialized.p", "wb" ) )

#     chain = pickle.load( open("chain_iterations_1.p", "rb"))
#     chain.iterate()
#     pickle.dump(chain, open("chain_iterations_2.p", "wb"))

def import_PAN11():
        Corp = PAN11Corpus.Corpus('PAN11Large','LargeTrain.xml')
#         Corp = PAN11Corpus.Corpus('PAN11Large','pan11single.xml')
        Corp.import_PAN11()
        Corp.process_raw_data()
        return Corp
        
def PAN11_chain():
    Corp = import_PAN11()
    chain = model.MI_Chain(Corp,90,10)
    return chain
    
if __name__ == "__main__":
     main()
