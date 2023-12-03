#include "parser.h"

Parser::Parser()
{}

Parser::~Parser()
{}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string* output)
{
    size_t total_size = size * nmemb;
    output->append(static_cast<char*>(contents), total_size);
    return total_size;
}


std::vector<book_node> Parser::get_list(std::string book_name)
{
    curl_initialize();
    std::vector<std::string> books;
    books = get_book_list(book_name);
    if(books[0] == "none")
    {
        book_list.push_back({"none", "none", "none"});
    }
    else
        get_book_info(books);
    curl_easy_cleanup(curl); 
    return book_list;
}

void Parser::curl_initialize()
{
    Parser::curl = curl_easy_init();//инициализируем curl
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);//настройка соединения(не менять)
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);//настройка callback-функции(не менять)
    if(!curl)
    {
        std::cerr << "Failed to initialize libcurl." << std::endl;
    }
}

std::vector<std::string> Parser::get_book_list(std::string book_name)
{
    std::replace(book_name.begin(), book_name.end(), ' ', '+');
    std::string url = "http://flub.flibusta.is/booksearch?ask=\"" + book_name + "\"";// url format
    
    std::string response_buffer;//буфер данных для страницы с найденными книгами
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());//настройка адреса
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);//настройка буфера
    CURLcode res = curl_easy_perform(curl);

    if(res != CURLE_OK)
    {
        std::cerr << "Failed to perform HTTP request: " << curl_easy_strerror(res) << std::endl;
    }
    
    dom = dom_parser.parseTree(response_buffer);
    std::vector<std::string> books;
    int max = 10;
    for(tree<htmlcxx::HTML::Node>::iterator it = dom.begin(); it != dom.end(); ++it)//Получаем список книг
    {
        if(books.size() == max)
            break;
        if(it->isTag() && it->tagName() == "p")
        {
            ++it;
            if(it->text().find("Ничего не найдено") != std::string::npos)
            {
                if(books.size() == 0)
                    books.push_back("none");
                else
                    books[0] = "none";
                return books; 
            }
        }
        if (it->isTag() && it->tagName() == "a")//Поиск книг
        {
            it->parseAttributes();
            if(it->attribute("href").second.find("/b/") != std::string::npos)
            {
                books.push_back(it->attribute("href").second);
            }
        }
        if(it->isTag() && it->tagName() == "h3")//Поиск количества найденных книг
        {
            ++it;
            if(it->text().find("книги") != std::string::npos)
            {
                int result = std::stoi(it->text().substr(it->text().size() - 3, 1));
                if(result < max)
                {
                    if(!std::isdigit(it->text()[it->text().size() - 4]))
                        max = result;
                }
            }
        }
    }
    return books;
}

void Parser::get_book_info(std::vector<std::string> books)
{
    int max = books.size();
    for(int i = 0; i < books.size() && i < max; ++i)
    {
        std::string new_url = "http://flub.flibusta.is" + books[i];
        std::string author, name, buffer;
        
        curl_easy_setopt(curl, CURLOPT_URL, new_url.c_str());//новый url
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);//новый буффер
        CURLcode new_res = curl_easy_perform(curl);
        
        if(new_res != CURLE_OK)
        {
            std::cerr << "Failed to perform HTTTP request: " << curl_easy_strerror(new_res) << std::endl;
        }
        
        dom = dom_parser.parseTree(buffer);//получение нового дерева
        
        for (tree<htmlcxx::HTML::Node>::iterator it = dom.begin(); it != dom.end(); ++it) 
        {
            if (it->isTag() && it->tagName() == "h1")//Получаем название книги из заголовка
            {
                ++it;
                if(!it->isTag() && it->text().find("(") != std::string::npos)//проверяем, если ли ( в тексте(так надо)
                {
                    name = it->text();
                    do
                    {
                        ++it;
                    }while(it->tagName() != "a"); //Переходим далее по дереву, пока не находим автора
                    ++it;
                    author = it->text(); 
                    break;
                }
            }
        }
        book_list.push_back({name, author, new_url});
    }
}
