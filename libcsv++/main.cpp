#include "csv_document.h"
#include <iostream>

int main()
{
	CSV::CSVDocument lDoc;
	lDoc.parse("csv/test.csv");

	std::cout << lDoc.get_element(0, 5) << std::endl;
	CSV::CSVDocument::row_type row = lDoc.get_row(0);
	for (CSV::CSVDocument::row_type::iterator itr = row.begin(); itr != row.end(); ++itr)
	{
		std::cout << *itr << ",";
	}
	std::cout << std::endl;

	lDoc.add_row(row);
	lDoc.to_file("csv/test_out.csv", CSV::CSVDocument::CompleteEnclosure);
	lDoc.remove_row(lDoc.get_document().size() - 1); 
	lDoc.to_file("csv/test_out_2.csv", CSV::CSVDocument::CompleteEnclosure);

	std::cout << lDoc[0][1] << std::endl;
	lDoc[0][1] = "test";
	std::cout << lDoc[0][1] << std::endl;
	for (CSV::CSVDocument::iterator itr = lDoc.begin(); itr != lDoc.end(); ++itr)
	{
		for (CSV::CSVDocument::row_type::iterator row_itr = itr->begin(); row_itr != itr->end(); ++row_itr)
		{
			std::cout << *row_itr;
			if (row_itr + 1 != itr->end())
			{
				std::cout << ",";
			}
		}
		std::cout << std::endl;
	}
	
}