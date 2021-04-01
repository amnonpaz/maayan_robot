#ifndef ROBOT_MESSAGES_H_
#define ROBOT_MESSAGES_H_

#include "mqtt_client.hpp"

namespace mrobot {

enum MessageType {
    MessageTypeLightSet,
    MessageTypeMax
};

class MessageLightSet : public messaging::IncomingMessage {
    public:
        MessageLightSet() = default;
        virtual ~MessageLightSet() = default;

        bool deserialize(const unsigned char *payload, ::size_t size) override;

        uint32_t getId() const noexcept { return m_data.id; }
        uint32_t getState() const noexcept { return m_data.state; }

    private:
        struct {
            uint32_t id;
            uint32_t state;
        } m_data __attribute__((packed));
};

class RobotImcomingMessagesRouter final : public comm::MqttIncomingRouter {
    public:
        explicit RobotImcomingMessagesRouter(std::string topicPrefix = "") :
            comm::MqttIncomingRouter(MessageTypeMax, std::move(topicPrefix)) {}
        virtual ~RobotImcomingMessagesRouter() = default;

    protected:
        std::unique_ptr<messaging::IncomingMessage> factory(uint32_t messageId) const override;
        uint32_t topicToMessageId(const std::string &topic) const override;
        const std::string &messageIdToTopic(uint32_t messageId) const override;
};

} // namespace mrobot

#endif // ROBOT_MESSAGES_H_