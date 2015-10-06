'''
Created on Sep 30, 2015

@author: thomas
'''

import observable
import string
import nltk
import h5py

class Corpus(observable.Corpus):
    '''
    Class which inherits from Corpus to import a PAN11 dataset.
    '''

    def __init__(self, name, filename):
        self.filename = filename
        super(Corpus, self).__init__(name)
 
    def import_PAN11(self):
        '''
        Import documents from a PAN11 file.
        ''' 
        with open(self.filename,'r') as f:
            
            in_text = False
            in_body = False
            
            # Loop over lines.
            for line in f:
                
                # Check line for different possible elements (text,author,body)
                if not(in_text):
                    # Check for beginning of text element. 
                    if line.find("<text file=") != -1: 
                        # Find the indices of the information in the string.
                        start_ind = line.find( "file=") +6
                        stop_ind = line.find( ">") -1
                        # Extract textID. 
                        textID = line[start_ind:stop_ind];
                        
                        # Create new document with ID.
                        doc = observable.Document(textID)
                        self.documents.append(doc)
                        
                        # Set the in_text flag.
                        in_text = True
                else:
                    # Check if in body.
                    if in_body:
                        # Check for end of body element.
                        if line.find("</body>") != -1:
                            # Set flag and continue with next line.
                            in_body = False
                            continue
                        else:
                            # Append the line to document body and continue with next line.
                            self.documents[-1].body += line;
                            continue
                    
                    # Check for beginning of body element.
                    if line.find( "<body>") != -1:
                        # Set flag and continue with next line.
                        in_body = True
                        continue
                    
                    # Check for end of a text element.
                    if line.find( "</text>") != -1:
                        # Set the flag and continue with next line.
                        in_text = False
                        continue
                        
                    # Check for an author element.
                    if line.find( "<author id=") != -1:
                        # Find the indices of the information in the string.
                        start_ind = line.find("id=") +4
                        stop_ind = line.find("/>") -1
                        # Extract author ID.
                        author = line[start_ind:stop_ind]
                        # Store author ID in last created document.
                        self.documents[-1].authorID = author
                        continue
                    
        # Update document number.
        self.D = len(self.documents)
        
    def index_words(self,vocab_dict):
        '''
        Index word given the vocabulary of a training corpus.
        Ignore words that doesn't exist in the original vocabulary. 
        '''
        # Loop over all documents.
        for doc in self.documents:
            # Loop over all words in a document.
            for token in doc.tokens:
                if token in vocab_dict.keys():
                    doc.words.append(vocab_dict[token])
                    
            # Update number of words in document. Ignore unknown words.
            doc.N = len(doc.words)
        
        
    def process_test_texts(self,corp):
        '''
        Process the test texts.
        '''
        # Word processing.
        self.word_tokenize()
        # Copy vocabulary from 
        self.index_words(corp.vocab_dict)
          
          
    def save_test_to_hdf5(self):
        '''
        Save the complete corpus to an hdf5 file.
        '''
        filename = self.name + '.h5'
        # Create new file, let fail if exists.
        f = h5py.File(filename, 'r+')
        
        corpus_group = f.create_group('test_corpus')
        corpus_group.attrs['name'] = self.name
        corpus_group.attrs['info'] = 'Created from PAN11 LargeTest data set. Words tokenized using nltk.word_tokenize() and normalized using string.lower().'
        
        group_documents = corpus_group.create_group('documents')
        group_documents.attrs['info'] = 'Group of documents, each labeled by their integer index, containing a vector of words in form of integer labels.'
        
        authors_vec = [] # Vector containing author labels for each document.
        N = []      # Vector containing number of words for each document.


        for ind, doc in enumerate(self.documents):
            # Safe words vector to the database.
            dset_doc = group_documents.create_dataset("{}".format(ind), data=doc.words)
            dset_doc.attrs['text'] = doc.body
            dset_doc.attrs['textID'] = doc.ID
            dset_doc.attrs['number of words'] = doc.N
                      
            # Copy word number.
            N.append(doc.N)
            # Copy author index.
            authors_vec.append(doc.author_index)

        dset_N = corpus_group.create_dataset('N', data=N)                     # Vector of word counts.
        dset_N.attrs['info'] = 'Vector of word counts for every document.'
                
        dset_D = corpus_group.create_dataset('D', data=self.D)
        dset_D.attrs['info'] = 'Number of documents.'
        

#         dset_vocab = corpus_group.create_dataset('Vocabulary', data=self.vocab)
#         dset_vocab.attrs['info'] = 'Vocabulary vector. Words in form of index refer to this vocabulary vector.'
  
    