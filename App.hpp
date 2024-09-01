#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/centerbox.h"
#include "gtkmm/enums.h"
#include "gtkmm/filedialog.h"
#include "gtkmm/label.h"
#include "gtkmm/settings.h"
#include "sigc++/adaptors/bind.h"
#include "sigc++/functors/mem_fun.h"
#include <algorithm>
#include <exception>
#include <fstream>
#include<gtkmm.h>
#include<deque>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

class Main : public Gtk::Window {
private:
    std::deque<char> file_data;
    bool file_loaded = false;

    Gtk::Label caution;
   
    std::mutex lock;
public: 
    Main();
    ~Main();


    void on_button_load_file();
    void file_load_chunk(const std::string &name, std::streampos start, std::streamsize size);
    void on_file_load_finish(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog);

    void on_button_encrypt();
    void on_button_decrypt();


    void on_button_save_file();
    void file_save_chunk(const std::string &filename, std::streampos start, std::streamsize size);
    void on_file_save_finish(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog);

    
    void xor_chunk(std::deque<char> &file_data, const std::string &key, size_t start, size_t end);
    void xor_crypt(const std::string &key);
    void xor_decrypt(const std::string &key);
};