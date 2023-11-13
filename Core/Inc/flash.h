#pragma once

#include <cstring> // std::memcpy
#include <algorithm>
#include <iterator>
//#include "periph_flash.h"
#include "timers.h"



template<size_t n, class Int = size_t>
class SizedInt {
    Int value {0};
public:
    inline Int operator++(int)   { auto v = value; value = (++value < n) ? value : 0; return v; }
    inline Int operator++()      { return value = (++value < n) ? value : 0; }
    inline Int operator--(int)   { return value = (value == 0) ? n : value - 1; }
    inline operator Int() const  { return value; }
    inline Int operator= (Int v) { return value = v; }
};

struct Pair {
    uint8_t offset;
    uint8_t value;
    operator uint16_t() { return uint16_t(value) << 8 | offset; }
};

union Word {
    Pair     pair;
    uint16_t data;
};

struct Memory {
    Word*  pointer;
    size_t size;
    Memory (Word* pointer, size_t size)
        :  pointer {pointer}
        ,  size {size}
    {}

    class Iterator {
        Word* p {nullptr};
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = Word;
        using difference_type   = size_t;
        using pointer           = const Word*;
        using reference         = Word;
        Iterator (Word* p) : p{p} {}
        Iterator() = default;
        operator Word*() { return p; }
        Word&        operator*  () const { return *p; }
        Word*        operator-> () const { return p; }
        bool         operator!= (const Iterator& other) const {return p != other.p; }
        Iterator& operator++ () {
            p++;
            return *this;
        }
        Iterator& operator+ (int v) {
            p += v;
            return *this;
        }
    };

    Iterator begin() {return Iterator{pointer};}
    Iterator end()   {return Iterator{pointer + size};}
};

template <class Data, FLASH_::Sector ... sector>
class Flash_updater_impl : private TickSubscriber
{
public:
    Flash_updater_impl(Data*);
    Flash_updater_impl(); // не читает данные
    ~Flash_updater_impl() { stop(); }
    void start() { tick_subscribe(); }
    void stop()  { tick_unsubscribe(); }
    void set_data(Data* v) { original = v; }
    void read_to(Data* data) {
        original = data;
        // flash.lock(); // check if need
        if (not is_read())
            *data = Data{};
    }
    bool done() {
        auto v = done_;
        done_ = false;
        return v;
    }
private:
    FLASH_&  flash   {mcu::make_reference<mcu::Periph::FLASH>()};
    Data*    original;
    uint8_t  copy[sizeof(Data)];
    static constexpr auto sectors {std::array{sector...}} ;
    SizedInt<sizeof...(sector)> current {};
    bool need_erase[sizeof...(sector)] {};
    int  erase_index {0};
    std::array<Memory, sizeof...(sector)> memory;
    Memory::Iterator memory_offset{nullptr};
    bool done_ {false};


    enum State {
      check_changes,
      start_write,
      check_write,
      erase,
      check_erase,
      rewrite
    };
    volatile State state {check_changes};
    volatile State return_state {check_changes};
    volatile uint8_t writed_data; // TODO: проверить без volatile
    SizedInt<sizeof(Data), uint8_t> data_offset {};

    // возвращает true , если данные прочитаны
    //            false, если нет или данные не полные
    bool is_read();
    void notify() override;
    bool is_need_erase();

};
