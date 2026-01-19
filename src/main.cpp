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
    ord::ordinal o;
    std::shared_mutex m;

    std::thread t;
    std::condition_variable_any cv;
    std::atomic<bool> state = false;
    std::atomic<bool> stopped = false;

 public:
    animation (size_t ums, std::vector<size_t>&& wt) {
        t = std::thread ([this, ums, wt = std::move (wt)] {
            const auto bound = wt.size ();

            for (size_t ut = 0;;) {
                std::unique_lock l (m);
                cv.wait (l, [this] () -> bool { return state; });
                if (stopped) break;

                while (ut < ums) {
                    if (!o.to_next (bound)) {
                        stopped = true;
                        break;
                    }
                    ut += wt[bound - o.complexity ()];
                }

                l.unlock ();
                std::this_thread::sleep_for (std::chrono::milliseconds (ut / ums * 10));
                ut %= ums;
            }
        });
    }

    void start () {
        state = true;
        cv.notify_one ();
    }

    void pause () { state = false; }

    std::optional<std::string> get () {
        if (stopped) return {};

        std::unique_lock l (m);

        std::stringstream ss;
        ss << o.std ();
        return ss.str ();
    }

    ~animation () {
        stopped = true;

        if (t.joinable ()) t.join ();
    }
};

int main (int argc, char** argv) {
    size_t ums = 10;
    std::vector<size_t> wait_time = {10, 12, 15, 22, 30, 50, 80, 120, 200, 300, 500, 800, 1200, 2000, 3000, 5000};

    if (argc > 2) {
        try {
            ums = std::stoull (argv[1], nullptr, 10);
            for (size_t i = 2; i < argc; ++i) wait_time.push_back (std::stoull (argv[1], nullptr, 10));
        } catch (...) {
            std::cerr << "invalid argument" << std::endl;
            return 0;
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

    animation a (ums, std::move (wait_time));

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

    svr.Get ("/control/resume", [&] (const httplib::Request&, httplib::Response& res) {
        a.start ();
        res.status = 204;
    });

    svr.Get ("/control/pause", [&] (const httplib::Request&, httplib::Response& res) {
        a.pause ();
        res.status = 204;
    });

    svr.listen ("0.0.0.0", 8080);

    return 0;
}
