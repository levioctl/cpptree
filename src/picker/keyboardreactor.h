#ifndef KEYBOARD_REACTOR_H__
#define KEYBOARD_REACTOR_H__

namespace picker {

template<typename T>
class KeyboardReactor
{
public:
    KeyboardReactor(std::function<void(void)> initialize_callback,
                    std::function<bool(char)> char_pressed_callback);

    void run(void);

    void trigger_from_stdin(boost::asio::io_service *io_service);

private:
    boost::asio::io_service _io_service;
    std::function<void(void)> _initialize_callback;
    std::function<bool(char)> _char_pressed_callback;
};

template<typename T>
KeyboardReactor<T>::KeyboardReactor(std::function<void(void)> initialize_callback,
                                    std::function<bool(char)> char_pressed_callback):
    _io_service(),
    _initialize_callback(initialize_callback),
    _char_pressed_callback(char_pressed_callback)
{
}

template<typename T>
void KeyboardReactor<T>::run(void) {
    boost::asio::io_service::work work(_io_service);

    _initialize_callback();
    // Handle keyboard events from a separate thread.
    // See https://stackoverflow.com/questions/10934444/asio-service-handler-for-stdin-keypress
    std::thread listen_thread(&KeyboardReactor<T>::trigger_from_stdin, this, &_io_service);
    _io_service.run();

    listen_thread.join();
    endwin();
}

template<typename T>
void KeyboardReactor<T>::trigger_from_stdin(boost::asio::io_service *io_service) {
    assert(io_service != NULL);
    bool is_finished = false;
    while (not is_finished) {
        const char c = getch();
        auto one_menu_action =
            [&]() { is_finished = _char_pressed_callback(c); };
        _io_service.dispatch(one_menu_action);
    }
    kill(getpid(), SIGTERM);
}

}

#endif
