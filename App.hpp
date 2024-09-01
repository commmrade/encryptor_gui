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
    Main() {
        printf("Constructor\n");

        auto settings = get_settings();
        settings->property_gtk_application_prefer_dark_theme().set_value(true);

        set_title("Test");
        set_default_size(1280/ 2, 720 / 2);
        set_resizable(false);

        
        caution.set_visible(false);

        //Creating elements
        Gtk::CenterBox center;
        Gtk::Box box(Gtk::Orientation::VERTICAL, 10);
        Gtk::Box box2(Gtk::Orientation::HORIZONTAL, 5);


        Gtk::Label name("The great KEncryptor");
        Gtk::Button load_file_btn("Load file");
        Gtk::Button encrypt_btn("Encrypt");
        Gtk::Button decrypt_btn("Decrypt");

        Gtk::Button save_file_btn("Save file");

        //Buttons setup functions
        load_file_btn.signal_clicked().connect(sigc::mem_fun(*this, &Main::on_button_load_file));

        encrypt_btn.signal_clicked().connect(sigc::mem_fun(*this, &Main::on_button_encrypt));
        decrypt_btn.signal_clicked().connect(sigc::mem_fun(*this, &Main::on_button_decrypt));

        save_file_btn.signal_clicked().connect(sigc::mem_fun(*this, &Main::on_button_save_file));

        //Adding elements to box    
        box.append(caution);
        box.append(name);
        box.append(load_file_btn);
        box2.append(encrypt_btn);
        box2.append(decrypt_btn);
        box.append(box2);
        box.append(save_file_btn);


        center.set_margin(720 / 2 / 2);
        center.set_center_widget(box);

        set_child(center);

    }
    ~Main() {
        printf("Destructor\n");
    }


    void on_button_load_file() {
        caution.set_visible(false); //Hiding sign in case it is visible

        auto dialog = Gtk::FileDialog::create();
        dialog->open(sigc::bind(sigc::mem_fun(*this, &Main::on_file_load_finish), dialog)); 
    }

    void file_load_chunk(const std::string &name, std::streampos start, std::streamsize size) {
        std::ifstream file;
        file.open(name, std::ios::binary);

        if (file.is_open()) {
            file.seekg(start);
            std::vector<char> local_buf(size);
            file.read(local_buf.data(), size); //Reading data to local buffer
            
            std::lock_guard<std::mutex> lk(lock);
            std::copy(local_buf.begin(), local_buf.end(), file_data.begin() + start); //Adding data to the main buffer
            file.close();
        }


        //file.close();
    }


    void on_file_load_finish(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog)
    {
        try
        {
            auto file = dialog->open_finish(result);

           
            auto filename = file->get_path();
            std::cout << "File selected: " <<  filename << std::endl;

            std::ifstream file_open;
            file_open.open(filename, std::ios::ate);

            auto file_size = file_open.tellg(); //Figuring out file size
            file_open.seekg(0, std::ios::beg);

            file_data.resize(file_size);


            size_t num_threads = std::thread::hardware_concurrency();
            size_t chunk_size = file_size / num_threads;

            std::vector<std::thread> threads;
            for (size_t i = 0; i < num_threads; i++) {
                auto start = i * chunk_size; //Figuring out start pos
                auto size = (i == num_threads - 1) ? ((int)file_size - start) : chunk_size; //figureing out how much we need to read
                //(could also just figure out the end)

                threads.emplace_back(&Main::file_load_chunk, this, std::ref(filename), start, size); //Creating a thread
            }

            for (auto &thr : threads) {
                thr.join(); //Activating threads
            }
            
            file_open.close();
            file_loaded = true;

            printf("done loading\n");
        }
        catch (const Gtk::DialogError& err)
        {
            
            std::cout << "No file selected. " << err.what() << std::endl;
        }
        catch (const Glib::Error& err)
        {
            std::cout << "Unexpected exception. " << err.what() << std::endl;
        } 
    }

    void on_button_encrypt() {
        
        if (!file_data.empty() && (file_data[file_data.size() - 3] == 'k' && file_data[file_data.size() - 2] == 'l' && file_data[file_data.size() - 1] == 'e')) {
            caution.set_text("Already encrypted");
            caution.set_visible();
            //Thats some kind of a mark that is left in an encrypted file
            return;
        }


        if (file_loaded) {
            set_sensitive(false);
            xor_crypt("afkfkdsdkj"); //Encrypting with this key
            set_sensitive(true);
        } else {
            caution.set_text("Please, load a file first");
            caution.set_visible(true);
        }
    }
    void on_button_decrypt() {
        if (!file_data.empty() && (file_data[file_data.size() - 3] != 'k' && file_data[file_data.size() - 2] != 'l' && file_data[file_data.size() - 1] != 'e')) {
            caution.set_text("Already decrypted/Not encrypted");
            caution.set_visible();
            //Thats some kind of a mark that is left in an encrypted file
            return;
        } else {
            caution.set_visible(false);
        }


        if (file_loaded) {
            set_sensitive(false);
            xor_decrypt("afkfkdsdkj");
            set_sensitive(true);
        } else {
            caution.set_text("Please, load a file first");
            caution.set_visible(true); 
        }
    }


    void on_button_save_file() {
        if (file_loaded) {
            auto dialog = Gtk::FileDialog::create();
            dialog->save(sigc::bind(sigc::mem_fun(*this, &Main::on_file_save_finish), dialog));
        } else {
            caution.set_text("Please, load a file first");
            caution.set_visible();
        }
    }

    void file_save_chunk(const std::string &filename, std::streampos start, std::streamsize size) {
        
        std::ofstream file;
        file.open(filename);

        if (file.is_open()) {
            
            file.seekp(start);
            std::lock_guard<std::mutex> lk(lock);
            std::vector<char> local_buf(size);
            std::copy(file_data.begin() + start, file_data.begin() + start + size, local_buf.begin());
            //std::cout << local_buf.size() << std::endl;
           
            //file.write(local_buf.data(), size);
            
            for (auto &ch : local_buf) {
                file.put(ch);
            }
            file.flush();
            file.close();
        }

        
    }


    void on_file_save_finish(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog)
    {
 
        try
        {
            auto file = dialog->save_finish(result);

           
            auto filename = file->get_path();
            std::cout << "File selected: " <<  filename << std::endl;

            std::ofstream file_save;
            file_save.open(filename);

            if (file_save.is_open()) {
                for (auto ch : file_data) {
                    file_save.put(ch); //Loading data back to file but encrypted/decrypted
                }
            }

            file_data.clear();
            file_data.shrink_to_fit(); //Clearing memory that was used for deque

            file_loaded = false;

            printf("saved file");
            //////////////////////////////////////

            // std::ofstream file_save;
            // file_save.open(filename);
            // file_save.seekp(file_data.size() - 1);
            // // std::cout << file_data[file_data.size() - 3] << std::endl;
            // // std::cout << file_data[file_data.size() - 2] << std::endl;
            // // std::cout << file_data[file_data.size() - 1] << std::endl;
            // file_save.put('\0');
            // file_save.flush();
            // file_save.close();

            // size_t num_threads = std::thread::hardware_concurrency();
            // size_t file_size = file_data.size();
            // size_t chunk_size = file_size / num_threads;


            // std::vector<std::thread> threads;

            // for (size_t i = 0; i < num_threads; i++) {
            //     auto start = i * chunk_size;
            //     auto size = (i == num_threads - 1) ? (file_size - start) : chunk_size;

            //     threads.emplace_back(&Main::file_save_chunk, this, std::ref(filename), start, size);
            // }


            // for (auto &thr : threads) {
            //     thr.join();
            // }

            // file_loaded = false;

            // file_data.clear();
            // file_data.shrink_to_fit();

        }
        catch (const Gtk::DialogError& err)
        {
            
            std::cout << "No file selected. " << err.what() << std::endl;
        }
        catch (const Glib::Error& err)
        {
            std::cout << "Unexpected exception. " << err.what() << std::endl;
        } 
    }

    
    void xor_chunk(std::deque<char> &file_data, const std::string &key, size_t start, size_t end) {
        for (size_t i = start; i < end; i ++) {
            file_data[i] ^= key[i % key.size()];
        }
    }

    //Utils functions
    void xor_crypt(const std::string &key)
    {
        size_t num_threads = std::thread::hardware_concurrency();

        size_t chunk_size = file_data.size() / num_threads;
        
        std::vector<std::thread> threads;

        for (size_t i = 0; i < num_threads; ++i) {
            auto start = i * chunk_size;
            auto end = (i == num_threads - 1) ? file_data.size() : start + chunk_size;
            auto thread = std::thread(&Main::xor_chunk, this, std::ref(file_data), std::ref(key), start, end);
    
            // Move the thread into the vector
            threads.emplace_back(std::move(thread));
        }

        for(auto &thr : threads) {
            thr.join();
        }

        file_data.push_back('k');
        file_data.push_back('l');
        file_data.push_back('e'); //Leaving signature
    }




    void xor_decrypt(const std::string &key)
    {
        file_data.pop_back();
        file_data.pop_back();
        file_data.pop_back(); //Removing signat ure

        size_t num_threads = std::thread::hardware_concurrency();

        size_t chunk_size = file_data.size() / num_threads;
        
        std::vector<std::thread> threads;

        for (size_t i = 0; i < num_threads; ++i) {
            auto start = i * chunk_size;
            auto end = (i == num_threads - 1) ? file_data.size() : start + chunk_size; //If chunk_size doesnt result a full number
            auto thread = std::thread(&Main::xor_chunk, this, std::ref(file_data), std::ref(key), start, end);
    
            // Move the thread into the vector
            threads.emplace_back(std::move(thread));
        }

        for(auto &thr : threads) {
            thr.join();
        }
    }
};