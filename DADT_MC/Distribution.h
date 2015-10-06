/*
 * Distribution.h
 *
 *  Created on: Sep 28, 2015
 *      Author: thomas
 */

#ifndef DISTRIBUTION_H_
#define DISTRIBUTION_H_

#include <list>
#include <iostream>
#include "hdf5.h"
#include "hdf5_hl.h"

class Distribution {
private:
	int length;
	double* probabilities;
	double zcp; 		// Zero count probability. Probability of words with zero count.
	double zcp_sw;	// Zero count probability for stopwords.
public:
	Distribution(int length);
	virtual ~Distribution();
	double at(int i);				// Get the probability at index i.
	void set(int i, double p);		// Set the probability at index i to p.
	double sum();					// Return the sum of all probabilities.
	int get_length();				// Return the array length.
	void set_zcp(double p);			// Set zero count probability.
	double get_zcp();				// Get zero count probability.
	void set_zcp_sw(double p);		// Set zero count probability for stopwords.
	double get_zcp_sw();			// Get zero count probability for stopwords.
	void print_topN(int N);			// Print the N most frequent words with their frequency.
	void save_hdf5(hid_t loc_id, std::string name);		// Save the distribution to an HDF5 database.
};

#endif /* DISTRIBUTION_H_ */
