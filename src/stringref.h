#ifndef TEST_STRINGREF_H
#define TEST_STRINGREF_H

#include <string>

class stringref {
    char const* base_ = nullptr;
    char const* end_ = nullptr;

private:
    stringref(char const* s, char const* t)
        : base_(s)
        , end_(t) {}

public:
    stringref() noexcept = default;
    ~stringref() = default;

    stringref(stringref const&) noexcept = default;
    stringref& operator=(stringref const&) noexcept = default;

    stringref(std::string const& s) noexcept
        : base_(s.data())
        , end_(s.data() + s.size()) {}

    template <typename P>
    stringref try_feed(P&& f) {
        stringref ret{base_, base_};

        while (base_ < end_ && f(*base_)) {
            ++base_;
        }

        ret.end_ = base_;
        return ret;
    }

    void advance() {
        base_++;
    }

    stringref try_feed(std::string const& s) {
        if (std::equal(s.begin(), s.end(), base_, std::min(end_, base_ + s.size()))) {
            stringref ret{base_, base_ + s.size()};
            base_ += s.size();
            return ret;
        }

        return {};
    }

    std::string str() const {
        return std::string(base_, end_);
    }

    char operator[](size_t i) const noexcept {
        return base_[i];
    }

    bool empty() const noexcept {
        return base_ == end_;
    }

    char const* begin() const noexcept {
        return base_;
    }

    char const* end() const noexcept {
        return end_;
    }

    friend bool operator==(stringref const& s1, std::string const& s2) {
        return std::equal(s1.begin(), s1.end(), s2.begin(), s2.end());
    }
};


#endif //TEST_STRINGREF_H
