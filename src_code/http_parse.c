//
// Latest edit by TeeKee on 2017/7/23.
//

#include "http.h"

int tk_http_parse_request_line(tk_http_request_t *request){
    /* 有限状态机：由于http的报文是文本形式，无法按照字节的形式提取报文中的字段，因此采用有限状态机的方式来完成。
                 http报文中的字段有限，可以将不同字段的识别过程设置为不同的状态，每识别到分割字符时，就改变当前的状态。 */
    /* 枚举常量表用于表示http request的识别状态 */
    /* sw_表示short word */
    enum{
        /* 下列短词中，没写的指针指向情况一般是报错或跳过 */

        /* （短：通常情况下只进入一次；报错）开始解析http request文件，进入时指向请求方法中的第一个字符，也有可能是空行（换行标志，会跳过） */
        sw_start = 0,
        /* （长：通常情况下会进入多次；报错或跳过）开始解析请求方法字段，直到指向请求方法后面的空格 */
        /* 进入时指向请求方法中的第二个字符 */
        sw_method,
        /* （短；报错）开始解析URL地址，进入时指向URL地址的第一个字符'/'，也有可能是http版本号前面的多余空格（会跳过） */
        /* 命名中的before可以理解为在pos指向请求方法后的空格时获得该状态 */
        sw_spaces_before_uri,
        /* （长；跳过）解析URL地址，进入时指向URL地址'/'后面的一个字符，直到指向URL地址后面的空格 */
        sw_after_slash_in_uri,
        /* （短；报错）开始解析HTTP版本号，进入时指向版本号的第一个字符'H'，也有可能是http版本号前面的多余空格（会跳过） */
        sw_http,
        /* （短；报错）开始解析HTTP版本号第一个字符，进入时指向版本号的第二个字符'T' */
        sw_http_H,
        /* （短；报错）开始解析HTTP版本号第二个字符，进入时指向版本号的第三个字符'T' */
        sw_http_HT,
        /* （短；报错）开始解析HTTP版本号第三个字符，进入时指向版本号的第四个字符'P' */
        sw_http_HTT,
        /* （短；报错）开始解析HTTP版本号第四个字符，进入时指向版本号的第五个字符'/' */
        sw_http_HTTP,
        /* （短；报错）开始解析HTTP主版本号的第一个数字，进入时指向主版本号的第一个数字 */
        sw_first_major_digit,
        /* （短；报错）开始解析HTTP主版本号的第二个数字，进入时指向'.'，也有可能是主版本号的第二个数字 */
        sw_major_digit,
        /* （短；报错）开始解析HTTP副版本号的第一个数字，进入时指向副版本号的第一个数字 */
        sw_first_minor_digit,
        /* （短；报错）开始解析HTTP副版本号的第二个数字，进入时指向换行标志（回车符或换行符），也有可能是副版本号的第二个数字或多余空格（空格会跳过） */
        sw_minor_digit,
        /* （短；报错）开始解析换行标志，指向换行标志的第一个字符，也有可能是多余空格（空格会跳过） */
        sw_spaces_after_digit,
        /* （短；报错）考虑操作系统是Windows的情况，开始解析换行标志的第二个字符，指向换行标志的第二个字符 */
        sw_almost_done
    }state;
    state = request->state;

    u_char ch, *p, *m;
    size_t pi;
    for(pi = request->pos; pi < request->last; pi++){
        p = (u_char *)&request->buff[pi % MAX_BUF];
        ch = *p;

        switch(state){
            case sw_start:
                request->request_start = p;
                /* ch == CR || ch == LF，02？？？排除请求行前的空行 */
                if(ch == CR || ch == LF)
                    break;
                /* ch != '_'，02？？？排除无效请求方法，在Nginx的源码中，还包括这一条件：&& ch != '-' */
                if((ch < 'A' || ch > 'Z') && ch != '_')
                    return TK_HTTP_PARSE_INVALID_METHOD;
                state = sw_method;
                break;
        
            case sw_method:
                if(ch == ' '){
                    request->method_end = p;
                    m = request->request_start;
                    switch(p - m){
                        case 3:
                            /* 使用逐字节比较而不是字符串比较的好处在于前者速度更快 */
                            if(tk_str3_cmp(m, 'G', 'E', 'T', ' ')){
                                request->method = TK_HTTP_GET;
                                break;
                            }
                            /* 此处应该报错或者跳到default而不是break，因为此时method未初始化，01？？？
                               会在case sw_spaces_before_uri里return TK_HTTP_PARSE_INVALID_REQUEST */
                            break;
                        case 4:
                            if(tk_str3Ocmp(m, 'P', 'O', 'S', 'T')){
                                request->method = TK_HTTP_POST;
                                break;
                            }
                            if(tk_str4cmp(m, 'H', 'E', 'A', 'D')){
                                request->method = TK_HTTP_HEAD;
                                break;
                            }
                            /* 此处应该报错或者跳到default而不是break，因为此时method未初始化，01？？？ 
                               会在case sw_spaces_before_uri里return TK_HTTP_PARSE_INVALID_REQUEST */
                            break;
                        default:
                            request->method = TK_HTTP_UNKNOWN;
                            /* break后继续解析其余字段，但此时的method被初始化为TK_HTTP_UNKNOWN，01？？？
                               会在case sw_spaces_before_uri里return TK_HTTP_PARSE_INVALID_REQUEST */
                            break;
                    }
                    state = sw_spaces_before_uri;
                    break;
                }

                /* ch != '_'，02？？？排除无效请求方法，在Nginx的源码中，还包括这一条件：&& ch != '-' */
                if((ch < 'A' || ch > 'Z') && ch != '_')
                    return TK_HTTP_PARSE_INVALID_METHOD;
                break;

            case sw_spaces_before_uri:
                if(ch == '/'){
                    request->uri_start = p + 1;
                    state = sw_after_slash_in_uri;
                    break;
                }
                switch(ch){
                    case ' ':
                        /* 过滤掉URL地址前多余的空格 */
                        break;
                    default:
                        return TK_HTTP_PARSE_INVALID_REQUEST;
                }
                /* 此处没必要加break，01？？？ */
                break;

            case sw_after_slash_in_uri:
                switch(ch){
                    case ' ':
                        request->uri_end = p;
                        state = sw_http;
                        break;
                    default:
                        break;
                }
                /* 此处没必要加break，01？？？ */                
                break;

            case sw_http:
                switch(ch){
                    /* 过滤掉URL地址前多余的空格 */
                    case ' ':
                        break;
                    case 'H':
                        state = sw_http_H;
                        break;
                    default:
                        return TK_HTTP_PARSE_INVALID_REQUEST;
                }
                /* 此处没必要加break，01？？？ */                
                break;

            case sw_http_H:
                switch(ch){
                    case 'T':
                        state = sw_http_HT;
                        break;
                    default:
                        return TK_HTTP_PARSE_INVALID_REQUEST;
                }
                /* 此处没必要加break，01？？？ */                
                break;

            case sw_http_HT:
                switch(ch){
                    case 'T':
                        state = sw_http_HTT;
                        break;
                    default:
                        return TK_HTTP_PARSE_INVALID_REQUEST;
                }
                /* 此处没必要加break，01？？？ */                
                break;

            case sw_http_HTT:
                switch(ch){
                    case 'P':
                        state = sw_http_HTTP;
                        break;
                    default:
                        return TK_HTTP_PARSE_INVALID_REQUEST;
                }
                /* 此处没必要加break，01？？？ */                
                break;

            case sw_http_HTTP:
                switch(ch){
                    case '/':
                        state = sw_first_major_digit;
                        break;
                    default:
                        return TK_HTTP_PARSE_INVALID_REQUEST;
                }
                /* 此处没必要加break，01？？？ */                 
                break;

            case sw_first_major_digit:
                if(ch < '1' || ch > '9')
                    return TK_HTTP_PARSE_INVALID_REQUEST;
                /* ch - '0'，可以方便计算出主版本号的第一个数字 */
                request->http_major = ch - '0';
                state = sw_major_digit;
                break;

            case sw_major_digit:
                if(ch == '.'){
                    state = sw_first_minor_digit;
                    break;
                }
                if(ch < '0' || ch > '9')
                    return TK_HTTP_PARSE_INVALID_REQUEST;
                /* 考虑到主版本号更迭到双位数的情况，此时指向主版本号的第二个字符，实际上现在只有0.9/1./1.1/2.0四个版本 */
                request->http_major = request->http_major * 10 + ch - '0';
                break;

            case sw_first_minor_digit:
                /* 指向副版本号的第一个字符 */
                if(ch < '0' || ch > '9')
                    return TK_HTTP_PARSE_INVALID_REQUEST;
                request->http_minor = ch - '0';
                state = sw_minor_digit;
                break;

            case sw_minor_digit:
                /* 指向回车符（Mac系统换行标志或Window系统换行标志的第一个字符） */
                if(ch == CR){
                    state = sw_almost_done;
                    break;
                }
                /* 指向换行符（Unix系统换行标志） */
                if(ch == LF)
                    goto done;
                /* 指向空格的情况 */
                if(ch == ' '){
                    state = sw_spaces_after_digit;
                    break;
                }
                if(ch < '0' || ch > '9')
                    return TK_HTTP_PARSE_INVALID_REQUEST;
                /* 考虑到副版本号更迭到双位数的情况，此时指向副版本号的第二个字符，实际上现在只有0.9/1./1.1/2.0四个版本 */
                request->http_minor = request->http_minor * 10 + ch - '0';
                break;

            case sw_spaces_after_digit:
                switch(ch){
                    /* 忽略副版本号之后，换行标志之前的多余空格 */
                    case ' ':
                        break;
                    /* 对于Windows的换行标志 */    
                    case CR:
                        state = sw_almost_done;
                        break;
                    /* 对于Unix的换行标志 */
                    case LF:
                        goto done;
                    default:
                        return TK_HTTP_PARSE_INVALID_REQUEST;
                }
                break;

            case sw_almost_done:
                /* 考虑是Wnindows的情况 */
                request->request_end = p - 1;
                switch(ch){
                    case LF:
                        goto done;
                    default:
                        return TK_HTTP_PARSE_INVALID_REQUEST;
                }
        }
    }
    request->pos = pi;
    request->state = state;
    return TK_AGAIN;

    done:
    /* request->pos指向请求头的第一个字符 */
    request->pos = pi + 1;
    /* request_end什么时候不为NULL，02？？？？
       对于Windows系统而言，request->request_end指向回车符，不进入该if语句 */
    if (request->request_end == NULL)
        /* request_end指向请求行末尾的换行符 */
        request->request_end = p;
    request->state = sw_start;
    return 0;
}

int tk_http_parse_request_body(tk_http_request_t *request){
    // 状态列表
    enum{
        sw_start = 0,
        sw_key,
        sw_spaces_before_colon,
        sw_spaces_after_colon,
        sw_value,
        sw_cr,
        sw_crlf,
        sw_crlfcr
    }state;
    state = request->state;

    size_t pi;
    unsigned char ch, *p;
    tk_http_header_t *hd;
    for (pi = request->pos; pi < request->last; pi++) {
        p = (unsigned char*)&request->buff[pi % MAX_BUF];
        ch = *p;

        switch(state){
            case sw_start:
                if(ch == CR || ch == LF)
                    break;
                request->cur_header_key_start = p;
                state = sw_key;
                break;

            case sw_key:
                if(ch == ' '){
                    request->cur_header_key_end = p;
                    state = sw_spaces_before_colon;
                    break;
                }
                if(ch == ':'){
                    request->cur_header_key_end = p;
                    state = sw_spaces_after_colon;
                    break;
                }
                break;

            case sw_spaces_before_colon:
                if(ch == ' ')
                    break;
                else if(ch == ':'){
                    state = sw_spaces_after_colon;
                    break;
                }
                else
                    return TK_HTTP_PARSE_INVALID_HEADER;

            case sw_spaces_after_colon:
                if (ch == ' ')
                    break;
                state = sw_value;
                request->cur_header_value_start = p;
                break;

            case sw_value:
                if(ch == CR){
                    request->cur_header_value_end = p;
                    state = sw_cr;
                }
                if(ch == LF){
                    request->cur_header_value_end = p;
                    state = sw_crlf;
                }
                break;

            case sw_cr:
                if(ch == LF){
                    state = sw_crlf;
                    hd = (tk_http_header_t *) malloc(sizeof(tk_http_header_t));
                    hd->key_start = request->cur_header_key_start;
                    hd->key_end = request->cur_header_key_end;
                    hd->value_start = request->cur_header_value_start;
                    hd->value_end = request->cur_header_value_end;
                    list_add(&(hd->list), &(request->list));
                    break;
                }
                else
                    return TK_HTTP_PARSE_INVALID_HEADER;

            case sw_crlf:
                if(ch == CR)
                    state = sw_crlfcr;
                else{
                    request->cur_header_key_start = p;
                    state = sw_key;
                }
                break;

            case sw_crlfcr:
                switch(ch){
                    case LF:
                        goto done;
                    default:
                        return TK_HTTP_PARSE_INVALID_HEADER;
                }
        }
    }
    request->pos = pi;
    request->state = state;
    return TK_AGAIN;

    done:
    request->pos = pi + 1;
    request->state = sw_start;
    return 0;
}
