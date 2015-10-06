/*
 * Clasifier.cpp
 *
 *  Created on: Oct 1, 2015
 *      Author: thomas
 */

#include "Classifier.h"

Classifier::Classifier(std::string database_path) {
	// Generator.
	this->database_path = database_path;

	// Iteration numbers.
	this->num_burn_in = 100;
	this->num_average = 1;

	// Open the HDF5 database.
	hid_t file_id;
	file_id = H5Fopen(this->database_path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

	// Open group for test texts.
	std::string test_text_group_name = std::string("test_corpus");
	hid_t test_text_group_id = H5Gopen(file_id, test_text_group_name.c_str(), H5P_DEFAULT);

	// Load number of documents.
	int D_buffer[1];
	H5LTread_dataset_int(test_text_group_id, std::string("D").c_str(), D_buffer);
	this->D = D_buffer[0];

	// Load word counts of documents.
	int* N_buffer = new int[this->D];
	H5LTread_dataset_int(test_text_group_id, std::string("N").c_str(), N_buffer);

	// Initialize test documents array.
	this->documents = new TestDocument*[this->D];

	// Open the document group.
	std::string document_group_name = std::string("documents");
	hid_t document_group_id = H5Gopen(test_text_group_id, document_group_name.c_str(), H5P_DEFAULT);

	// Create documents with information from database.
	for (int i=0; i<this->D; i++) {
		int* words_buffer = new int[N_buffer[i]];
		H5LTread_dataset_int(document_group_id, std::to_string(i).c_str(), words_buffer);

		// Create the new document.
		this->documents[i] = new TestDocument(N_buffer[i], words_buffer, i);
		delete words_buffer;
	}

	// Close groups and files.
	H5Gclose(document_group_id);
	H5Gclose(test_text_group_id);
	H5Fclose(file_id);

	delete N_buffer;

	// Initialize MIChain.
	this->model_chain = new MIChain(database_path, 0);
}

Classifier::~Classifier() {
	// TODO Auto-generated destructor stub
	delete[] this->documents;
}

void Classifier::attribute_author(MIChain* mc, TestDocument* doc) {
	// Attribute an author to a text.
	AChain* ac = new AChain(doc->N, doc->words, mc);
	// Obtain an author attribution for the text.
	int aa = ac->get_most_probable_author(this->num_burn_in, this->num_average);
	// Add the author to the document author vector.
	doc->add_author(aa);
	std::cout << "Document " << doc->docID << " attributed to author " << aa << std::endl;
//	doc->print_authors();
//	doc->print_words();

	delete ac;
}


void Classifier::attribution_testset(int chain_id, int iteration) {
	// Attribute the author of every test text using the inferred model in chain_id and using the given iteration number.

	// Load the requested sample into the MIChain.
	this->model_chain->load_sample(chain_id, iteration);
	// Calculate distributions.
	this->model_chain->calc_distributions();
//	std::cout << "Author distribution:" << std::endl;
//	for (int i=0; i<this->model_chain->A; i++) {
//		std::cout << "Author " << i << " : " << this->model_chain->chi_a->at(i) << std::endl;
//	}

	// Iterate over all documents...
	for (int i=0; i<this->D; i++) {
		this->attribute_author(this->model_chain, this->documents[i]);
	}
}





TestDocument::TestDocument(int N, int* words, int id) {
	// Constructor.
	this->N = N;
	this->words = new int[this->N];
	for (int i=0; i<this->N; i++) this->words[i] = words[i];

	this->probable_author = -1;
	this->author_cnt = 0;
	this->score = 0;
	this->docID = id;

}
TestDocument::~TestDocument() {
	delete this->words;
}

void TestDocument::add_author(int a) {
	// Add a possible author. Append to vector of authors and update most probable author and score.
	if (this->authors.count(a) == 1) { // author already in map
		this->authors[a] += 1;
	} else { // Create an entry.
		this->authors.insert(std::pair<int,int>(a,1));
	}
	// Update author count.
	this->author_cnt += 1;

	// Update most probable author. For equal number of attributions just choose first author.
	int arg = -1;
	int cnt = 0;
	for (std::map<int,int>::iterator it=this->authors.begin(); it!=this->authors.end(); ++it) {
		if (it->second > cnt) {
			arg = it->first;
			cnt = it->second;
		}
	}
	this->probable_author = arg;

	// Update score.
	this->score = cnt / this->author_cnt;
}

int TestDocument::get_author() {
	// Return the most probable author of the text by choosing the most frequent author in the author vector.
	return this->probable_author;
}

double TestDocument::get_score() {
	// Calculate the score of the most probable author.
	return this->score;
}

void TestDocument::print_words() {
	// Print the words to std out.
	std::cout << "Words in document " << this->docID << std::endl;
	for (int i=0; i<this->N; i++) std::cout << this->words[i] << ", ";
	std::cout << std::endl;
}

void TestDocument::print_authors() {
	// Print the words to std out.
	std::cout << "Document " << this->docID << " has authors:"<< std::endl;
	for (std::map<int,int>::iterator it=this->authors.begin(); it!=this->authors.end(); ++it) {
		std::cout << "Author " << it->first << " with count " << it->second << std::endl;
	}
}
