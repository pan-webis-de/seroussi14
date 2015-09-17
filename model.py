'''
Created on Sep 15, 2015

@author: thomas
'''

import observable
import numpy as np
from numpy import dtype
from numpy import random
import h5py

class MI_Chain(object):
    '''
    Class for a Markov MI_Chain used for model inference.
    '''

    def __init__(self, Corp, T_A, T_D):
        '''
        Constructor:
        Copy observable variables from the corpus.
        '''
        # Seed the random number generator.
        random.seed(0)
        # Copy the name.
        self.name = Corp.name
        
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
            self.indicators.append([None]*doc.N)

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
        
        # Construct a lookup table for indices and indicator, topic combinations in form of tuples (y,t). Used for sampling.
        self.lookup_comb = {}
        self.num_comb = self.T_A+self.T_D
        # Loop over both types of topics and count.
        cnt = 0
        for t in range(self.T_D):
            self.lookup_comb[cnt] = (0,t)     # y=0 for document topics.
            cnt += 1    # Update counter.
        for t in range(self.T_A):
            self.lookup_comb[cnt] = (1,t)     # y=1 for author topic.
            cnt += 1    # Update counter.
              
        
    def sample(self,d,i):
        '''
        Sample for the di-th word from the conditional distribution for feature extraction (Seroussi 2014, Authorship Attribution with Topic Models, Equation 11). 
        '''
        # Get the word.
        w = self.words[d][i]
        # Get the author of the document.
        a = self.authors[d]
        # Get the topic indicator.
        y = self.indicators[d][i]
        # Get topic.
        t = self.topics[d][i]
        # Initialize the probability vector.
        p = np.zeros(self.num_comb)
        
        # Loop over all combinations indicators and topics and calculate probabilities.
        cnt = 0
        
        for top in range(self.T_D):
            # Probability for y=0 case.
            # First calculate the factors needed in eqn 11 in Seroussi 2014.
            # b is the term in parentheses.
            b = self.delta_D + self.c_d_DD[d]
            # n1 is the first numerator.
            if top in self.c_dt_DT[d]:        # Consider case that key d is not in dict.
                n1 = self.alpha_D + self.c_dt_DT[d][top]
            else:
                n1 = self.alpha_D
            # n2 is the second numerator.
            if w in self.c_tw_DTV[top]:
                n2 = self.beta_D[w] + self.c_tw_DTV[top][w]
            else:
                n2 = self.beta_D[w]
            # d1 is the first denominator.
            d1 = self.sum_alpha_D + sum(self.c_dt_DT[d].values())
            if y == 0:      # No problems with uninitialized entries because None == integer returns false.
                d1 -= 1
            # d2 is the second denominator.
            d2 = self.sum_beta_D + sum(self.c_tw_DTV[top].values())
            if y == 0 and t == top:
                d2 -= 1
            # Calculate probability.
            p[cnt] = b*n1*n2/d1/d2
            cnt += 1    # Update counter.
        for top in range(self.T_A):
            # Probability for y=1 case.
            # First calculate the factors needed in eqn 11 in Seroussi 2014.
            # b is the term in parentheses.
            b = self.alpha_A + self.c_d_DA[d]
            # n1 is the first numerator.
            if top in self.c_at_AT[a]:        # Consider case that key d is not in dict.
                n1 = self.alpha_A + self.c_at_AT[a][top]
            else:
                n1 = self.alpha_A
            # n2 is the second numerator.
            if w in self.c_tw_ATV[top]:
                n2 = self.beta_A[w] + self.c_tw_ATV[top][w]
            else:
                n2 = self.beta_A[w]
            # d1 is the first denominator.
            d1 = self.sum_alpha_A + sum(self.c_at_AT[a].values())
            if y == 1:      # No problems with uninitialized entries because None == integer returns false.
                d1 -= 1
            # d2 is the second denominator.
            d2 = self.sum_beta_A + sum(self.c_tw_ATV[top].values())
            if y == 1 and t == top:
                d2 -= 1
            # Calculate probability.
            p[cnt] = b*n1*n2/d1/d2
            cnt += 1    # Update counter.
                
        # Check that all entries of p are positive.
        if (p < 0).any():
            print("Non-positive probability encourntered.")
            print(p)
            print(self.c_d_DA)
        
        # Check that all counts are positive.
#         self.check_counts_non_negative()
                
        # Normalize p to 1.
        p = p/sum(p)
        
        # Draw a sample from the distribution.
        ind = random.choice(self.num_comb,p=p)
        
        # Look up the combination of author, indicator and topic.
        comb = self.lookup_comb[ind]
        
        # Update the latent variables.
        self.indicators[d][i] = comb[0]
        self.topics[d][i] = comb[1]
        
        # Update counts.
        self.update_counts(y, t, comb[0], comb[1], a, d , w)
        
             
    def update_counts(self, old_y, old_t, new_y, new_t, a, d ,w):
        '''
        Update the count variables.
        '''
        # Lower the counts according to the old variables.
        if old_y != None:   # Consider the case that its the initial iteration. Then the old variables are None.
            if old_y == 0:
                self.c_d_DD[d] -= 1
                self.c_dt_DT[d][old_t] -= 1
                self.c_tw_DTV[old_t][w] -= 1 
            else:
                self.c_d_DA[a] -= 1
                self.c_at_AT[a][old_t] -= 1
                self.c_tw_ATV[old_t][w] -= 1
        
        # Raise the counts according to the new variables.
        if new_y == 0:
            self.c_d_DD[d] += 1
            if new_t in self.c_dt_DT[d]:
                self.c_dt_DT[d][new_t] += 1
            else:
                self.c_dt_DT[d][new_t] = 1
            if w in self.c_tw_DTV[new_t]:
                self.c_tw_DTV[new_t][w] += 1
            else:
                self.c_tw_DTV[new_t][w] = 1 
        else:
            self.c_d_DA[a] += 1
            if new_t in self.c_at_AT[a]:
                self.c_at_AT[a][new_t] += 1
            else:
                self.c_at_AT[a][new_t] = 1
            if w in self.c_tw_ATV[new_t]:
                self.c_tw_ATV[new_t][w] += 1
            else:
                self.c_tw_ATV[new_t][w] = 1
         
             
    def check_counts_non_negative(self):
        '''
        Check that all counts are non-negative.
        '''
        # Loop over all counts.
        
        for t in self.c_tw_DTV:        # Count of word w in document topic t.
            if (np.array(t.values()) < 0).any():
                print("Negative count in c_tw_DTV for topic {}".format(t))
        
        for t in self.c_tw_ATV:        # Count of word w in author topic t.
            if (np.array(t.values()) < 0).any():
                print("Negative count in c_tw_ATV for topic {}".format(t))
        
        for a in self.c_at_AT:        # Count of author topic t assignments to author a.
            if (np.array(a.values()) < 0).any():
                print("Negative count in c_at_AT for author {}".format(a))
                
        for d in self.c_dt_DT:        # Count of words assigned to document topic t in document d.
            if (np.array(d.values()) < 0).any():
                print("Negative count in c_dt_DT for document {}".format(d))
       
        if (self.c_d_DD < 0).any():   # Count of words assigned to document topics in document d.
            print("Negative count in c_d_DD")
        if (self.c_d_DA < 0).any():   # Count of words assigned to author topics in document d.
            print("Negative count in c_d_DA")
        
             
    def iterate(self):
        '''
        Perform one iteration of the Markov chain
        '''
        # Loop over all words in all documents.
        for d in range(self.D):
#             print d
            for i in range(self.N[d]):
                self.sample(d,i)
        