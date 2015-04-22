#include <acqua/text/json_parser.hpp>

int main(int, char ** argv)
{
    int x;
    acqua::text::basic_json_parser<char, int> json(x);
    char const * s = argv[1];
    while(json.parse(*s), *s++)
      ;
}
