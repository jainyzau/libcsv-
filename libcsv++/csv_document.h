#ifndef _LIBCSVPLUSPLUS_CSV_DOCUMENT_H_
#define  _LIBCSVPLUSPLUS_CSV_DOCUMENT_H_
#include <vector>
#include <list>
#include <string>
namespace CSV
{
	class CSVDocument
	{
	public:
		typedef std::string element_type;
		typedef std::vector<element_type> row_type;
		typedef std::list<row_type> document_type;
		typedef document_type::size_type row_index_type;
		typedef row_type::size_type column_index_type;
		typedef document_type::iterator iterator;
		typedef document_type::const_iterator const_iterator;

		enum ParseState{
			LineStart,
			FieldStart,
			FrontQuote,
			BackQuote,
			EscapeOn,
			EscapeOff,
			FieldEnd,
			LineEnd
		};

		enum OutputMode{
			CompleteEnclosure,
			OptionalEnclosure
		};

		struct ParseContex {
			ParseContex(){
				idx = 0;
				field_beg = 0;
				field_end = 0;
				state = LineEnd;
				row_count = 0;
				col_count = 0;
			}
			std::string read_str;
			row_index_type row_count;
			column_index_type col_count;
			std::string row_str;
			std::string::size_type idx;
			std::string::size_type field_beg;
			std::string::size_type field_end;
			std::string elem;
			row_type row;
			ParseState state;
		};

		long long parse(const std::string& file_path);
		row_index_type to_file(const std::string& file_path, OutputMode output_mode = OptionalEnclosure);
		const document_type& get_document() const;
		const row_type& get_row(row_index_type row) const;
		const element_type& get_element(row_index_type row, column_index_type col) const;
		row_index_type size() const;
		iterator begin();
		iterator end();
		row_type& operator[](row_index_type idx);

		void merge_document(const document_type& doc);
		void add_row(const row_type& row);
		void remove_row(row_index_type row_idx);
		void replace_row(row_index_type row_idx, const row_type& row);
		void update_elem(row_index_type row, column_index_type col, const element_type& new_val);
		void clear();

	private:
		int _replace_all( std::string &field, const std::string& old_str, const std::string& new_str );
		void _write_optional_enclosure_field( std::ostream& out_stream, const element_type& elem, bool last_elem );
		void _write_complete_enclosure_field( std::ostream& out_stream, const element_type& elem, bool last_elem );
		void _check_row_index( row_index_type row_idx ) const;
		void _check_col_index( column_index_type col ) const;

		void _hanlde_line_start(ParseContex& contex);
		void _handle_field_start(ParseContex& contex);
		void _handle_field_end(ParseContex& contex);
		void _handle_escape_off(ParseContex& contex);
		void _handle_back_quote(ParseContex& contex);
		void _handle_front_quote(ParseContex& contex);

		document_type m_doc;
	};

}
#endif