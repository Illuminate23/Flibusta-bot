#pragma once
#include <iostream>
#include <cstdlib>
#include <curl/curl.h>
#include <ostream>
#include <string>
#include <algorithm>
#include <htmlcxx/html/ParserDom.h>
#include <htmlcxx/html/utils.h>
#include <utility>
#include <vector>
#include <ctype.h>

struct book_node
{
    std::string name;
    std::string author;
    std::string url;
};


class Parser{
    private:
        std::vector<book_node> book_list;
        CURL* curl;
        htmlcxx::HTML::ParserDom dom_parser;
        tree<htmlcxx::HTML::Node> dom;
    public:
        Parser();
        ~Parser();
        std::vector<book_node> get_list(std::string book_name);
        void curl_initialize();
        std::vector<std::string> get_book_list(std::string book_name);
        void get_book_info(std::vector<std::string> books);
};
