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
#include <map>
#include <vector>
#include <iostream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

class Dynamic_Counter;
class Static_Counter;

class MIChain {
public:

	int A;			// Number of authors.
	int D;			// Number of documents.
	int V;			// Number of words in vocabulary.
	int N_sw;		// Number of stopwords in vocabulary.
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

	// Variables for sums needed for probability calculation.
	double sum_alpha_A;
	double sum_alpha_D;
	double sum_beta_A;
	double sum_beta_D;

	// Count variables.
	Static_Counter* c_d_DA;      			// Count of words assigned to author topics in document d.
	Static_Counter* c_d_DD;					// Count of words assigned to document topics in document d.
	Static_Counter** c_dt_DT;	// Count of words assigned to document topic t in document d.
	Static_Counter** c_at_AT;	// Count of author topic t assignments to author a.
	Dynamic_Counter** c_tw_ATV;	// Count of word w in author topic t.
	Dynamic_Counter** c_tw_DTV;	// Count of word w in document topic t.

	std::map<int,std::pair<int,int> >* lookup;		// Lookuptable for combinations.
	int N_comb;										// Number of possible combinations of identifier and topics.

	// Random number generator.
	boost::mt19937 rng;
	// Vector of probabilities.
	std::vector<double> p;

	// Flag for first iteration.
	bool is_first_iteration;


	MIChain(std::string h5filename);
	virtual ~MIChain();

	void init_rng();

	void iterate(int count); 						// Iterate the chain count times.

	void sample(int d, int i);						// Sample from the probability distribution for i-th word in d-th document.
	void calc_prob(int w, int a, int y_old, int t_old, int d, int i);
	int draw();			// Draw from the discrete distribution given by weights p.
	void update_counts(int w, int d, int a, int y_old, int y_new, int t_old, int t_new);		// Update counts.

	int sum_map_values(std::map<int,int>* m);		// Sum the values of a map.
};

class Dynamic_Counter {
public:
	// Map to store entries and counts.
	std::map<int,int> entries;
	// Variable to cache sum.
	int count_sum;


	Dynamic_Counter() {
		// Constructor
		this->count_sum = 0;
	}

	void inc(int ind) {
		// Increment count of index ind by 1 if exist.
		if (this->entries.count(ind) == 1)
			this->entries[ind] += 1;
		// Create new entry if it doesn't exist.
		else
			this->entries.insert(std::pair<int,int>(ind, 1));
		// Update sum.
		this->count_sum += 1;
	}

	void dec(int ind) {
		// Decrement count of index ind by 1 if it exists and its greater than 1.
		if (this->entries.count(ind) == 1) {
			// Delete entry if its decremented to 0.
			if (this->entries[ind] == 1) {
				this->entries.erase(ind);
			} else {
			// Else decrement.
				this->entries[ind] -=1;
			}
			// In both cases decrement sum.
			this->count_sum -= 1;
		} else {
			// Attempt to decrement non-existent index. Print error.
			std::cerr << "Attempt to decrement non-existent count. Exiting!" << std::endl;
		}
	}

	int at(int ind) {
		// Return the count to index ind. Return 0 if entry to ind doesn't exist.
		int cnt = 0;
		if (this->entries.count(ind) == 1) cnt = this->entries[ind];
		return cnt;
	}

	int sum() {
		// Return sum.
		return this->count_sum;
	}
};

class Static_Counter {
private:
	// Map to store entries and counts.
	int* entries;
	int length;
	// Variable to cache sum.
	int count_sum;

public:
	Static_Counter() {
		// Constructor
		this->length = 0;
		this->entries = new int[0];
		this->count_sum = 0;
	}
	Static_Counter(int length) {
		// Constructor
		this->length = length;
		this->entries = new int[this->length];
		for (int i=0; i<this->length; i++) this->entries[i] = 0;
		this->count_sum = 0;
	}

	virtual ~Static_Counter() {
		delete[] this->entries;
	}

	void reconstruct(int length) {
		// Set new length;
		this->length = length;
		this->count_sum = 0;
		// Delte old array;
		delete[] this->entries;
		// Create new array;
		this->entries = new int[this->length];
		for (int i=0; i<this->length; i++) this->entries[i] = 0;
	}

	void inc(int ind) {
		// Increment count of index ind by 1 if exist.
		this->entries[ind] += 1;
		// Update sum.
		this->count_sum += 1;
	}

	void dec(int ind) {
		// Decrement count of index ind by 1 if it exists and its greater than 1.
		if (this->entries[ind] > 0) {
			// Decremented entry.
			this->entries[ind] -=1;
			// Decrement sum.
			this->count_sum -= 1;
		} else {
			// Attempt to decrement non-existent index. Print error.
			std::cerr << "Attempt to decrement non-existent count. Exiting!" << std::endl;
		}
	}

	int at(int ind) {
		// Return the count to index ind. Return 0 if entry to ind doesn't exist.
		return this->entries[ind];
	}

	int sum() {
		// Return sum.
		return this->count_sum;
	}
};

#endif /* MICHAIN_H_ */
