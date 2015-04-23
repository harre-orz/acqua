#include <acqua/text/json_parser.hpp>

int main(int, char ** argv)
{
    int x;
    acqua::text::json_parser<int, char> parser(x);
    char const *s = argv[1];
    while( parser.parse(*s), *s++ )
        ;
}
