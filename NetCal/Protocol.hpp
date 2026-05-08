// Protocol.hpp
#pragma once
#include "Logger.hpp"
#include <functional>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <string>

// 请求报文
// 一个请求,形如x+y,由三个成员组成
class Request
{
public:
    Request() {}

    Request(int x, int y, char oper) : _x(x), _y(y), _oper(oper) {}

    // 序列化
    void Serialize(std::string* out)
    {
        Json::Value root;
        root["x"] = _x;
        root["y"] = _y;
        root["oper"] = _oper;

        Json::FastWriter writer;
        *out = writer.write(root);
    }

    // 反序列化
    void Deserialize(std::string& in)
    {
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(in, root))
        {
            LOG(LogLevel::WARNING) << "Json parse error";
            return;
        }

        _x = root["x"].asInt();
        _y = root["y"].asInt();
        _oper = root["oper"].asInt();
    }

    int Getx() const
    {
        return _x;
    }

    int Gety() const
    {
        return _y;
    }

    char Getoper() const
    {
        return _oper;
    }

private:
    int _x;
    int _y;
    char _oper;
};

// 应答报文
// 一个应答,需要包含运算结果和错误标志(如除零等错误)
class Response
{
public:
    Response() {}

    Response(const std::string& res, int code) : _res(res), _code(code) {}

    // 序列化
    void Serialize(std::string* out)
    {
        Json::Value root;
        root["res"] = _res;
        root["code"] = _code;

        Json::FastWriter writer;
        *out = writer.write(root);
    }

    // 反序列化
    void Deserialize(std::string& in)
    {
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(in, root))
        {
            LOG(LogLevel::WARNING) << "Json parse error";
            return;
        }

        _res = root["res"].asString();
        _code = root["code"].asInt();
    }

    std::string Getres() const
    {
        return _res;
    }

    int Getcode() const
    {
        return _code;
    }

private:
    std::string _res;
    int _code;
};

// 协议类，需要包含封装方法、解包方法、分析报文方法
// 我们的协议，约定报文格式为: "有效载荷长度/r/n序列化json字符串/r/n"
const std::string gsep = "\r\n";

// 需要提供怎么处理请求和应答的业务方法函数, 服务端需要知道怎么处理请求并返回应答，客户端需要知道怎么处理应答
using HanderRequest_t = std::function<Response(Request&)>;
using HanderResponse_t = std::function<void(Response&)>;

class Protocol
{
public:
    Protocol(HanderRequest_t handler_request) : _handler_request(handler_request) {}

    Protocol(HanderResponse_t handler_response) : _handler_response(handler_response) {}

    // 封装报文方法
    std::string Packet(const std::string& json_string)
    {
        // "有效载荷长度/r/n序列化json字符串/r/n"
        return std::to_string(json_string.size()) + gsep + json_string + gsep;
    }

    // 解包方法
    // packet表示当前读到的字符串
    // 如果能在packet中读到一个完整报文，返回1。否则返回0.读到的json串记录到第二个参数中
    // 如果出现其他错误，返回-1
    int Unpack(std::string& packet, std::string* json_string)
    {
        if (packet.empty())
        {
            return 0;
        }
        if (json_string == nullptr)
        {
            return -1;
        }

        // 首先提取报头，报头代表有效载荷长度
        auto pos = packet.find(gsep);
        if (pos == std::string::npos)
        {
            return 0;
        }

        std::string lenstr = packet.substr(0, pos);
        int len = std::stoi(lenstr);
        int total_len = lenstr.size() + len + 2 * gsep.size();

        // 如果当前packet长度小于这条报文总长，肯定读不到; 反之则能
        if (packet.size() < total_len)
        {
            return 0;
        }

        *json_string = packet.substr(pos + gsep.size(), len);
        packet.erase(0, total_len);
        return 1;
    }

    // 分析请求
    // 如果读到不是完整的报文，什么都不做，继续读取
    // 如果读到了完整的报文，交给计算函数处理
    std::string ParseRequest(std::string& inbuffer)
    {
        // 如果能读到多个请求报文，都处理完再一次性返回所有的应答报文
        std::string result;

        while (1)
        {
            // 1. 获取请求json字符串, 解包
            std::string json_string;
            int n = Unpack(inbuffer, &json_string);
            if (n < 0)
            {
                LOG(LogLevel::WARNING) << "nullptr error";
                return std::string();
            }
            if (n == 0)
            {
                // 读不到更多报文了
                LOG(LogLevel::INFO) << inbuffer << "parse done";
                return result;
            }
            if (n == 1)
            {
                LOG(LogLevel::INFO) << "json_string:\n" << json_string;
                LOG(LogLevel::INFO) << "inbuffer:\n" << inbuffer;

                // 现在，有了一个完整的json串，是一个请求
                // 2. 请求反序列化
                Request req;
                req.Deserialize(json_string);

                // 3. 业务处理。我协议层不知道怎么处理，交给业务处理层，只要返回一个应答就行了
                Response resp;
                resp = _handler_request(req);

                // 4. 应答序列化
                std::string resp_json_string;
                resp.Serialize(&resp_json_string);

                // 5. 添加报头
                result += Packet(resp_json_string);
            }
        }
    }

    std::string ParseResponse(std::string& inbuffer)
    {
        while (1)
        {
            // 1. 获取应答json字符串, 解包
            std::string json_string;
            int n = Unpack(inbuffer, &json_string);
            if (n < 0)
            {
                LOG(LogLevel::WARNING) << "nullptr error";
                return std::string();
            }
            if (n == 0)
            {
                // 读不到更多报文了
                LOG(LogLevel::INFO) << inbuffer << "parse done";
                return std::string();
            }
            if (n == 1)
            {
                // 2. 应答反序列化
                Response resp;
                resp.Deserialize(json_string);

                // 3. 业务处理，我协议层不知道怎么处理应答，回调函数
                _handler_response(resp);
            }
        }
    }

private:
    HanderRequest_t _handler_request;
    HanderResponse_t _handler_response;
};
