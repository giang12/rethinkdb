#ifndef __PROTOCOL_REDIS_REDIS_HPP__
#define __PROTOCOL_REDIS_REDIS_HPP__

#include "arch/arch.hpp"
#include "store.hpp"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
using std::string;

//Redis result types. All redis commands return one of these types. Any redis parser
//should know how to serialize these
enum redis_status {
    OK,
    ERROR
};

struct status_result_struct {
    redis_status status;
    const char *msg;
};

typedef const boost::shared_ptr<status_result_struct> status_result;
typedef unsigned integer_result;
typedef const boost::shared_ptr<std::string> bulk_result;
typedef const boost::shared_ptr<std::vector<std::string> > multi_bulk_result;

//These macros deliberately correspond to those in the redis parser. Though it may
//seem a bit silly to turn a simple method declaration into a macro like this (it
//really isn't any less typing) it does allow us to simply copy the declaration of
//a new command from the parser when adding a new command and it makes the
//correspondence between commands in the parser and in the redis_interface clearer.
#define COMMAND_0(RETURN, CNAME) RETURN##_result CNAME();
#define COMMAND_1(RETURN, CNAME, ARG_TYPE_ONE) RETURN##_result CNAME(ARG_TYPE_ONE);
#define COMMAND_2(RETURN, CNAME, ARG_TYPE_ONE, ARG_TYPE_TWO)\
            RETURN##_result CNAME(ARG_TYPE_ONE, ARG_TYPE_TWO);
#define COMMAND_3(RETURN, CNAME, ARG_TYPE_ONE, ARG_TYPE_TWO, ARG_TYPE_THREE)\
            RETURN##_result CNAME(ARG_TYPE_ONE, ARG_TYPE_TWO, ARG_TYPE_THREE);
#define COMMAND_N(RETURN, CNAME)\
            RETURN##_result CNAME(std::vector<string>&);

//The abstract redis interface. Importantly it is not dependent on the vaguaries
//of any particular protocol. A unified text request protocol parser or a lua
//scripting layer or even a staticly linked redis client library could sit
//on top of it. Have fun!
struct redis_interface_t {
    redis_interface_t(get_store_t *get_store, set_store_interface_t *set_store);
    ~redis_interface_t();

    //KEYS
    COMMAND_N(integer, del)
    COMMAND_1(integer, exists, string&)
    COMMAND_2(integer, expire, string&, unsigned)
    COMMAND_2(integer, expireat, string&, unsigned)
    COMMAND_1(multi_bulk, keys, string&)
    COMMAND_2(integer, move, string&, string&)
    COMMAND_1(integer, persist, string&)
    COMMAND_0(bulk, randomkey)
    COMMAND_2(status, rename, string&, string&)
    COMMAND_2(integer, renamenx, string&, string&)
    COMMAND_1(integer, ttl, string&)
    COMMAND_1(status, type, string&)

    //Strings
    COMMAND_2(integer, append, string&, string&)
    COMMAND_1(integer, decr, string&)
    COMMAND_2(integer, decrby, string&, int)
    COMMAND_1(bulk, get, string&)
    COMMAND_2(integer, getbit, string&, unsigned)
    COMMAND_3(bulk, getrange, string&, int, int)
    COMMAND_2(bulk, getset, string&, string&)
    COMMAND_1(integer, incr, string&)
    COMMAND_2(integer, incrby, string&, int)
    COMMAND_N(multi_bulk, mget)
    COMMAND_N(status, mset)
    COMMAND_N(integer, msetnx)
    COMMAND_2(status, set, string&, string&)
    COMMAND_3(integer, setbit, string&, unsigned, unsigned)
    COMMAND_3(status, setex, string&, unsigned, string&)
    COMMAND_3(integer, setrange, string&, unsigned, string&)
    COMMAND_1(integer, Strlen, string&)
    
protected:
    get_store_t *get_store;
    set_store_interface_t *set_store;
};

//Equivalently (perhaps deliberately equivalently :) ) named macros might live
//in files that include this header. Let's be good citizens.
#undef COMMAND_0
#undef COMMAND_1
#undef COMMAND_2
#undef COMMAND_3
#undef COMMAND_N
#endif /* __PROTOCOL_REDIS_REDIS_HPP__ */
