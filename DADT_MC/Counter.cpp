/*
 * Counter.cpp
 *
 *  Created on: Sep 17, 2015
 *      Author: thomas
 */

#include "Counter.h"

Dynamic_Counter::Dynamic_Counter() {
	// Constructor
	this->count_sum = 0;
}

Dynamic_Counter::~Dynamic_Counter() {}

void Dynamic_Counter::inc(int ind) {
	// Increment count of index ind by 1 if exist.
	if (this->entries.count(ind) == 1)
		this->entries[ind] += 1;
	// Create new entry if it doesn't exist.
	else
		this->entries.insert(std::pair<int,int>(ind, 1));
	// Update sum.
	this->count_sum += 1;
}

void Dynamic_Counter::dec(int ind) {
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

int Dynamic_Counter::at(int ind) {
	// Return the count to index ind. Return 0 if entry to ind doesn't exist.
	int cnt = 0;
	if (this->entries.count(ind) == 1) cnt = this->entries[ind];
	return cnt;
}

int Dynamic_Counter::sum() {
	// Return sum.
	return this->count_sum;
}

void Dynamic_Counter::save(hid_t loc_id, std::string name) {
	// Save the counter to the HDF5 location loc_id in a dataset named name.
	// Set data dimensionality.
	int rank = 2;
	// Create array with dimensions of data.
	int size = this->entries.size();
	hsize_t dims[2] = {size, 2};
	// Data buffer
	int buffer[2*size];
	int cnt = 0;
	for (std::map<int,int>::iterator it=this->entries.begin(); it!=this->entries.end(); ++it) {
		buffer[2*cnt] = it->first;
		buffer[2*cnt+1] = it->second;
		cnt++;
	}
	// H5LTmake_dataset(file_id, name.c_str(), rank, dims, H5T_NATIVE_INT, buffer);
	H5LTmake_dataset(loc_id, name.c_str(), rank, dims, H5T_NATIVE_INT, buffer);
	int sum_buffer[1] = {this->sum()};
	H5LTset_attribute_int(loc_id, name.c_str(), "sum", sum_buffer, 1);
}

void Dynamic_Counter::load(hid_t loc_id, std::string name) {
	// Load counter data from data in dataset named name in HDF5 location loc_id.
	// !No error handling. Function expect length of data to match.

	// Create array to store dimensions of data.
	hsize_t dims[2];
	H5LTget_dataset_info ( loc_id, name.c_str(), dims, NULL, NULL );

	int size = dims[0];
	// Data buffer
	int buffer[2*size];

	H5LTread_dataset_int ( loc_id, name.c_str(), buffer );

	// Empty map.
	this->entries.empty();

	// Insert data into map.
	for (int i=0; i<size; i++) {
		this->entries.insert(std::pair<int,int>(buffer[2*i],buffer[2*i+1]));
	}

	// Get sum.
	int sum_buffer[1];
	H5LTget_attribute_int( loc_id, name.c_str(), "sum",  sum_buffer );
	this->count_sum = sum_buffer[0];
}

void Dynamic_Counter::print() {
	// Print out the entries.
	std::cout << "Printing counter..." << std::endl;
	for (std::map<int,int>::iterator it=this->entries.begin(); it != this->entries.end(); ++it) {
		std::cout << it->first << " -> " << it->second << std::endl;
	}
	std::cout << "Counter sum = " << this->count_sum << std::endl;
}



Static_Counter::Static_Counter(int length) {
	// Constructor
	this->length = length;
	this->entries = new int[this->length];
	for (int i=0; i<this->length; i++) this->entries[i] = 0;
	this->count_sum = 0;
}

Static_Counter::~Static_Counter() {
	delete this->entries;
}

void Static_Counter::inc(int ind) {
	// Increment count of index ind by 1 if exist.
	this->entries[ind] += 1;
	// Update sum.
	this->count_sum += 1;
}

void Static_Counter::dec(int ind) {
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

int Static_Counter::at(int ind) {
	// Return the count to index ind. Return 0 if entry to ind doesn't exist.
	return this->entries[ind];
}


int Static_Counter::sum() {
	// Return sum.
	return this->count_sum;
}

void Static_Counter::save(hid_t loc_id, std::string name) {
	// Save the counter to the HDF5 location loc_id in a dataset named name.
	// Set data dimensionality.
	int rank = 1;
	// Create array with dimensions of data.
	hsize_t dims[1] = {this->length};
	//	H5LTmake_dataset(file_id, name.c_str(), rank, dims, H5T_NATIVE_INT, buffer);
	H5LTmake_dataset(loc_id, name.c_str(), rank, dims, H5T_NATIVE_INT, this->entries);
	int sum_buffer[1] = {this->sum()};
	H5LTset_attribute_int(loc_id, name.c_str(), "sum", sum_buffer, 1);
}

void Static_Counter::load(hid_t loc_id, std::string name) {
	// Load counter data from data in dataset named name in HDF5 location loc_id.
	// !No error handling. Function expect length of arrays to match.
	hsize_t dims[1];
	H5T_class_t class_id[1];
	size_t type_size[1];
	H5LTget_dataset_info ( loc_id, name.c_str(), dims, class_id, type_size);


//	// Update length.
//	this->length = dims[0];
//	// Create new entries array.
//	this->entries = new int[this->length];

	int* buffer = new int[dims[0]];

	H5LTread_dataset_int ( loc_id, name.c_str(), buffer);
	for (int i=0; i<this->length; i++) this->entries[i] = buffer[i];
	int sum_buffer[1];
	H5LTget_attribute_int( loc_id, name.c_str(), "sum",  sum_buffer );
	this->count_sum = sum_buffer[0];
}

void Static_Counter::print() {
	// Print out the entries.
	std::cout << "Printing counter..." << std::endl;
	for (int i=0; i<this->length; i++) {
		std::cout << i << " -> " << this->at(i) << std::endl;
	}
	std::cout << "Counter sum = " << this->count_sum << std::endl;
}
