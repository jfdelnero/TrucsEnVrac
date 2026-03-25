typedef struct html_specchars_
{
	char ascii;
	char * decimal_code;
	char * html_code;
}html_specchars;

const html_specchars * is_special_char(char c);

const html_specchars * is_special_char2(char c);
