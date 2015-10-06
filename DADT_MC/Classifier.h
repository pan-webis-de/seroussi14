/*
 * Clasifier.h
 *
 *  Created on: Oct 1, 2015
 *      Author: thomas
 */

#ifndef CLASSIFIER_H_
#define CLASSIFIER_H_

#import "MIChain.h"
#import "AChain.h"
#include "hdf5.h"
#include "hdf5_hl.h"

class TestDocument;

class Classifier {
	// This class organized the attribution of test texts to author using the AChain class and an inferred model with the MIChain class.
public:
	MIChain* model_chain;		// Model inference chain.

	std::string database_path;

	int num_burn_in;	// Number of burn in iterations.
	int num_average;	// Number of averaging iterations.

	int D;				// Number of documents.
	TestDocument** documents;		// Array of TestDocuments

	Classifier(std::string database_path);
	virtual ~Classifier();

	void attribute_author(MIChain* mc, TestDocument* doc);	// Attribute an author to a text.

	void attribution_testset(int chain_id, int iteration);		// Attribute the author of every test text using the inferred model in chain_id and using the given iteration number.

};

class TestDocument {
public:
	// This class is used to manage test documents.
	int* words;						// Words of the document.
	int N;							// Number of words.
	int docID;				// ID of text.
	std::map<int,int> authors;		// Vector of authors attributed to the text.
	int probable_author;			// Attributed author of the text.
	double score;					// Trust score of the attributed author.
	int author_cnt;					// Number of authors added to the document.

	TestDocument(int N, int* words, int id);
	virtual ~TestDocument();

	void add_author(int a);		// Add a possible author.
	int get_author();			// Return the most probable author of the text by choosing the most frequent author in the author vector.
	double get_score();			// Calculate the score of the most probable author.

	void print_words();			// Print the words to std out.
	void print_authors();
};

#endif /* CLASSIFIER_H_ */
