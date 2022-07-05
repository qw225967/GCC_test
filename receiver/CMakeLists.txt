cmake_minimum_required(VERSION 3.20)
project(sender)

set(CMAKE_CXX_STANDARD 17)

set(BOOST_DIR /opt/homebrew/Cellar/boost/1.76.0)
include_directories(
        rtc/
        util/
        gcc_server/
        libwebrtc/libwebrtc/
        libwebrtc/deps/abseil-cpp/abseil-cpp/
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl
        json/include/
        openssl/openssl/include/
        openssl/openssl/include/openssl/
        ${BOOST_DIR}/include/
)

link_directories(
        ${BOOST_DIR}/lib/
)

set(LUALIB
        boost_log_setup-mt
        boost_log-mt
        )

aux_source_directory(rtc/ RTC_CC)
aux_source_directory(util/ UTIL_CC)
aux_source_directory(gcc_server/ GCC_SERVER_CC)
aux_source_directory(libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/ LIB_GCC_CC)

add_executable(sender
        main.cpp

        ${RTC_CC}
        ${UTIL_CC}
        ${GCC_SERVER_CC}
        ${LIB_GCC_CC}

        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/hash/internal/hash.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/hash/internal/city.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/container/internal/hashtablez_sampler.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/container/internal/hashtablez_force_sampling.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/container/internal/raw_hash_set.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/base/internal/raw_logging.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/debugging/stacktrace.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/clock.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/time.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/duration.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/internal/cctz/include/cctz/time_zone.h
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/internal/cctz/src/time_zone_info.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/internal/cctz/src/zone_info_source.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/internal/cctz/src/time_zone_if.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/internal/cctz/src/time_zone_libc.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/internal/cctz/src/time_zone_posix.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/internal/cctz/src/time_zone_fixed.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/internal/cctz/src/time_zone_lookup.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/time/internal/cctz/src/time_zone_impl.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/base/internal/cycleclock.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/base/internal/low_level_alloc.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/base/internal/spinlock.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/base/internal/spinlock_wait.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/base/internal/unscaledcycleclock.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/base/internal/thread_identity.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/base/internal/sysinfo.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/synchronization/internal/graphcycles.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/synchronization/internal/create_thread_identity.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/synchronization/internal/waiter.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/debugging/symbolize.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/numeric/int128.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/synchronization/mutex.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/synchronization/internal/per_thread_sem.cc
        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/types/bad_optional_access.cc

        libwebrtc/libwebrtc/call/rtp_transport_controller_send.cc
        libwebrtc/libwebrtc/modules/pacing/paced_sender.cc
        libwebrtc/libwebrtc/system_wrappers/source/field_trial.cc
        libwebrtc/libwebrtc/modules/pacing/bitrate_prober.cc
        libwebrtc/libwebrtc/api/transport/network_types.cc
        libwebrtc/libwebrtc/modules/pacing/interval_budget.cc
        libwebrtc/libwebrtc/modules/bitrate_controller/send_side_bandwidth_estimation.cc
        libwebrtc/libwebrtc/modules/remote_bitrate_estimator/bwe_defines.cc
        libwebrtc/libwebrtc/modules/bitrate_controller/loss_based_bandwidth_estimation.cc
        libwebrtc/libwebrtc/modules/congestion_controller/rtp/control_handler.cc
        libwebrtc/libwebrtc/modules/congestion_controller/rtp/transport_feedback_adapter.cc
        libwebrtc/libwebrtc/modules/congestion_controller/rtp/send_time_history.cc
        libwebrtc/libwebrtc/modules/rtp_rtcp/include/rtp_rtcp_defines.cc
        libwebrtc/libwebrtc/api/units/data_rate.cc
        libwebrtc/libwebrtc/api/transport/goog_cc_factory.cc
        libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/goog_cc_network_control.cc
        libwebrtc/libwebrtc/rtc_base/experiments/field_trial_parser.cc
        libwebrtc/libwebrtc/rtc_base/experiments/field_trial_units.cc
        libwebrtc/libwebrtc/modules/remote_bitrate_estimator/inter_arrival.cc
        libwebrtc/libwebrtc/modules/remote_bitrate_estimator/aimd_rate_control.cc
        libwebrtc/libwebrtc/rtc_base/experiments/rate_control_settings.cc
        libwebrtc/libwebrtc/rtc_base/experiments/alr_experiment.cc
        libwebrtc/libwebrtc/modules/remote_bitrate_estimator/overuse_detector.cc
        libwebrtc/libwebrtc/api/transport/field_trial_based_config.cc
        )

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
ADD_DEFINITIONS(-DMS_BIG_ENDIAN)
target_link_libraries(sender ${LUALIB})