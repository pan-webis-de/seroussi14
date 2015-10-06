'''
Created on Sep 30, 2015

@author: thomas
'''

import PAN11Corpus
import PAN11TestCorpus

def main():
    corp = PAN11Corpus.Corpus("PAN11FridayRun","/Users/thomas/Documents/workspace/DADT/LargeTrain.xml")
    corp.import_PAN11()
    corp.process_raw_data()
    corp.save_to_hdf5()
    tcorp = PAN11TestCorpus.Corpus("PAN11FridayRun","/Users/thomas/GoogleDrive/aatm/resources/PAN11/test/LargeTest.xml")
    tcorp.import_PAN11()
    tcorp.process_test_texts(corp)
    tcorp.save_test_to_hdf5()
    
    
if __name__ == '__main__':
    main()