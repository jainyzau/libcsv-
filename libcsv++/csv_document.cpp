#include "csv_document.h"
#include <fstream>
#include <iostream>
#include <sstream>
namespace CSV
{

	long long CSVDocument::parse( const std::string& file_path )
	{
		std::ifstream csv_file(file_path.c_str());
		if (csv_file.fail())
		{
			throw std::runtime_error("Failed to open file " + file_path + ".");
		}
		
		ParseContex contex;
		while (std::getline(csv_file, contex.read_str))
		{
			contex.read_str.append("\n"); //std::getline() reads until CRLF, and doesn't add it to reading string, so we add it here.
			if (contex.state == LineEnd)
			{
				contex.state = LineStart;
				_hanlde_line_start(contex);
			}
			else
			{
				contex.row_str.erase(0, contex.field_beg);
				contex.row_str += contex.read_str;
				contex.idx -= contex.field_beg;
				contex.field_beg = contex.field_end = 0;
			}
			
			element_type elem;
			while(contex.idx < contex.row_str.size())
			{
				if (contex.row_str[contex.idx] == '"')
				{
					if (contex.field_beg == contex.idx && contex.state == FieldStart)
					{
						contex.state = FrontQuote;
						_handle_front_quote(contex);
					}
					else if (contex.state == FrontQuote || contex.state == EscapeOff)
					{
						contex.state = EscapeOn;
					}
					else if (contex.state == EscapeOn)
					{
						contex.state = EscapeOff;
						_handle_escape_off(contex);
					}
					else
					{
						throw std::runtime_error("Syntax error: field has quote symbol in middle while no quote symbol in head.");
					}
				}
				else if (contex.row_str[contex.idx] == ',')
				{
					contex.field_end = contex.idx;
					if (contex.field_beg < contex.idx && contex.row_str[contex.idx-1] == '"' && (contex.state == EscapeOn || contex.state == EscapeOff))
					{
						contex.state = BackQuote;
						_handle_back_quote(contex);
					}

					if (contex.state == BackQuote || contex.state == FieldStart)
					{
						contex.state = FieldEnd;
						_handle_field_end(contex);
					}
				}
				else if (contex.row_str[contex.idx] == '\n')
				{
					if (contex.state == FieldStart)
					{
						// the last field is end, so we have FieldEnd state and then LineEnd state.
						contex.field_end = contex.idx;

						contex.state = FieldEnd;
						_handle_field_end(contex);

						contex.state = LineEnd;
					}
					else if (contex.state == FrontQuote || contex.state == EscapeOff)
					{
						// This field has a CRLF char, reads another line and continue.
					}
				}

				++contex.idx;
			}
		}

		return contex.row_count;
	}

	CSVDocument::row_index_type CSVDocument::to_file( const std::string& file_path, OutputMode output_mode /* = OptionalEnclosure*/ )
	{
		std::ofstream out_file(file_path.c_str());
		if (out_file.fail())
		{
			throw std::runtime_error("Failed to open file " + file_path + " for writing.");
		}
		for (document_type::iterator itr = m_doc.begin(); itr != m_doc.end(); ++itr)
		{
			for (row_type::iterator row_itr = itr->begin(); row_itr != itr->end(); ++row_itr)
			{
				element_type& elem = *row_itr;
				if (output_mode == OptionalEnclosure)
				{
					_write_optional_enclosure_field(out_file, elem, row_itr + 1 == itr->end());
				}
				else
				{
					_write_complete_enclosure_field(out_file, elem, row_itr + 1 == itr->end());
				}
			}
		}

		return m_doc.size();
	}

	const CSVDocument::document_type& CSVDocument::get_document() const
	{
		return m_doc;
	}

	const CSVDocument::row_type& CSVDocument::get_row( row_index_type row ) const
	{
		_check_row_index(row);

		for (document_type::const_iterator itr = m_doc.begin(); itr != m_doc.end(); ++itr)
		{
			if (row-- == 0)
			{
				return *itr;
			}
		}

		// never reach here, but in order to make compiler happy, we add a return statement here.
		return m_doc.front();
	}

	const CSVDocument::element_type& CSVDocument::get_element( row_index_type row, column_index_type col ) const
	{
		_check_row_index(row);
		_check_col_index(col);

		for (document_type::const_iterator itr = m_doc.begin(); itr != m_doc.end(); ++itr)
		{
			if (row-- == 0)
			{
				const row_type& fields = *itr;
				return fields[col];
			}
		}

		// never reach here, but in order to make compiler happy, we add a return statement here.
		return m_doc.front().front();
	}

	void CSVDocument::merge_document( const document_type& doc )
	{
		for (document_type::const_iterator itr = doc.begin(); itr != doc.end(); ++itr)
		{
			m_doc.push_back(*itr);
		}
	}

	void CSVDocument::add_row( const row_type& row )
	{
		m_doc.push_back(row);
	}

	void CSVDocument::remove_row( row_index_type row_idx )
	{
		_check_row_index(row_idx);

		document_type::iterator itr = m_doc.begin();
		std::advance(itr, row_idx);
		m_doc.erase(itr);
	}

	void CSVDocument::replace_row( row_index_type row_idx, const row_type& row )
	{
		_check_row_index(row_idx);

		document_type::iterator itr = m_doc.begin();
		std::advance(itr, row_idx);
		m_doc.erase(itr);

		itr = m_doc.begin();
		std::advance(itr, row_idx - 1);
		m_doc.insert(itr, row);
	}

	void CSVDocument::update_elem( row_index_type row, column_index_type col, const element_type& new_val )
	{
		_check_row_index(row);
		_check_col_index(col);

		document_type::iterator itr = m_doc.begin();
		std::advance(itr, row);
		row_type& update_row = *itr;
		update_row[col] = new_val;
	}

	void CSVDocument::clear()
	{
		m_doc.clear();
	}

	CSVDocument::row_index_type CSVDocument::size() const
	{
		return m_doc.size();
	}

	CSVDocument::iterator CSVDocument::begin()
	{
		return m_doc.begin();
	}

	CSVDocument::iterator CSVDocument::end()
	{
		return m_doc.end();
	}

	CSVDocument::row_type& CSVDocument::operator[]( row_index_type idx )
	{
		document_type::iterator itr = m_doc.begin();
		std::advance(itr, idx);
		return *itr;
	}

	int CSVDocument::_replace_all( std::string &field, const std::string& old_str, const std::string& new_str )
	{
		std::string::size_type quote_pos = 0;
		int replace_count = 0;
		while ((quote_pos = field.find(old_str, quote_pos)) != std::string::npos)
		{
			field.replace(quote_pos, old_str.size(), new_str);
			quote_pos += new_str.size();
			++replace_count;
		}

		return replace_count;
	}

	void CSVDocument::_write_optional_enclosure_field( std::ostream& out_stream, const element_type& elem, bool last_elem )
	{
		if (elem.find("\"") != std::string::npos)
		{
			std::string new_elem = elem;
			_replace_all(new_elem, "\"", "\"\"");
			out_stream << "\"" << new_elem << "\"";
		}
		else if (elem.find(",") != std::string::npos || elem.find("\n") != std::string::npos)
		{
			out_stream << "\"" << elem << "\"";
		}
		else
		{
			out_stream << elem;
		}

		out_stream << (last_elem ? "\n" : ",");
	}

	void CSVDocument::_write_complete_enclosure_field( std::ostream& out_stream, const element_type& elem, bool last_elem )
	{
		if (elem.find("\"") != std::string::npos)
		{
			std::string new_elem = elem;
			_replace_all(new_elem, "\"", "\"\"");
			out_stream << "\"" << new_elem << "\"";
		}
		else
		{
			out_stream << "\"" << elem << "\"";
		}
		out_stream << (last_elem ? "\n" : ",");
	}


	void CSVDocument::_check_row_index( row_index_type row_idx ) const
	{
		if (row_idx >= m_doc.size())
		{
			std::ostringstream str_stream;
			str_stream << "row index " << row_idx << " is out of range. (max: " << m_doc.size() - 1 << ")";
			throw std::out_of_range(str_stream.str());
		}
	}

	void CSVDocument::_check_col_index( column_index_type col ) const
	{
		if (m_doc.front().size() <= col)
		{
			std::ostringstream str_stream;
			str_stream << "column index " << col << " is out of range. (max: " << m_doc.front().size() - 1 << ")";
			throw std::out_of_range(str_stream.str());
		}
	}

	void CSVDocument::_handle_field_end( ParseContex& contex )
	{
		contex.elem.append(contex.row_str.c_str() + contex.field_beg, contex.field_end - contex.field_beg);
		contex.field_beg = contex.idx + 1;
		contex.row.push_back(contex.elem);

		contex.state = FieldStart;
		_handle_field_start(contex);
	}

	void CSVDocument::_handle_escape_off( ParseContex& contex )
	{
		if (contex.row_str[contex.idx - 1] != '"') // in this state, (idx - 1) is definitely valid.
		{
			throw std::runtime_error("Syntax error: quote is not escaped properly.");
		}
		contex.elem.append(contex.row_str.c_str() + contex.field_beg, contex.idx - contex.field_beg);
		contex.field_beg = contex.idx + 1;
	}

	void CSVDocument::_handle_back_quote( ParseContex& contex )
	{
		contex.field_end = contex.idx - 1;
	}

	void CSVDocument::_handle_front_quote( ParseContex& contex )
	{
		++contex.field_beg;
	}

	void CSVDocument::_handle_field_start( ParseContex& contex )
	{
		contex.elem.clear();
	}

	void CSVDocument::_hanlde_line_start( ParseContex& contex )
	{
		if (contex.row.size() > 0)
		{
			if (contex.col_count == 0)
			{
				contex.col_count = contex.row.size();
			}
			else if (contex.row.size() > contex.col_count)
			{
				std::ostringstream str_stream;
				str_stream << "Syntax error: too much fields in line " << contex.row_count << ".";
				throw std::runtime_error(str_stream.str());
			}
			m_doc.push_back(contex.row);
			contex.row.clear();
		}
		++contex.row_count;
		contex.row_str = contex.read_str;
		contex.idx = 0;
		contex.field_beg = contex.field_end = 0;

		contex.state = FieldStart;
		_handle_field_start(contex);
	}

}