#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include "csv_document.h"

class LibCSVPlusPlusTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		// change the following code to define different test case.
		const size_t row_count = 4;
		const size_t col_count = 5;
		const char* test_data[row_count][col_count] = {
			{"ID", "\"Nam\ne\"", "Age", "School", "Address"},
			{"1001", "Zhang San", "30", "\tSWUST", "\"Chengdu, Sichuan\""},
			{"1002", "Li Si", "30", " SWUST", "\"Nanchong\"\" Sichuan\"" },
			{"1003", "Wang Wu", "32", "", "" }
		};
		m_row_count = row_count;
		m_col_count = col_count;
		const char* test_file = "libcsv++_test.csv";
		std::ofstream out_file(test_file);
		if (out_file.fail())
		{
			throw std::runtime_error("Failed to open file for write.");
		}
		for (size_t i = 0; i < row_count; ++i)
		{
			for (size_t j = 0; j < col_count; ++j)
			{
				out_file << test_data[i][j];
				if (j != col_count - 1)
				{
					out_file << ",";
				}
			}
			if (i != row_count - 1)
			{
				out_file << std::endl;
			}
		}
		out_file.close();
		csv_doc.load_file(test_file);
	}

	size_t expected_row_count() const
	{
		return m_row_count;
	}

	size_t expected_col_count() const
	{
		return m_col_count;
	}

	CSV::CSVDocument csv_doc;
private:
	size_t m_row_count;
	size_t m_col_count;

};
TEST_F(LibCSVPlusPlusTest, test_1)
{
	// summary info
	EXPECT_TRUE(csv_doc.size() == expected_row_count());
	EXPECT_TRUE(csv_doc.row_count() == expected_row_count());
	EXPECT_TRUE(csv_doc.col_count() == expected_col_count());

	// normal value
	EXPECT_TRUE(csv_doc[0][0] == "ID");
	// leading space is not ignored
	EXPECT_TRUE(csv_doc[1][3] == "\tSWUST");
	EXPECT_TRUE(csv_doc[2][3] == " SWUST");
	// CRLF in value
	EXPECT_TRUE(csv_doc[0][1] == "Nam\ne");
	// comma in value
	EXPECT_TRUE(csv_doc[1][4] == "Chengdu, Sichuan");
	// quote in value
	EXPECT_TRUE(csv_doc[2][4] == "Nanchong\" Sichuan");
	// empty field
	EXPECT_TRUE(csv_doc[3][3] == "");

	// remove a line
	EXPECT_TRUE(csv_doc[1][0] == "1001");
	csv_doc.remove_row(1);
	EXPECT_TRUE(csv_doc.size() == expected_row_count() - 1);
	EXPECT_TRUE(csv_doc[1][0] == "1002");

	// clear
	csv_doc.clear();
	EXPECT_TRUE(csv_doc.size() == 0);
}