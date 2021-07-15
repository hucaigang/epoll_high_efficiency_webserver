#ifndef __HTTP_HPP__
#define __HTTP_HPP__
#include<map>
#include<string>
enum HTTP{
    GET,
    POST,
    OPTION
};
class http_requests{
public:
    http_requests(int m){
        Method = m;
        row ="HTTP/1.1 200 OK \r\n";
        body = "<!DOCTYPE html>\n\
                <html>\n\
                <head>\n\
                <meta charset='utf-8'>\n\
                <title>菜鸟教程(runoob.com)</title>\n\
                </head>\n\
                <body>\n\
                <h1>我的第一个标题</h1>\n\
                <p>我的第一个段落。</p>\n\
                </body>\n\
                </html>";
        header = "Server:Apache-Coyote/1.1\r\n\
                    Accept-Ranges:bytes\r\n\
                    Content-Type:text/html\r\n\
                    Date:Thu,30 Jun 2016 12:31:12 GMT\r\n\r\n";
        package = row+header+body;
    }
    std::string render_request(){
        return package;
    }
    int length(){
        return package.size();
    }
    static http_requests request;
private:
    int Method;
    std::string row;
    std::string header;
    std::string body;
    std::string package;
    std::map<std::string,std::string> para;
};
http_requests http_requests::request(HTTP::GET);
#endif