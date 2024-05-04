#ifndef MEDIASERVER_COMMONEXCEPTION_H
#define MEDIASERVER_COMMONEXCEPTION_H

#include <exception>

class CommonException : public std::exception {
    const char* error_msg;
public:
    CommonException() : error_msg{"Error."} {}
    explicit CommonException(const char* error_msg) {
        this->error_msg = error_msg;
    }
    [[nodiscard]] const char* what() const noexcept override {
        return this->error_msg;
    }
};

#endif //MEDIASERVER_COMMONEXCEPTION_H