///////////////////////////////////////////////////////////////////////////////////
// File : coff_format.h
// Contains: coff file format
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

//
// Text/documentation taken from :
// https://wiki.osdev.org/COFF
//
// COFF stands for Common Object File Format. It is a file format used for storing compiled code,
// such as that outputted by a compiler or a linker.
//
// Like most compiler file formats, COFF defines structures within the file for storing information
// about the sections of the program, such as .text and .data, and about the symbols that the program declares or defines.
//
// COFF can be used to store individual functions or symbols, fragments of programs, libraries or entire executables.
//
// The Microsoft PE Executable format (strictly PE/COFF) contains a version of COFF.
//
// Whilst COFF is still around today, it is regarded as being overly complicated and in many
// cases its use has been replaced by ELF or other executable file formats.
//


// ---------------
// File Format
// ---------------

// COFF files contain a number of different tables, as well as areas of data that are referenced by the tables.
//
// Structure                          Purpose                               Location                                    Length
// --------------------------------------------------------------------------------------------------------------------------------------------------
// File Header                        Stores the basic information          At the beginning of the file,               Fixed structure length.
//                                    about the file as well as             except in Microsoft PE/COFF
//                                    pointers to the other structures.     Image files (see above).
//
//
// Optional Header                    Stores additional information         Only present if indicated                   Specified in the File Header
//                                    about the execution of the file.      by the File Header.Immediately
//                                                                          follows the File Header.
//
//
// Section Header                     Stores information about the          Immediately following the Optional Header.  Calculated by the number of sections
//                                    different sections defined            If no Optional Header is provided,          (specified in the File Header) multiplied
//                                    in the file.                          immediately follows the File Header.        by the fixed structure length.
//
//
// Section Relocation Table           Allows the file to be relocated       At most one table exists for each Section.  Calculated by the number of relocation
//                                    to any area of memory by providing    Location is indicated by the corresponding  entries (specified in the Section header)
//                                    information on which addresses        Section Header.                             multiplied by the fixed structure length.
//                                    need to be changed.
//
// Section Line Number Table          Provides debugging information        At most one table exists for each Section.  Calculated by the number of line number entries
//                                    to map code addresses to source       Location is indicated by the corresponding  (specified in the Section header) multiplied by
//                                    file line numbers.                    Section Header.                             the fixed structure length.
//
//
// Symbol Table                       Stores information on each            Indicated to a pointer in the File Header.  Calculated by the number of symbols
//                                    symbol defined or declared by                                                     (specified in the File Header) multiplied by
//                                    the code.                                                                         the fixed structure length.
//
// String Table                       Stores any Section or Symbol names    Immediately following the Symbol table.     Indicated by the first 32 bits of the string table,
//                                    that are longer than eight characters.                                            evaluated as a long
//
//

// Most COFF files contain additional space between the end of the Section table and the start of the Symbol table which stores the data for the sections themselves,
// although the location of this data relative to the Symbol (and String) table does not seem to be specified by the standard.


// ---------------
// File Header
// ---------------

// The file header exists at the beginning of a COFF file (except in Microsoft PE/COFF Image files as noted above).
// It contains a magic number field and other general information about the File.
// The structure of this header can be found in filehdr.h, and is 20 bytes longs.

// The magic number varies from implementation to implementation, for example, DJGPP generates COFF
// files with the value 0x14C in this field.
//
// The number of sections indicates how many section structures should be expected in the Section table.
//
// The time and date stamp indicate when the COFF file was created. This field is of the type time_t,
// being the number of seconds since epoch ( 1970-01-01 00:00:00 GMT ).
//
// The Symbol table pointer indicates the offset within the file where the Symbol table begins.
//
// The number of symbols indicate how many (fixed length) symbol structures are in the symbol table.
// This value coupled with the Symbol table pointer allow you to calculate where the String table starts.
//
// The flags field usually indicates the state of the file.
// Usually this is used to quickly lookup information that could be calculated from the rest of the file.
// For example, one bit of this field indicates if relocation information has been removed from the file,
// another bit indicates if all the external symbols have been resolved, etc.


#pragma pack(1)

typedef struct _coff_file_header
{
	uint16_t    f_magic;    // Magic number
	uint16_t    f_nscns;    // Number of Sections
	int32_t     f_timdat;   // Time & date stamp
	int32_t     f_symptr;   // File pointer to Symbol Table
	int32_t     f_nsyms;    // Number of Symbols
	uint16_t    f_opthdr;   // sizeof(Optional Header)
	uint16_t    f_flags;    // Flags
}coff_file_header;

// ---------------
// Optional Header
// ---------------

// The Optional header provides run-time information about the file and therefore is only present in executable COFF files.
// This header is only present in the file if the File header field f_opthdr is non-zero and, if present, immediately follows the File header.
// The structure of this header can be found in aouthdr.h, and is 28 bytes long.


typedef struct _coff_file_optional_header
{
	uint16_t    magic;      /* Magic Number                    */
	uint16_t    vstamp;     /* Version stamp                   */
	uint32_t    tsize;      /* Text size in bytes              */
	uint32_t    dsize;      /* Initialised data size           */
	uint32_t    bsize;      /* Uninitialised data size         */
	uint32_t    entry;      /* Entry point                     */
	uint32_t    text_start; /* Base of Text used for this file */
	uint32_t    data_start; /* Base of Data used for this file */
}coff_file_optional_header;

// ---------------
// Section Header
// ---------------

// The Section table provides information about the individual section contained within the file.
//
// The Section table is an array of Section header structures. It immediately follows the Optional header (or File header if no Optional header exists)
// and its length in Section header structures is given by the f_nscns field in the File header.
//
// Unlike most other tables, the Section table is 1-based, meaning that the first Section is referred to as Section 1 instead of Section 0.
// This is done because in the Symbol table, a Section number of 0 has a special meaning.
//
// The structure of this header can be found in scnhdr.h, and is 40 bytes long.

// The Section name field stores the name of this section if it is eight characters or fewer,
// otherwise this field contains a pointer into the String table. See the entry on the String table below.
//
// The Physical address field stores the memory start address for this file.
// If the file has been linked so that this section is to be loaded to a specific location in memory, this field will have been set to that value.
//
// The Virtual address field is equivalent to the Physical address field for the Virtual memory space.
// However, in almost all instances, these two fields contain the same value.
//
// The s_size field contains the Section size in bytes.
//
// The Section pointer field contains the file offset where the raw data for this section begins.
// This, coupled with the Section size, can be used to load the section data.
// If this is a BSS Section (indicated in the Section's Flags field), the section may well have a size, but a Section pointer of zero.
//
// The s_relptr indicates the file offset for the Relocation information for this Section.
// The number of entries in the Relocation table is given in the s_nreloc field.
// If the s_nreloc field is zero, the section does not contain any Relocation information.
//
// Like the Relocation pointer, the s_lnnoptr field indicates the file offset to the Line number table for the section,
// whose length in entries is given in the s_nlnno field. As with the s_nreloc, if the s_nlnno is zero then the Section contains no Line number information.
//

typedef struct _coff_section_header
{
	char        s_name[8];  // Section Name
	int32_t     s_paddr;    // Physical Address
	int32_t     s_vaddr;    // Virtual Address
	int32_t     s_size;     // Section Size in Bytes
	int32_t     s_scnptr;   // File offset to the Section data
	int32_t     s_relptr;   // File offset to the Relocation table for this Section
	int32_t     s_lnnoptr;  // File offset to the Line Number table for this Section
	uint16_t    s_nreloc;   // Number of Relocation table entries
	uint16_t    s_nlnno;    // Number of Line Number table entries
	int32_t     s_flags;    // Flags for this section
}coff_section_header;

// The Flags field gives important information on how the Section should be handled when being processed.
// The most important flag values relate to the type of field as follows:

#define STYP_TEXT 0x0020    // The section contains executable code.
#define STYP_DATA 0x0040    // The section contains initialised data.
#define STYP_BSS  0x0080    // The COFF file contains no data for this section, but needs space to be allocated for it.


// ---------------
// Relocation Entries
// ---------------

// Using Relocation entries, it is possible to load a section of code into an arbitrary memory address and
// then modify the code so that the memory addresses for Symbols are correct. This means you can take a COFF file,
// load the .text section of the file into one place, the .data section into another, the .bss section somewhere else again,
// and then update the .text section so that the code is able to find its values and function correctly.
//
// The Relocation Table is an array of Relocation Entry structures and is zero based.
//
// A COFF file can contain a Relocation table for each Section. .text ( STYP_TEXT ) sections often have relocation information,
// it is also possible for .data ( STYP_DATA ) and other sections to have relocation information.
//
// The Relocation table for a section has its location and length stored in the Section's header.
//
// The structure of this header can be found in reloc.h, and is 10 bytes long.

// The Address field gives the offset within the Section's raw data where the address starts.
// The reference will almost always be a 32-bit value starting at the given address, but this is dependant
// on implementation and the r_type field.
//
// The Symbol index gives the (zero based) index in the Symbol table to which the reference refers.
// Once you have loaded the COFF file into memory and know where each symbol is,
// you find the new updated address for the given symbol and update the reference accordingly.
//
// The r_type indicates how the address should be updated. It is, unfortunately, implementation specific.
// For an example, DJGPP can indicate that this be the 32-bit absolute address of the symbol (type 6),
// or it can specify a 32-bit address that is relative to the location of the reference (type 20).

typedef struct _coff_reloc_entry
{
	int32_t     r_vaddr;     // Reference Address
	int32_t     r_symndx;    // Symbol index
	uint16_t    r_type;      // Type of relocation
}coff_reloc_entry;

// ---------------
// Line Number Entries
// ---------------

// Line number entries are used by debugging programs to relate symbols and physical instruction addresses to source file line numbers.
//
// The Line Number table is an array of Line Number Entry structures and is zero based.
//
// A COFF file can contain a Line number table for each Section, but it would be unusual
// to find line number information for anything other than .text ( STYP_TEXT ) Sections.
//
// Like the Relocation table, the Line Number table for a section has its location and length stored in the Section's header.
//
// The structure of this header can be found in linenum.h, and is 6 bytes long.
//

// In brief, The line number table will contain an entry with a line number
// of zero in which the l_symndx field will indicate the function name Symbol in the Symbol table.
// This entry will be followed by additional entries with incrementing line numbers which indicate,
// through the l_paddr field, the byte offset into the section where this line starts. Given this,
// an exception that occurs during processing of the COFF file can be traced back to a function
// and a line number within that function.

typedef struct _coff_linenumber_entry
{
	union
	{
		long        l_symndx;       // Symbol Index
		long        l_paddr;        // Physical Address
	} l_addr;
	unsigned short      l_lnno;     // Line Number
}coff_linenumber_entry;

// ---------------
// Symbol Table
// ---------------

// The Symbol Table contains information on each symbol that is declared or defined by this file.
// There will be symbols in this table for each function name, for each global variable, for each static variable,
// for each variable defined as extern, etc. There are even symbols in this table that indicate the start of
// each section and further information about the file as a whole.
//
// The Symbol table is an array of Symbol Entry structures and is zero based.
//
// The Symbol table location is given in the File Header f_symptr field, and its length in entries is given in the File Header f_nsyms field.
//
// The structure of this header can be found in syms.h, and is 18 bytes long.
//

// The Symbol name field stores the name of this symbol if it is eight characters or fewer,
// otherwise this field contains a pointer into the String table. See the entry on the String table below.
//
// Each symbol in the Symbol table can be followed by zero or more Auxiliary entries which provide further information about the symbol.
// Each of these entries is the same length as the Symbol table entry, but has a different structure depending on the parent Symbol.
//
// The Section number field n_scnum indicates the Section to which this Symbol belongs.
// If this value is greater than zero, it refers to a Section in the Section table (hence why the Section table is one-based).
// I can also take one of the following special values:
//
// The n_value, n_scnum and n_sclass fields in the table need to be considered together because they are interlinked.
// A number of the more common combinations of these fields is given in the following table.
// (Much of the information in the following table is from observation and not from referenced sources. It is incomplete and may not be accurate.)

// n_sclass     n_scnum             n_value               Meaning of n_value    Typical use
// --------------------------------------------------------------------------------------------------------------------
// C_EXT (2)    0                   0                                           Unresolved external Symbol
//              0                   >0                    Size of variable      Uninitialised global variable (not included in BSS)
//              .text               Any                   Offset into Section   Function entry point
//              .data               Any                   Offset into Section   Initialised global variable
// C_STAT (3)   .text, .data, .bss  0                                           Section Symbol indicating start of Section
//              .data               Any                   Offset into Section   Initialised static variable
//              .bss                Any                   Offset into Section   Unitialised static variable


// As noted in the table, uninitialised global variables do not seem to be included in the size of the BSS section,
// only unitialised static variables. If you are loading a COFF file into memory,
// it may be required to allocate space in addition to the BSS section size to cover these symbols.

typedef struct _coff_symbol_table
{
	uint8_t     n_name[8];  // Symbol Name
	int32_t     n_value;    // Value of Symbol
	int16_t     n_scnum;    // Section Number
	uint16_t    n_type;     // Symbol Type
	char        n_sclass;   // Storage Class
	char        n_numaux;   // Auxiliary Count
}coff_symbol_table;

// n_scnum meaning
#define N_DEBUG 2           //  A debugging symbol. In the example below, information about the file has been put into a symbol like this.
#define N_ABS   1           //  An absolute symbol. This means that the value of the n_value field is an absolute value.
#define N_UNDEF 0           //  An undefined external symbol

// ---------------
// String Table
// ---------------

// The String table contains string names for any sections or symbols that are longer than eight characters.
//
// The String table immediately follows the Symbol table, and can be located by using the following formula:
//
// String Table Offset = File Header.f_symptr + File Header.f_nsyms * sizeof( Symbol Table Entry )
//
// The first four bytes of the string table are an integer indicating the overall size in bytes of the string table.
// Each of the strings in the table are null terminated.
//
// The Section table name field and the Symbol table name field are actaully more complicated than was detailed above,
// they in fact look more like this:

typedef struct _coff_name
{
	union
	{
		char    name[8];
		struct
		{
			unsigned long zeroes;
			unsigned long offset;
		};
	};
}coff_name;

// If the name is eight characters or fewer, then the field "zeroes" will be non-zero,
// and "name" should be interpretted as a character array. Note that this field is not
// null-terminated unless it is fewer than eight characters in length.
//
// If the name is more than eight characters, the "zeroes" field (the first four bytes of "name")
// will be zero.
// In this case the "offset" field should be used as an offset value into the String
// table to locate the Symbol or Section name.

#pragma pack()
