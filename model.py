'''
Created on Sep 15, 2015

@author: thomas
'''

import observable
import numpy as np
from numpy import dtype

class Learning_Chain(object):
    '''
    Class for a Markov Learning_Chain used for model inference.
    '''

    def __init__(self, Corp, T_A, T_D):
        '''
        Constructor:
        Copy observable varaibles from the corpus.
        '''
        
        self.A = Corp.A     # Number of authors.
        self.D = Corp.D     # Number of documents.
        self.V = Corp.V     # Number of words in vocabulary.
        self.N = []         # Vector of number of words in each document.
        self.T_A = T_A      # Number of author topics.
        self.T_D = T_D      # Number of document topics.
        
        self.authors = []       # Vector of author indices.
        self.words = []         # Vector of word vectors.
        self.topics = []        # Vector of topic vectors.
        self.indicators = []    # Vector of indicator vectors.
            
        self.alpha_A = min(0.1,5.0/self.T_A)      # Author topic prior. (Scalar because prior is assumed to be symmetric).
        self.alpha_D = min(0.1,5.0/self.T_D)      # Document topic prior. (Scalar because prior is assumed to be symmetric).
        
        self.delta_A = 4.889        # Prior for author topic ratio.
        self.delta_D = 1.222        # Prior for document topic ratio. 
        
        self.eta = np.ones(self.A)       # Author in corpus prior.
        
        # Copy document level data.
        for doc in Corp.documents:
            # Copy word number.
            self.N.append(doc.N)
            # Copy author index.
            self.authors.append(doc.author_index)
            # Copy word vector.
            self.words.append(doc.words)
            # Initialize topic vector.
            self.topics.append([None]*doc.N)
            # Initialize indicator vector.
            self.indicators .append([None]*doc.N)

        # Calculate topic priors.
        self.epsilon = 0.009        # Stopword parameter.
        self.beta_A = 0.01*np.ones(self.V) + self.epsilon*np.array(Corp.is_stopword)     # Word in author topic prior.
        self.beta_D = 0.01*np.ones(self.V) - self.epsilon*np.array(Corp.is_stopword)     # Word in document topic prior.
        
        # Calculate sums needed for probability calculations.
        self.sum_alpha_A = self.alpha_A*self.T_A        # Sum over all entries in the author topic prior. The equality holds because the prior is assumed to be symmetric. 
        self.sum_alpha_D = self.alpha_D*self.T_D        # Sum over all entries in the document topic prior. The equality holds because the prior is assumed to be symmetric.
        self.sum_beta_D = sum(self.beta_D)              # Sum over all entries of the word in document topic prior.
        self.sum_beta_A = sum(self.beta_A)              # Sum over all entries of the word in document topic prior.
        
        # Initialize the count variables.
        # Topic related counts.
        self.c_tw_DTV = [{} for i in range(self.T_D)]       # Count of word w in document topic t.
        self.c_tw_ATV = [{} for i in range(self.T_A)]       # Count of word w in author topic t.
        # Author related counts.
        self.c_at_AT = [{} for i in range(self.A)]          # Count of author topic t assignments to author a.
        # Document related counts.
        self.c_d_DD = np.zeros(self.D, dtype=np.int32)      # Count of words assigned to document topics in document d.
        self.c_d_DA = np.zeros(self.D, dtype=np.int32)      # Count of words assigned to author topics in document d.
        self.c_dt_DT = [{} for i in range(self.D)]          # Count of words assigned to document topic t in document d.