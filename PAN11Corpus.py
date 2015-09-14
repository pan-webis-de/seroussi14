'''
Created on Sep 14, 2015

@author: thomas
'''

import structure
import string

class Corpus(structure.Corpus):
    '''
    Class which inherits from Corpus to import a PAN11 dataset.
    '''

    def __init__(self, name, filename):
        self.filename = filename
        super(Corpus, self).__init__(name)
 
    def importPAN11(self):
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
                    if string.find(line, "<text file=") != -1: 
                        # Find the indices of the information in the string.
                        start_ind = string.find(line, "file=") +6
                        stop_ind = string.find(line, ">") -1
                        # Extract textID. 
                        textID = line[start_ind:stop_ind];
                        
                        # Create new document with ID.
                        doc = structure.Document(textID)
                        self.documents.append(doc)
                        
                        # Set the in_text flag.
                        in_text = True
                else:
                    # Check if in body.
                    if in_body:
                        # Check for end of body element.
                        if string.find(line,"</body>") != -1:
                            # Set flag and continue with next line.
                            in_body = False
                            continue
                        else:
                            # Append the line to document body and continue with next line.
                            self.documents[-1].body += line;
                            continue
                    
                    # Check for beginning of body element.
                    if string.find(line, "<body>") != -1:
                        # Set flag and continue with next line.
                        in_body = True
                        continue
                    
                    # Check for end of a text element.
                    if string.find(line, "</text>") != -1:
                        # Set the flag and continue with next line.
                        in_text = False
                        continue
                        
                    # Check for an author element.
                    if string.find(line, "<author id=") != -1:
                        # Find the indices of the information in the string.
                        start_ind = string.find(line,"id=") +4
                        stop_ind = string.find(line,"/>") -1
                        # Extract author ID.
                        author = line[start_ind:stop_ind]
                        # Store author ID in last created document.
                        self.documents[-1].authorID = author
                        continue
                    
        # Update document number.
        self.D = len(self.documents)
    