#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <shared_mutex>
#include <sstream>
#include <thread>

#include "httplib.h"
#include "ord.h"

class animation {
    size_t bound;

    ord::ordinal o;
    std::shared_mutex m;

    std::thread t;
    std::condition_variable_any cv;
    bool state = false;
    bool stopped = false;

    std::vector<size_t> wait_time;

 public:
    animation (size_t bound, size_t rec, size_t ums): bound (bound), wait_time (1, rec) {
        for (size_t i = 0; i < bound; ++i) {
            wait_time.push_back (wait_time.back () * (rec + 1) / rec);
        }

        t = std::thread ([this, ums] {
            for (size_t ut = 0;;) {
                std::unique_lock l (m);
                cv.wait (l, [this] { return state; });
                if (stopped) break;

                while (ut < ums) {
                    if (!o.to_next (this->bound)) {
                        stopped = true;
                        break;
                    }
                    ut += wait_time[this->bound - o.complexity ()];
                }

                l.unlock ();
                std::this_thread::sleep_for (std::chrono::milliseconds (ut / ums * 10));
                ut %= ums;
            }
        });
    }

    void start () {
        std::unique_lock l (m);
        state = true;
        l.unlock ();
        cv.notify_one ();
    }

    void pause () {
        std::unique_lock l (m);
        state = false;
    }

    std::optional<std::string> get () {
        std::shared_lock l (m);
        if (stopped) return {};

        std::stringstream ss;
        ss << o.std ();
        return ss.str ();
    }

    ~animation () {
        std::shared_lock l (m);
        stopped = true;
        l.unlock ();

        if (t.joinable ()) t.join ();
    }
};

int main (int argc, char** argv) {
    size_t bound = 100, rec = 10, ums = 2000;
    if (argc == 4) {
        try {
            bound = std::stoull (argv[1], nullptr, 10);
            rec = std::stoull (argv[2], nullptr, 10);
            ums = std::stoull (argv[3], nullptr, 10);
        } catch (...) {
            bound = 100, rec = 10, ums = 2000;
        }
    }

    std::ifstream findex ("index.html");
    if (!findex) {
        std::cerr << "index.html not found" << std::endl;
        return 0;
    }
    std::stringstream ss;
    ss << findex.rdbuf ();
    std::string index = ss.str ();

    animation a (bound, rec, ums);

    httplib::Server svr;

    svr.set_default_headers ({{"Access-Control-Allow-Origin", "*"},
                              {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
                              {"Access-Control-Allow-Headers", "Content-Type"}});

    svr.Options (".*", [] (const httplib::Request&, httplib::Response& res) { res.status = 200; });

    svr.Get ("/", [&] (const httplib::Request&, httplib::Response& res) { res.set_content (index, "text/html"); });

    svr.Get ("/next", [&] (const httplib::Request&, httplib::Response& res) {
        auto str = a.get ();
        res.set_content ((str.has_value () ? str.value () : "---"), "text/plain");
    });

    svr.Post ("/control/resume", [&] (const httplib::Request&, httplib::Response& res) { a.start (); });

    svr.Post ("/control/pause", [&] (const httplib::Request&, httplib::Response& res) { a.pause (); });

    svr.listen ("0.0.0.0", 8080);

    return 0;
}
