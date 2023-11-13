#pragma once

#include "flash.h"
#include "reflect_ext.h"

template <class Data, class Updater1, class Updater2, class Updater3>
class Safe_flash_updater_impl : private TickSubscriber {
public:
    Safe_flash_updater_impl(Data*);
    ~Safe_flash_updater_impl() { tick_unsubscribe(); }

private:
    Updater1 updater1;
    Updater2 updater2;
    Updater3 updater3;
    int current;
    void notify() override;
};
