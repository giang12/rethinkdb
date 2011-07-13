#include "protocol/memcached/file.hpp"
#include "protocol/memcached/memcached.hpp"
#include "progress/progress.hpp"
#include "concurrency/fifo_checker.hpp"

/* `file_memcached_interface_t` is a `memcached_interface_t` that reads queries
from a file and ignores the responses to its queries. */

class file_memcached_interface_t : public memcached_interface_t {

private:
    FILE *file;
    file_progress_bar_t progress_bar;
    signal_t *interrupt;

public:
    file_memcached_interface_t(const char *filename, signal_t *interrupt) :
        file(fopen(filename, "r")),
        progress_bar(std::string("Import"), file),
        interrupt(interrupt)
    { }
    ~file_memcached_interface_t() {
        fclose(file);
    }

    /* We throw away the responses */
    void write(UNUSED const char *buffer, UNUSED size_t bytes) { }
    void write_unbuffered(UNUSED const char *buffer, UNUSED size_t bytes) { }
    void flush_buffer() { }
    bool is_write_open() { return false; }

    void read(void *buf, size_t nbytes) {
        if (interrupt->is_pulsed()) throw no_more_data_exc_t();
        if (fread(buf, nbytes, 1, file) == 0)
            throw no_more_data_exc_t();
    }

    void read_line(std::vector<char> *dest) {
        if (interrupt->is_pulsed()) throw no_more_data_exc_t();
        int limit = MEGABYTE;
        dest->clear();
        char c; 
        const char *head = "\r\n";
        while ((*head) && ((c = getc(file)) != EOF) && (limit--) > 0) {
            dest->push_back(c);
            if (c == *head) head++;
            else head = "\r\n";
        }
        //we didn't every find a crlf unleash the exception 
        if (*head) throw no_more_data_exc_t();
    }
};

/* In our current use of import we ignore gets, the easiest way to do this
 * is with a dummyed get_store */
class dummy_get_store_t : public get_store_t {
    get_result_t get(UNUSED const store_key_t &key, UNUSED order_token_t token) { return get_result_t(); }
    rget_result_t rget(UNUSED rget_bound_mode_t left_mode, UNUSED const store_key_t &left_key,
                       UNUSED rget_bound_mode_t right_mode, UNUSED const store_key_t &right_key, 
                       UNUSED order_token_t token)
    {
        return rget_result_t();
    }
};

void import_memcache(const char *filename, set_store_interface_t *set_store, signal_t *interrupt) {
    rassert(interrupt);
    interrupt->assert_thread();

    dummy_get_store_t dummy_get_store;
    file_memcached_interface_t interface(filename, interrupt);

    handle_memcache(&interface, &dummy_get_store, set_store, MAX_CONCURRENT_QUEURIES_ON_IMPORT);
}
