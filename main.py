'''
Created on Sep 14, 2015

@author: thomas
'''

import PAN11Corpus
import model
import timeit

def main():        
    Corp = import_PAN11()
    chain = model.Learning_Chain(Corp,90,10)
    

def import_PAN11():
        Corp = PAN11Corpus.Corpus('PAN11Large','../resources/PAN11/training/LargeTrain.xml')
        Corp.import_PAN11()
        Corp.process_raw_data()
        return Corp
        
def PAN11_chain():
    Corp = import_PAN11()
    chain = model.Learning_Chain(Corp,90,10)
    return chain
    
if __name__ == "__main__":
    main();