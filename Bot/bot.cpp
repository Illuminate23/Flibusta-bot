#include "parser.h"

int main()
{
    Parser parser;
    std::vector<book_node> book_list = parser.get_list("Мастер и Маргарита");
    for(int i = 0; i < book_list.size(); ++i)
    {
        std::cout << book_list[i].name << " " << book_list[i].author << " " << book_list[i].url << std::endl;
    }
    return 0;
}
