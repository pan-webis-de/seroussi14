/*
 * MIChain.cpp
 *
 *  Created on: Sep 16, 2015
 *      Author: thomas
 */

#include "MIChain.h"

MIChain::MIChain(std::string h5filename) {
	// Constructor

	// Open the h5 filename.
	hid_t file_id;

	file_id = H5Fopen(h5filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

	// Read number of authors.
	H5LTread_dataset_int(file_id,"/corpus/A",&(this->A));
	// Read number of documents.
	H5LTread_dataset_int(file_id,"/corpus/D",&(this->D));
	// Read number of words in vocabulary.
	H5LTread_dataset_int(file_id,"/corpus/V",&(this->V));
	// Read word count vector.
	this->N = new int[this->D];
	H5LTread_dataset_int(file_id,"/corpus/N",this->N);


	this->T_A = 90;					// Number of author topics.
	this->T_D = 10;					// Number of document topics.

	// Read author vector.
	this->authors = new int[this->D];
	H5LTread_dataset_int(file_id,"/corpus/authors",this->authors);

	// Initialize per document arrays.
	this->words = new int*[this->D];		// Vector of word vectors.
	this->topics = new int*[this->D]; 		// Vector of topic vectors.
	this->indicators = new int*[this->D];	// Vector of indicator vectors.

	// Initialize on document level.
	for (int i=0; i<this->D; i++) {
		// Copy words.
		this->words[i] = new int[this->N[i]];
		std::string dataset_string = std::string("/corpus/documents/") + std::to_string(i);
		H5LTread_dataset_int(file_id,dataset_string.c_str(),this->words[i]);
		// Initialize the other arrays to the correct size.
		this->topics[i] = new int[this->N[i]];
		this->indicators[i] = new int[this->N[i]];
	}

	// Calculate author topic prior. (Scalar because prior is assumed to be symmetric).
	this->alpha_A = std::min(0.1,5.0/this->T_A);
	// Calculate document topic prior. (Scalar because prior is assumed to be symmetric).
	this->alpha_D = std::min(0.1,5.0/this->T_D);

	this->delta_A = 4.889;			// Prior for author topic ratio.
	this->delta_D = 1.222;			// Prior for document topic ratio.

	this->eta = 1;					// Author in corpus prior.

	// Calculate word in topic priors using the stopword_indicator vector.
	this->epsilon = 0.009;			// Stopword parameter.
	int* stopword_indicator = new int[this->V];
	H5LTread_dataset_int(file_id,"/corpus/stopword_indicator",stopword_indicator);
	this->beta_A = new double[this->V];
	this->beta_D = new double[this->V];
	for (int i=0; i<this->V; i++) {
		this->beta_A[i] = 0.01 + this->epsilon*stopword_indicator[i];
		this->beta_D[i] = 0.01 - this->epsilon*stopword_indicator[i];
	}



}

MIChain::~MIChain() {
	// TODO Auto-generated destructor stub
	// Delete all arrays on document basis.
	for (int i=1; i<this->D; i++) {
		delete[] this->words[i];
		delete[] this->topics[i];
		delete[] this->indicators[i];
	}
	//	Delete all arrays on corpus level.
	delete[] this->words;
	delete[] this->topics;
	delete[] this->indicators;
	delete[] this->N;
}

