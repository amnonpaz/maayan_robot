#ifndef MOSQUITTO_CLIENT_H_
#define MOSQUITTO_CLIENT_H_

#include "message.hpp"

#include <mosquitto.h>

#include <mutex>
#include <condition_variable>
#include <list>

namespace mrobot {

namespace comm {

class MqttSession {
    public:
        MqttSession() {
            ::mosquitto_lib_init();

            int major, minor, revision;
            ::mosquitto_lib_version(&major, &minor, &revision);
            m_version = std::to_string(major) + "." + std::to_string(minor) +
                        std::to_string(revision);
        }

        ~MqttSession() {
            ::mosquitto_lib_cleanup();
        }

        const std::string &version() { return m_version; }

        static std::shared_ptr<MqttSession> getSession() {
            static std::shared_ptr<MqttSession> session{nullptr};
            if (!session) {
                session = std::make_shared<MqttSession>();
            }

            return session;
        }

    private:
        std::string m_version;
};

class MqttIncomingRouter : public messaging::IncomingRouter {
    public:
        MqttIncomingRouter(uint32_t maxMessagesTypes, std::string topicPrefix = "") :
            IncomingRouter(maxMessagesTypes),
            m_topicPrefix(topicPrefix) {}
        virtual ~MqttIncomingRouter() = default;

        inline bool route(const std::string &topic, const unsigned char *payload, ::size_t size) const {
            std::string temp{topic, m_topicPrefix.length()};
            return IncomingRouter::route(topicToMessageId(temp), payload, size);
        }

        inline std::string getTopic(uint32_t messageId) const {
            return m_topicPrefix + messageIdToTopic(messageId);
        }

    protected:
        virtual uint32_t topicToMessageId(const std::string &topic) const = 0;
        virtual const std::string &messageIdToTopic(uint32_t messageId) const = 0;

    private:
        const std::string m_topicPrefix;
};


class MqttClient;

class MqttSender : public messaging::Sender {
    public:
        explicit MqttSender(const MqttClient *owner, std::string topicPrefix = "") :
            m_owner(owner),
            m_topicPrefix(topicPrefix) {}
        virtual ~MqttSender() = default;

        bool send(uint32_t messageId, messaging::OutgoingMessage *pMessage) override;

        inline std::string getTopic(uint32_t messageId) const {
            return m_topicPrefix + messageIdToTopic(messageId);
        }

    protected:
        virtual const std::string &messageIdToTopic(uint32_t messageId) const = 0;

        const MqttClient *m_owner;
        const std::string m_topicPrefix;
};

class MqttClient final {
    public:
        MqttClient(std::string clientName,
                   std::string brokerAddress,
                   uint16_t brokerPort,
                   const MqttIncomingRouter *router) :
            m_clientName(std::move(clientName)),
            m_brokerAddress(std::move(brokerAddress)),
            m_brokerPort(brokerPort),
            m_firstConnectionAttempt(true),
            m_session(nullptr),
            m_incomingRouter(router) {}

        bool initialize();
        void finalize();

        bool connect();
        bool disconnect();

        bool send(const char *topic, const unsigned char *payload, ::size_t size) const;
        void receive(const char *topic,
                     const unsigned char *payload,
                     ::size_t size);

        bool subscribe();

    private:
        const std::string m_clientName;
        const std::string m_brokerAddress;
        const uint16_t m_brokerPort;
        bool m_firstConnectionAttempt;
        std::shared_ptr<MqttSession> m_session;
        ::mosquitto *m_mosq;

        const MqttIncomingRouter *m_incomingRouter;

        static constexpr uint32_t s_keepAlive_sec = 30;
        static constexpr int s_defaultQOS = 1;
};

} // namespace comm

} // namespace mrobot

#endif // MOSQUITTO_CLIENT_H_
