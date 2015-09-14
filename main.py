'''
Created on Sep 14, 2015

@author: thomas
'''

import PAN11Corpus


def main():        
    Corp = PAN11Corpus.Corpus('PAN11Large','../resources/PAN11/training/LargeTrain.xml')
#     Corp = PAN11Corpus.Corpus('PAN11Large','pan11single.xml')
#     Corp = PAN11Corpus.Corpus('PAN11Large','pan11some.xml')
    Corp.importPAN11()
   
    Corp.process_raw_data()
    
#     for doc in Corp.documents:
#         print("AuthorID : {}".format(doc.authorID))
#         print("Author ind : {}".format(doc.author))
        
    print("Vocabulary size : {}".format(Corp.V))
    print("Number of documents : {}".format(Corp.D)) 
    print("Number of authors : {}".format(Corp.A))
    

if __name__ == "__main__":
    main();