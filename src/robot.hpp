#ifndef _ROBOT_H_
#define _ROBOT_H_

#include "led.hpp"
#include "conf.hpp"
#include "mqtt_client.hpp"
#include "robot_messages_router.hpp"

#include <string>
#include <array>
#include <memory>

namespace mrobot {
    
class Robot final {
    public:
        enum Light {
            LightEyeRight,
            LightEyeLeft,
            LightAntenaRight,
            LightAntenaLeft,
            LightMax
        };

        explicit Robot(std::string confFile) :
            m_confFile(std::move(confFile)),
            m_running(false) {}
        ~Robot() = default;

        bool initialize();
        void finalize();
        bool run() const;
        void stop();

        bool setLight(Light light, bool on);

    private:
        bool initLights();
        bool communiationOpen();
        void communiationClose();

        static const char *to_string(Light light);

        const std::string m_confFile;
        conf::Database m_configuration;
        std::array<std::unique_ptr<hw::Led>, LightMax> m_lights;
        bool m_running;

        std::unique_ptr<comm::MqttClient> m_comm;
        const RobotMessagesRouter m_messagesRouter;

        static constexpr uint32_t s_mainLoopSleepTime_ms = 50;
};

} // namespace mrobot 

#endif // _ROBOT_H_
