// Calculator.hpp
#pragma once
#include "Protocol.hpp"
#include <string>
class Calculator
{
public:
    Response Execute(const Request& req)
    {
        int x = req.Getx();
        int y = req.Gety();
        char oper = req.Getoper();

        int res = 0;
        int code = 0;
        switch (oper)
        {
        case '+':
            res = x + y;
            break;
        case '-':
            res = x - y;
            break;
        case '*':
            res = x * y;
            break;
        case '/':
            if (y == 0)
                code = 1;
            else
                res = x / y;
            break;
        case '%':
            if (y == 0)
                code = 2;
            else
                res = x % y;
            break;
        default:
            code = 3;
            break;
        }
        std::string res_string = "error";
        if (code == 0)
        {
            res_string = std::to_string(x) + oper + std::to_string(y) + '=' + std::to_string(res);
        }
        Response resp(res_string, code);
        return resp;
    }
};
