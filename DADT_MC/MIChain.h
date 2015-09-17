/*
 * MIChain.h
 *
 *  Created on: Sep 16, 2015
 *      Author: thomas
 */

#ifndef MICHAIN_H_
#define MICHAIN_H_


#include <stdlib.h>
#include "hdf5.h"
#include "hdf5_hl.h"
#include <string>
#include <iostream>

class MIChain {
public:

	int A;			// Number of authors.
	int D;			// Number of documents.
	int V;			// Number of words in vocabulary.
	int* N;			// Vector of number of words in each document.
	int T_A;		// Number of author topics.
	int T_D	;		// Number of document topics.

	int* authors;		// Vector of author indices.
	int** words;		// Vector of word vectors.
	int** topics; 		// Vector of topic vectors.
	int** indicators;	// Vector of indicator vectors.

	double alpha_A;		// Author topic prior.
	double alpha_D;		// Document topic prior.

	double delta_A;		// Prior for author topic ratio.
	double delta_D;		// Prior for document topic ratio.

	double epsilon;		// Stopword parameter.
	double* beta_A;		// Word in author topic prior.
	double* beta_D;		// Word in document topic prior.

	int eta;		// Author in corpus prior.

	MIChain(std::string h5filename);
	virtual ~MIChain();
};

#endif /* MICHAIN_H_ */
