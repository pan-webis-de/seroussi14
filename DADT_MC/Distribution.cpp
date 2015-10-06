/*
 * Distribution.cpp
 *
 *  Created on: Sep 28, 2015
 *      Author: thomas
 */

#include "Distribution.h"

Distribution::Distribution(int length) {
	// Constructor
	this->length = length;
	this->probabilities = new double[this->length];
	for (int i=0; i<this->length; i++) this->probabilities[i] = 0;
}

Distribution::~Distribution() {
	// Destructor
	delete[] this->probabilities;
}

double Distribution::at(int i) {
	// Return probability at index i.
	return this->probabilities[i];
}

void Distribution::set(int i, double p) {
	// Set probability at index i to p.
	this->probabilities[i] = p;
}

double Distribution::sum() {
	// Return the sum of all probabilities.
	double sum = 0;
	for (int i=0; i<this->length; i++) sum += this->probabilities[i];
	return sum;
}


int Distribution::get_length() {
	// Return the length of the probability distribution array.
	return this->length;
}

void Distribution::set_zcp(double p) {
	// Set zero count probability.
	this->zcp = p;
}

double Distribution::get_zcp() {
	// Get zero count probability.
	return this->zcp;
}

void Distribution::set_zcp_sw(double p) {
	// Set zero count probability for stopwords.
	this->zcp_sw = p;
}

double Distribution::get_zcp_sw() {
	// Get zero count probability for stopwords.
	return this->zcp_sw;
}

void Distribution::print_topN(int N) {
	// Print the N most frequent words with their frequency.
	std::list<std::pair<int,double> > list;

	// Initialize list.
	double p = this->at(0);
	list.push_back(std::pair<int,double>(0,p));

	for (int i=1; i<this->length; i++) { // Iterate over all words.
		p = this->at(i);

		// Check if probability is larger than min
		for ( std::list< std::pair<int,double> >::iterator it = list.begin(); it != list.end(); ++it ) {
			if ( p > it->second ) { // insert before the element
				list.insert(it, std::pair<int,double>(i,p));
				break;
			}
		}
	}

	std::list< std::pair<int,double> >::iterator it = list.begin();
	for (int i=0; i<N; i++) {
		std::cout << it->first << " : " << it->second << std::endl;
		it++;
	}
}


void Distribution::save_hdf5(hid_t loc_id, std::string name) {
	// Save the counter to the HDF5 location loc_id in a dataset named name.
	// Set data dimensionality.
	int rank = 1;
	// Create array with dimensions of data.
	hsize_t dims[1] = {this->length};
	//	H5LTmake_dataset(file_id, name.c_str(), rank, dims, H5T_NATIVE_INT, buffer);
	H5LTmake_dataset(loc_id, name.c_str(), rank, dims, H5T_NATIVE_DOUBLE, this->probabilities);
	int sum_buffer[1] = {this->sum()};
	H5LTset_attribute_int(loc_id, name.c_str(), "sum", sum_buffer, 1);
}
