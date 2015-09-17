/*
 * MIChain_test.cpp
 *
 *  Created on: Sep 16, 2015
 *      Author: thomas
 */

#include "MIChain.h"

# include "gtest/gtest.h"
# include <math.h>

TEST(InitTest, CanInitialize) {
    MIChain* chain = new MIChain("../PAN11Large.h5");
    EXPECT_EQ(72, chain->A);		// Check number of authors.
    EXPECT_EQ(9337, chain->D);		// Check number of documents.
    EXPECT_EQ(27101, chain->V);		// Check number of words in vocabulary.
    // Check some entries in the word count vector.
    EXPECT_EQ(chain->N[200], 15);
    EXPECT_EQ(chain->N[4044], 490);
    EXPECT_EQ(chain->N[6490], 31);
    // Check some entries in the author vector.
    EXPECT_EQ(chain->authors[200], 44);
    EXPECT_EQ(chain->authors[3487], 28);
    EXPECT_EQ(chain->authors[7477], 61);
    // Check some words.
    EXPECT_EQ(chain->words[267][31], 13019);
    EXPECT_EQ(chain->words[5572][6], 8613);
    EXPECT_EQ(chain->words[814][40], 10987);
}
