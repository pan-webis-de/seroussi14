'''
Created on Sep 14, 2015

@author: thomas
'''
import string


class Corpus(object):
    '''
    Class to store a text corpus.
    '''
    
    def __init__(self,name):
        '''
        Constructor
        '''
        self.documents = []
        self.name = name
        
        # Initialize corpus variables.
        self.V = 0      # Number of words in vocabulary.
        self.D = 0      # Number of documents.
        self.A = 0      # Number of authors.
        self.authors = {}
        
    def separate_words(self):
        '''
        For every document built a list of words_strings out of the documents' text.
        '''
        for doc in self.documents:
            doc.separate_words()
            
            
    def built_vocabluary(self):
        '''
        Built a list of words_strings in the corpus.
        The separate_words functions must be called prior to this function.
        '''
        # Create an empty set.
        vocab_set = set()
        
        # Loop over documents.
        for doc in self.documents:
            # Loop over words_strings and add each word to the set.
            for word in doc.words_strings:
                vocab_set.add(word)
        # Update vocabulary size variable.
        self.V = len(vocab_set)
        # Store the vocabulary in a dict.
        self.vocab = dict(zip(vocab_set,range(self.V)))
        
        
    def index_words(self):
        '''
        Apply the integer index for every word according to the vocabulary dict.
        '''
        
        # Loop over all documents.
        for doc in self.documents:
            # Loop over all words in a document.
            for word in doc.words_strings:
                doc.words.append(self.vocab[word])
      
      
    def find_all_authors(self):
        '''
        Construct a dict containing author IDs as keys and integer indices.
        '''
        # Initialize a set.
        authorID_set = set()
        # Loop over all documents and add the author to the set.
        for doc in self.documents:
            authorID_set.add(doc.authorID)
        # Update the number of authors.
        self.A = len(authorID_set)
        # Construct the dict containing IDs as keys and integer indices.
        self.authors = dict(zip(authorID_set,range(self.A)))
      
    def index_authors(self):
        '''
        Store the index of every documents author in the document object.
        '''
        # Loop over all documents.
        for doc in self.documents:
            doc.author = self.authors[doc.authorID]
      
    def process_raw_data(self):
        '''
        Apply all the steps necessary to obtain data in the form needed for the algorithm.
        '''
        # Word processing.
        self.separate_words()
        self.built_vocabluary()
        self.index_words()
        # Author processing.
        self.find_all_authors()
        self.index_authors()
        
                
        
class Document:
    '''
    Class to store and process document data
    '''
    
    # Set with punctuation characters.
    punct = set(string.punctuation)
    
    def __init__(self, ID):
        self.body = ""
        self.ID = ID
        self.authorID = ""
        self.author = -1    # Variable for the author index according to the set of corpus authors.
        # Initialize list for words in form of a string.
        self.words_strings = []
        # Initialize list for words in form of an index.
        self.words = []
        # Number of words in the document.
        self.N = 0
        
    def separate_words(self):
        ''' 
        Construct a list with words using the text in self.body.
        Words are not necessarily in the original sequence.
        Punctuation at the beginning and the end gets removed.
        '''
        
        words_splitted = string.split(self.body)
        
        # Loop over words.
        for word in words_splitted:
            # Ignore names specified by <NAME/>, for PAN11.
            word = word.replace("<NAME/>","")
            
            # Make all characters lower case.
            word = string.lower(word)
            
            # Set flags to false.
            is_left_punctuation_removed = False
            is_right_punctuation_removed = False
            
            # Loop until flags is True.
            while not(is_left_punctuation_removed and is_right_punctuation_removed):
                # Check beginning for punctuation character.
                if len(word) > 0 and word[0] in self.punct:
                    self.words_strings.append(word[0])
                    if len(word) > 0:
                        word = word[1:]
                else:
                    is_left_punctuation_removed = True
                # Check end for punctuation character.
                if len(word) > 0 and word[-1] in self.punct:
                    self.words_strings.append(word[-1])
                    if len(word) > 0:
                        word = word[:-1]
                else:
                    is_right_punctuation_removed = True
                    
            if len(word) > 0:
                self.words_strings.append(word)
        
        # Update number of words.
        self.N = len(self.words_strings)
                        