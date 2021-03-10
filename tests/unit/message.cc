#include "message.hpp"

#include "UnitTest.hpp"

#include <iostream>

TEST_MODULE(MessageTestModule)

using unique_message_ptr = std::unique_ptr<mrobot::messaging::Message>;

enum TestMessageType {
    TestMessageTypeFirst,
    TestMessageTypeSecond,
    TestMessageTypeThird,
    TestMessageTypeMax
};

#define INVALID_VALUE 99

template<typename T>
class TestMessage : public mrobot::messaging::Message {
    public:
        TestMessage() = default;
        virtual ~TestMessage() = default;

        bool deserialize(const unsigned char *payload, ::size_t size) override {
            if (size != sizeof(m_data)) {
                return false;
            }

           m_data = *reinterpret_cast<const T *>(payload);

            return true;
        }

        T get() const { return m_data; }

    private:
        T m_data = INVALID_VALUE;
};

typedef TestMessage<uint8_t> MessageFirst;
typedef TestMessage<uint16_t> MessageSecond;
typedef TestMessage<uint32_t> MessageThird;

template<typename T, class M>
class TestMessageHandler : public mrobot::messaging::Handler {
    public:
        TestMessageHandler() = default;

        virtual ~TestMessageHandler() = default;

        void handleMessage(const unique_message_ptr &message) override {
            auto *privMessage = dynamic_cast<const M *>(message.get());
            m_data = privMessage->get();
        }

        T get() const { return m_data; }
        void reset() { m_data = INVALID_VALUE; }

    private:
        T m_data = INVALID_VALUE;
};

typedef TestMessageHandler<uint8_t, MessageFirst> MessageFirstHandler;
typedef TestMessageHandler<uint16_t, MessageSecond> MessageSecondHandler;
typedef TestMessageHandler<uint32_t, MessageThird> MessageThirdHandler;

class TestMessagesFactory : public mrobot::messaging::Factory {
    public:
        TestMessagesFactory() = default;
        ~TestMessagesFactory() = default;

        unique_message_ptr create(uint32_t messageId,
                                  const unsigned char *payload,
                                  ::size_t size) const override
        {
            unique_message_ptr pMessage{nullptr};

            switch (messageId) {
                case TestMessageTypeFirst:
                    pMessage = std::make_unique<MessageFirst>();
                    break;
                case TestMessageTypeSecond:
                    pMessage = std::make_unique<MessageSecond>();
                    break;
                case TestMessageTypeThird:
                    pMessage = std::make_unique<MessageThird>();
                    break;
                default:
                    break;
            }

            return pMessage->deserialize(payload, size) ?
                   std::move(pMessage) : unique_message_ptr{nullptr};
        }

};

class TestRouter final : public mrobot::messaging::Router {
    public:
        TestRouter() : Router(TestMessageTypeMax, &m_testFactory) {
            registerHandler(TestMessageTypeFirst, &m_firstHandler);
            registerHandler(TestMessageTypeSecond, &m_secondHandler);
            registerHandler(TestMessageTypeThird, &m_thirdHandler);
        }

        virtual ~TestRouter() = default;

        uint8_t getFirst() { return m_firstHandler.get(); }
        uint16_t getSecond() { return m_secondHandler.get(); }
        uint32_t getThird() { return m_thirdHandler.get(); }

        void resetFirst() { m_firstHandler.reset(); }
        void resetSecond() { m_secondHandler.reset(); }
        void resetThird() { m_thirdHandler.reset(); }

    private:
        TestMessagesFactory m_testFactory;
        MessageFirstHandler m_firstHandler;
        MessageSecondHandler m_secondHandler;
        MessageThirdHandler m_thirdHandler;
};


UNIT_TEST(MessageTestModule, BasicTest) {
    TestRouter router;

    const uint8_t firstData = 80;
    const uint16_t secondData = 80;

    router.route(TestMessageTypeFirst, reinterpret_cast<const unsigned char *>(&firstData), sizeof(firstData));
    EXPECT_EQUAL(router.getFirst(), firstData);

    router.resetFirst();
    router.route(TestMessageTypeFirst, reinterpret_cast<const unsigned char *>(&secondData), sizeof(secondData));
    EXPECT_EQUAL(router.getFirst(), INVALID_VALUE);

}

