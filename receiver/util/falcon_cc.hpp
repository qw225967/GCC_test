///**
// * @file   falcon_cc.hpp
// * @author pengrl
// *
// */
//
//#pragma once
//
//#include <sys/time.h>
//#include <unistd.h>
//#include <atomic>
//#include <chrono>
//#include <map>
//#include <sstream>
//#include <string>
//#include <thread>
//#include <vector>
//#include "chef_task_thread.hpp"
//#include "helper.h"
//#include "log_adapter.hpp"
//#include "rapidjson_wrapper.hpp"
//
//using namespace rapidjson;
//
//namespace cls {
//
//template <class Dummy>
//class StatEntry_static {
//   protected:
//    struct StatInfo {
//        int max_   = 0;
//        int min_   = 2147483647;
//        int avg_   = 0;
//        int count_ = 0;
//        int total_ = 0;
//    };
//
//    struct StatPostData {
//        std::string metric_;
//        std::string endpoint_;
//        int64_t     timestamp_;
//        float       value_;
//        int         step_;
//        std::string counter_type_;
//        std::string tags_;
//
//        std::string stringify() const {
//            std::ostringstream oss;
//            oss << "{"
//                << "\"metric\":\"" << metric_ << "\","
//                << "\"endpoint\":\"" << endpoint_ << "\","
//                << "\"timestamp\":" << timestamp_ << ","
//                << "\"value\":" << value_ << ","
//                << "\"step\":" << step_ << ","
//                << "\"counterType\":\"" << counter_type_ << "\","
//                << "\"tags\":\"" << tags_ << "\""
//                << "}";
//            return oss.str();
//        }
//    };
//
//   protected:
//    // key code counter
//    typedef std::map<std::string, std::map<int, StatInfo> > StatInfoMap;
//
//   protected:
//    static std::string METRIC_EVENT_TOTAL;
//    static std::string METRIC_EVENT_RATE;
//    static std::string METRIC_EVENT_AVGTIME;
//    static std::string METRIC_EVENT_COUNT;
//
//   protected:
//    static std::mutex        mutex_;
//    static chef::task_thread backend_thread_;
//    static std::atomic<int>  timer_started_;
//    static StatInfoMap       core_;
//    static std::string       project_;
//    static std::string       falcon_ip_;
//    static uint16_t          falcon_port_;
//    static std::string       falcon_uri_;
//    static uint32_t          interval_sec_;
//    static uint32_t          port_;
//};
//
//class StatEntry : public StatEntry_static<void> {
//   public:
//    static void global_init(const std::string &project, uint32_t port, const std::string &falcon_ip = "127.0.0.1",
//                            uint16_t falcon_port = 1988, const std::string &falcon_uri = "/v1/push",
//                            uint32_t interval_sec = 60) {
//        {
//            std::lock_guard<std::mutex> lg(mutex_);
//            project_      = project;
//            falcon_ip_    = falcon_ip;
//            falcon_port_  = falcon_port;
//            falcon_uri_   = falcon_uri;
//            interval_sec_ = interval_sec;
//            port_         = port;
//        }
//        start_timer_once();
//    }
//
//    StatEntry(const std::string &event) : event_(event), start_(now_nanoseconds()) {}
//
//    ~StatEntry() {}
//
//    void End(const std::string &category, int code) {
//        uint64_t    duration = now_nanoseconds() - start_;
//        std::string key      = event_ + "." + category;
//
//        std::lock_guard<std::mutex> lg(mutex_);
//        StatInfo &                  old = core_[key][code];
//        old.count_++;
//        old.total_ += duration;
//        if (duration > (uint64_t)old.max_) {
//            old.max_ = duration;
//        }
//        if (duration < (uint64_t)old.min_) {
//            old.min_ = duration;
//        }
//    }
//
//   public:
//    static void global_uninit() {
//        timer_started_ = false;
//        backend_thread_.stop_and_join();
//    }
//
//   private:
//    static void start_timer_once() {
//        do {
//            std::lock_guard<std::mutex> lg(mutex_);
//            if (timer_started_) {
//                return;
//            }
//
//            timer_started_ = 1;
//        } while (0);
//
//        backend_thread_.start();
//        backend_thread_.add(std::bind(&StatEntry::timer_handler));
//    }
//
//    static void timer_handler() {
//        for (; timer_started_;) {
//            // std::cout << "CHEFERASEME timer loop\n";
//            std::this_thread::sleep_for(std::chrono::seconds(interval_sec_));
//
//            send(stringify(calc()));
//
//            std::lock_guard<std::mutex> lg(mutex_);
//            core_.clear();
//        }
//    }
//
//    static void send(const std::string &body) {
//        HTTPResponse resp;
//        Helper::get_or_post("http://127.0.0.1:1988/v1/push", NULL, NULL, body.c_str(), 3000, 3, resp);
//
//        CLS_LOG_INFO << "Report:Falcon, body:" << body.c_str() << ", resp:" << resp.content_;
//    }
//
//    static std::string stringify(const std::vector<StatPostData> &spds) {
//        std::ostringstream oss;
//        oss << "[";
//        if (!spds.empty()) {
//            if (spds.size() == 1) {
//                oss << spds[0].stringify();
//            } else {
//                std::size_t i = 0;
//                for (; i < spds.size() - 1; i++) {
//                    oss << spds[i].stringify() << ",";
//                }
//                oss << spds[i].stringify();
//            }
//        }
//        oss << "]";
//        return oss.str();
//    }
//
//    static std::vector<StatPostData> calc() {
//        StatPostData              spd;
//        std::vector<StatPostData> spds;
//
//        std::lock_guard<std::mutex> lg(mutex_);
//        for (auto &iter : core_) {
//            const std::string &event = iter.first;
//
//            int all_total = 0;
//            std::for_each(iter.second.begin(), iter.second.end(),
//                          [&](std::pair<const int, StatInfo> const &item) { all_total += item.second.count_; });
//            spd = make_spd(METRIC_EVENT_TOTAL, float(all_total), make_default_tags(event));
//            spds.push_back(spd);
//
//            for (auto &ci : iter.second) {
//                const int & code = ci.first;
//                StatInfo &  si   = ci.second;
//                float       rate = (float)si.count_ / all_total * 100;
//                float       avg  = (float)si.total_ / (si.count_ * 1000000);
//                std::string tags = make_tags(code, event);
//
//                spd = make_spd(METRIC_EVENT_RATE, rate, tags);
//                spds.push_back(spd);
//                spd = make_spd(METRIC_EVENT_AVGTIME, avg, tags);
//                spds.push_back(spd);
//                spd = make_spd(METRIC_EVENT_COUNT, si.count_, tags);
//                spds.push_back(spd);
//            }
//        }
//
//        return spds;
//    }
//
//    static StatPostData make_spd(const std::string &metric, float value, const std::string &tags) {
//        StatPostData spd;
//        spd.metric_        = metric;
//        char hostname[256] = {0};
//        gethostname(hostname, 255);
//        spd.endpoint_     = hostname;
//        spd.timestamp_    = now_unix_secconds();
//        spd.value_        = value;
//        spd.step_         = interval_sec_;
//        spd.counter_type_ = "GAUGE";
//        spd.tags_         = tags;
//
//        return spd;
//    }
//
//    static std::string make_default_tags(const std::string &event) {
//        std::ostringstream oss;
//        // std::cout << "CHEFERASEME " << project_ << "\n";
//        oss << "project=" << project_ << ",event=" << event << ",port=" <<port_;
//        return oss.str();
//    }
//
//    static std::string make_tags(int code, const std::string &event) {
//        std::ostringstream oss;
//        // std::cout << "CHEFERASEME " << project_ << "\n";
//        oss << "project=" << project_ << ",event=" << event  << ",port=" <<port_ << ",code=" << code;
//        return oss.str();
//    }
//
//   private:
//    static uint64_t now_unix_secconds() {
//        struct timeval tv;
//        gettimeofday(&tv, nullptr);
//        return (uint64_t)tv.tv_sec;
//    }
//
//    static uint64_t now_nanoseconds() {
//        struct timeval tv;
//        gettimeofday(&tv, nullptr);
//        return (uint64_t)tv.tv_sec * 1000000000 + (uint64_t)tv.tv_usec * 1000;
//    }
//
//   private:
//    std::string event_;
//    uint64_t    start_;
//};
//
//template <class Dummy>
//std::string StatEntry_static<Dummy>::METRIC_EVENT_TOTAL = "event.total";
//template <class Dummy>
//std::string StatEntry_static<Dummy>::METRIC_EVENT_RATE = "event.code.rate";
//template <class Dummy>
//std::string StatEntry_static<Dummy>::METRIC_EVENT_AVGTIME = "event.code.avgtime";
//template <class Dummy>
//std::string StatEntry_static<Dummy>::METRIC_EVENT_COUNT = "event.code.count";
//
//template <class Dummy>
//std::atomic<int> StatEntry_static<Dummy>::timer_started_;
//template <class Dummy>
//std::mutex StatEntry_static<Dummy>::mutex_;
//template <class Dummy>
//typename StatEntry_static<Dummy>::StatInfoMap StatEntry_static<Dummy>::core_;
//template <class Dummy>
//chef::task_thread StatEntry_static<Dummy>::backend_thread_;
//template <class Dummy>
//std::string StatEntry_static<Dummy>::project_;
//template <class Dummy>
//std::string StatEntry_static<Dummy>::falcon_ip_ = "127.0.0.1";
//template <class Dummy>
//uint16_t StatEntry_static<Dummy>::falcon_port_ = 1988;
//template <class Dummy>
//std::string StatEntry_static<Dummy>::falcon_uri_ = "/v1/push";
//template <class Dummy>
//uint32_t StatEntry_static<Dummy>::interval_sec_ = 60;
//template <class Dummy>
//uint32_t StatEntry_static<Dummy>::port_;
//
//}  // namespace cls
