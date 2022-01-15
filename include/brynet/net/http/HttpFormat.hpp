#pragma once

#include <array>
#include <cassert>
#include <map>
#include <string>

namespace brynet { namespace net { namespace http {

class HttpQueryParameter final
{
public:
    void add(const std::string& k, const std::string& v)
    {
        if (!mParameter.empty())
        {
            mParameter += "&";
        }

        mParameter += k;
        mParameter += "=";
        mParameter += v;
    }

    const std::string& getResult() const
    {
        return mParameter;
    }

private:
    std::string mParameter;
};

class HttpRequest final
{
public:
    enum class HTTP_METHOD
    {
        HTTP_METHOD_HEAD,
        HTTP_METHOD_GET,
        HTTP_METHOD_POST,
        HTTP_METHOD_PUT,
        HTTP_METHOD_DELETE,
        HTTP_METHOD_MAX
    };

    HttpRequest()
    {
        setMethod(HTTP_METHOD::HTTP_METHOD_GET);
    }

    void setMethod(HTTP_METHOD protocol)
    {
        mMethod = protocol;
        assert(mMethod > HTTP_METHOD::HTTP_METHOD_HEAD &&
               mMethod < HTTP_METHOD::HTTP_METHOD_MAX);
    }

    void setHost(const std::string& host)
    {
        addHeadValue("Host", host);
    }

    void setUrl(const std::string& url)
    {
        mUrl = url;
    }

    void setCookie(const std::string& v)
    {
        addHeadValue("Cookie", v);
    }

    void setContentType(const std::string& v)
    {
        addHeadValue("Content-Type", v);
    }

    void setQuery(const std::string& query)
    {
        mQuery = query;
    }

    void setBody(const std::string& body)
    {
        addHeadValue("Content-Length", std::to_string(body.size()));
        mBody = body;
    }

    void setBody(std::string&& body)
    {
        addHeadValue("Content-Length", std::to_string(body.size()));
        mBody = std::move(body);
    }

    void addHeadValue(const std::string& field,
                      const std::string& value)
    {
        mHeadField[field] = value;
    }

    std::string getResult() const
    {
        const auto MethodMax = static_cast<size_t>(HTTP_METHOD::HTTP_METHOD_MAX);
        const static std::array<std::string, MethodMax> HttpMethodString =
                {"HEAD", "GET", "POST", "PUT", "DELETE"};

        std::string ret;
        if (mMethod >= HTTP_METHOD::HTTP_METHOD_HEAD &&
            mMethod < HTTP_METHOD::HTTP_METHOD_MAX)
        {
            ret += HttpMethodString[static_cast<size_t>(mMethod)];
        }

        ret += " ";
        ret += mUrl;
        if (!mQuery.empty())
        {
            ret += "?";
            ret += mQuery;
        }

        ret += " HTTP/1.1\r\n";

        for (auto& v : mHeadField)
        {
            ret += v.first;
            ret += ": ";
            ret += v.second;
            ret += "\r\n";
        }

        ret += "\r\n";

        if (!mBody.empty())
        {
            ret += mBody;
        }

        return ret;
    }

private:
    std::string mUrl;
    std::string mQuery;
    std::string mBody;
    HTTP_METHOD mMethod;
    std::map<std::string, std::string> mHeadField;
};

class HttpResponse final
{
public:
    /*
    * Thanks to boost::beast
    */
    enum class status : unsigned
    {
        /** An unknown status-code.

        This value indicates that the value for the status code
        is not in the list of commonly recognized status codes.
        Callers interested in the exactly value should use the
        interface which provides the raw integer.
    */
        unknown = 0,

        continue_ = 100,

        /** Switching Protocols

        This status indicates that a request to switch to a new
        protocol was accepted and applied by the server. A successful
        response to a WebSocket Upgrade HTTP request will have this
        code.
    */
        switching_protocols = 101,

        processing = 102,

        ok = 200,
        created = 201,
        accepted = 202,
        non_authoritative_information = 203,
        no_content = 204,
        reset_content = 205,
        partial_content = 206,
        multi_status = 207,
        already_reported = 208,
        im_used = 226,

        multiple_choices = 300,
        moved_permanently = 301,
        found = 302,
        see_other = 303,
        not_modified = 304,
        use_proxy = 305,
        temporary_redirect = 307,
        permanent_redirect = 308,

        bad_request = 400,
        unauthorized = 401,
        payment_required = 402,
        forbidden = 403,
        not_found = 404,
        method_not_allowed = 405,
        not_acceptable = 406,
        proxy_authentication_required = 407,
        request_timeout = 408,
        conflict = 409,
        gone = 410,
        length_required = 411,
        precondition_failed = 412,
        payload_too_large = 413,
        uri_too_long = 414,
        unsupported_media_type = 415,
        range_not_satisfiable = 416,
        expectation_failed = 417,
        misdirected_request = 421,
        unprocessable_entity = 422,
        locked = 423,
        failed_dependency = 424,
        upgrade_required = 426,
        precondition_required = 428,
        too_many_requests = 429,
        request_header_fields_too_large = 431,
        connection_closed_without_response = 444,
        unavailable_for_legal_reasons = 451,
        client_closed_request = 499,

        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503,
        gateway_timeout = 504,
        http_version_not_supported = 505,
        variant_also_negotiates = 506,
        insufficient_storage = 507,
        loop_detected = 508,
        not_extended = 510,
        network_authentication_required = 511,
        network_connect_timeout_error = 599
    };


    HttpResponse(status state = status::ok,bool isKeepAlive = true)
        : mStatus(state)
    {
        if (isKeepAlive)
        {
            addHeadValue("Connection", "Keep-Alive");
        }
        else
        {
            addHeadValue("Connection", "Close");
        }
    }

    void setStatus(status status)
    {
        mStatus = status;
    }

    void setContentType(const std::string& v)
    {
        addHeadValue("Content-Type", v);
    }

    void addHeadValue(const std::string& field,
                      const std::string& value)
    {
        mHeadField[field] = value;
    }

    void setBody(const std::string& body)
    {
        addHeadValue("Content-Length", std::to_string(body.size()));
        mBody = body;
    }

    void setBody(std::string&& body)
    {
        addHeadValue("Content-Length", std::to_string(body.size()));
        mBody = std::move(body);
    }
    std::string getResult() const
    {
        std::string ret = "HTTP/1.1 ";

        ret += std::to_string(static_cast<int>(mStatus));
        ret += ' ';
        ret += toStatusString();

        ret += "\r\n";

        for (auto& v : mHeadField)
        {
            ret += v.first;
            ret += ": ";
            ret += v.second;
            ret += "\r\n";
        }

        ret += "\r\n";

        if (!mBody.empty())
        {
            ret += mBody;
        }

        return ret;
    }

private:
    status mStatus;
    std::map<std::string, std::string> mHeadField;
    std::string mBody;
    const char* toStatusString() const
    {
        switch (mStatus)
        {
            // 1xx
            case status::continue_:
                return "Continue";
            case status::switching_protocols:
                return "Switching Protocols";
            case status::processing:
                return "Processing";

            // 2xx
            case status::ok:
                return "OK";
            case status::created:
                return "Created";
            case status::accepted:
                return "Accepted";
            case status::non_authoritative_information:
                return "Non-Authoritative Information";
            case status::no_content:
                return "No Content";
            case status::reset_content:
                return "Reset Content";
            case status::partial_content:
                return "Partial Content";
            case status::multi_status:
                return "Multi-Status";
            case status::already_reported:
                return "Already Reported";
            case status::im_used:
                return "IM Used";

            // 3xx
            case status::multiple_choices:
                return "Multiple Choices";
            case status::moved_permanently:
                return "Moved Permanently";
            case status::found:
                return "Found";
            case status::see_other:
                return "See Other";
            case status::not_modified:
                return "Not Modified";
            case status::use_proxy:
                return "Use Proxy";
            case status::temporary_redirect:
                return "Temporary Redirect";
            case status::permanent_redirect:
                return "Permanent Redirect";

            // 4xx
            case status::bad_request:
                return "Bad Request";
            case status::unauthorized:
                return "Unauthorized";
            case status::payment_required:
                return "Payment Required";
            case status::forbidden:
                return "Forbidden";
            case status::not_found:
                return "Not Found";
            case status::method_not_allowed:
                return "Method Not Allowed";
            case status::not_acceptable:
                return "Not Acceptable";
            case status::proxy_authentication_required:
                return "Proxy Authentication Required";
            case status::request_timeout:
                return "Request Timeout";
            case status::conflict:
                return "Conflict";
            case status::gone:
                return "Gone";
            case status::length_required:
                return "Length Required";
            case status::precondition_failed:
                return "Precondition Failed";
            case status::payload_too_large:
                return "Payload Too Large";
            case status::uri_too_long:
                return "URI Too Long";
            case status::unsupported_media_type:
                return "Unsupported Media Type";
            case status::range_not_satisfiable:
                return "Range Not Satisfiable";
            case status::expectation_failed:
                return "Expectation Failed";
            case status::misdirected_request:
                return "Misdirected Request";
            case status::unprocessable_entity:
                return "Unprocessable Entity";
            case status::locked:
                return "Locked";
            case status::failed_dependency:
                return "Failed Dependency";
            case status::upgrade_required:
                return "Upgrade Required";
            case status::precondition_required:
                return "Precondition Required";
            case status::too_many_requests:
                return "Too Many Requests";
            case status::request_header_fields_too_large:
                return "Request Header Fields Too Large";
            case status::connection_closed_without_response:
                return "Connection Closed Without Response";
            case status::unavailable_for_legal_reasons:
                return "Unavailable For Legal Reasons";
            case status::client_closed_request:
                return "Client Closed Request";
            // 5xx
            case status::internal_server_error:
                return "Internal Server Error";
            case status::not_implemented:
                return "Not Implemented";
            case status::bad_gateway:
                return "Bad Gateway";
            case status::service_unavailable:
                return "Service Unavailable";
            case status::gateway_timeout:
                return "Gateway Timeout";
            case status::http_version_not_supported:
                return "HTTP Version Not Supported";
            case status::variant_also_negotiates:
                return "Variant Also Negotiates";
            case status::insufficient_storage:
                return "Insufficient Storage";
            case status::loop_detected:
                return "Loop Detected";
            case status::not_extended:
                return "Not Extended";
            case status::network_authentication_required:
                return "Network Authentication Required";
            case status::network_connect_timeout_error:
                return "Network Connect Timeout Error";

            default:
                break;
        }
        return "<unknown-status>";
    }
};

}}}// namespace brynet::net::http
