#include <exception>
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

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string* output)
{
    size_t total_size = size * nmemb;
    output->append(static_cast<char*>(contents), total_size);
    return total_size;
}

struct book_node
{
    std::string name;
    std::string author;
    std::string url;
};

std::vector<book_node>  parser(std::string book_name)
{
    std::vector<book_node> book_list;// Список книг
    CURL* curl = curl_easy_init();//инициализируем curl
    int max = 10;//Максимальный размер выдачи
    if(!curl)//Проверка подключения
    {
        std::cerr << "Failed to initialize libcurl." << std::endl;
    }
    std::replace(book_name.begin(), book_name.end(), ' ', '+');
    std::string url = "http://flub.flibusta.is/booksearch?ask=\"" + book_name + "\"";// url format
    std::string response_buffer;//буфер данных для страницы с найденными книгами
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());//настройка адреса
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);//настройка буфера
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);//настройка соединения(не менять)
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);//настройка callback-функции(не менять)
    
    CURLcode res = curl_easy_perform(curl);
    
    if(res != CURLE_OK)
    {
        std::cerr << "Failed to perform HTTTP request: " << curl_easy_strerror(res) << std::endl;
    }

    try
    {
        htmlcxx::HTML::ParserDom dom_parser; 
        tree<htmlcxx::HTML::Node> dom = dom_parser.parseTree(response_buffer);
        std::vector<std::string> books;         
        for (tree<htmlcxx::HTML::Node>::iterator it = dom.begin(); it != dom.end(); ++it) //Получение спииска найденных книг
        {
            if(books.size() == max)
                break;
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
    
    catch(const std::exception& e)
    {
        std::cerr << "Exception while parsing: " << e.what() << std::endl;
        curl_easy_cleanup(curl);
        return book_list;
    }
    curl_easy_cleanup(curl);
    return book_list;
}

int main()
{
    std::vector<book_node> book_list;
    std::cout << "Введите название книги: ";
    std::string book;
    getline(std::cin, book);
    std::cout << book << std::endl;
    book_list = parser(book);
    for(int i = 0; i < book_list.size(); ++i)
    {
        std::cout << book_list[i].name << " " << book_list[i].author << "   " << book_list[i].url << std::endl;
    }
    return 0;
}
