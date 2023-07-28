/**
 * MIT License
 *
 * Copyright 2023 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <signal.h>
#include <easylogging++.h>
#include <chrono>
#include <atomic>
#include <ConfigureLogger.hpp>
#include <ConfigurationJsonFile.hpp>
#include <Exceptions/ConfigurationException.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#undef max
#include <mqtt/async_client.h>
#include <MQTTAsync.h>

std::string SERVER_ADDRESS("tcp://localhost:1883");
std::string CLIENT_ID("paho_cpp_async_subcribe");
std::string TOPIC("umati/v2/#");

const int QOS = 1;
const int N_RETRY_ATTEMPTS = 5;

struct A_member {
  typedef MQTTAsync mqtt::async_client::*type;
  friend type get(A_member);
};

template <typename Tag, typename Tag::type M>
struct Rob {
  friend typename Tag::type get(Tag) { return M; }
};

template struct Rob<A_member, &mqtt::async_client::cli_>;

void traceCallback(enum MQTTASYNC_TRACE_LEVELS level, char* message) { std::cout << "TRACE(" << level << "): " << message << '\n'; }

class action_listener : public virtual mqtt::iaction_listener {
  std::string name_;

  void on_failure(const mqtt::token& tok) override {
    std::cout << name_ << " failure";
    if (tok.get_message_id() != 0) std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    std::cout << std::endl;
  }

  void on_success(const mqtt::token& tok) override {
    std::cout << name_ << " success";
    if (tok.get_message_id() != 0) std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    auto top = tok.get_topics();
    if (top && !top->empty()) std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
    std::cout << std::endl;
  }

 public:
  action_listener(const std::string& name) : name_(name) {}
};

class callback : public virtual mqtt::callback,
                 public virtual mqtt::iaction_listener

{
  int nretry_;
  mqtt::async_client& cli_;
  mqtt::connect_options& connOpts_;
  action_listener subListener_;

  void reconnect() {
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    try {
      cli_.connect(connOpts_, nullptr, *this);
    } catch (const mqtt::exception& exc) {
      std::cerr << "Error-Error-Code: " << exc.what() << std::endl;
      std::cerr << "Error-Error-String: " << exc.get_error_str() << std::endl;
      std::cerr << "Error-Reason-Code: " << exc.get_reason_code() << std::endl;
      std::cerr << "Error-Reason-String: " << exc.get_reason_code_str() << std::endl;
      std::cerr << "Error-String: " << exc.to_string() << std::endl;
      exit(1);
    }
  }

  void on_failure(const mqtt::token& tok) override {
    std::cout << "Connection attempt failed" << std::endl;
    std::cout << "Error-Reason-Code: " << tok.get_reason_code() << std::endl;
    std::cout << "Error-Reason-String: " << mqtt::exception::reason_code_str(tok.get_reason_code()) << std::endl;
    std::cout << "Error-Return-Code: " << tok.get_return_code() << std::endl;
    std::cout << "Error-Return-String: " << mqtt::exception::error_str(tok.get_return_code()) << std::endl;
    if (++nretry_ > N_RETRY_ATTEMPTS) exit(1);
    reconnect();
  }

  void on_success(const mqtt::token& tok) override {}

  void connected(const std::string& cause) override {
    std::cout << "\nConnection success" << std::endl;
    std::cout << "\nSubscribing to topic '" << TOPIC << "'\n"
              << "\tfor client " << CLIENT_ID << " using QoS" << QOS << "\n"
              << "\nPress Q<Enter> to quit\n"
              << std::endl;

    cli_.subscribe(TOPIC, QOS, nullptr, subListener_);
  }

  void connection_lost(const std::string& cause) override {
    std::cout << "\nConnection lost" << std::endl;
    if (!cause.empty()) std::cout << "\tcause: " << cause << std::endl;

    std::cout << "Reconnecting..." << std::endl;
    nretry_ = 0;
    reconnect();
  }

  void message_arrived(mqtt::const_message_ptr msg) override {
    std::cout << "Message arrived" << std::endl;
    std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
    // std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
  }

  void delivery_complete(mqtt::delivery_token_ptr token) override {}

 public:
  callback(mqtt::async_client& cli, mqtt::connect_options& connOpts) : nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

std::string getUri(std::string protocol, std::string host, std::uint16_t port) {
  std::stringstream ss;
  ss << protocol << "://" << host << ":" << port << "/ws";
  return ss.str();
}

mqtt::connect_options getOptions(const std::string& username, const std::string& password, const std::string caCertDirPath, const std::string caCertFilePath) {
  mqtt::connect_options opts_conn;
  opts_conn.set_keep_alive_interval(std::chrono::seconds(10));
  opts_conn.set_clean_session(true);

  opts_conn.set_automatic_reconnect(2, 10);

  if (!username.empty()) {
    opts_conn.set_user_name(username);
  }

  if (!password.empty()) {
    opts_conn.set_password(password);
  }

  mqtt::ssl_options ssl_opts;
#ifndef WIN32
  ssl_opts.ca_path("/etc/ssl/certs/");
#else
  ssl_opts.set_ca_path(caCertDirPath.c_str());
  ssl_opts.set_trust_store(caCertFilePath.c_str());
#endif
  ssl_opts.set_verify(true);
  opts_conn.set_ssl(ssl_opts);

  return opts_conn;
}

int main(int argc, char* argv[]) {
  std::string configFilename("configuration.json");

  if (argc >= 2) {
    configFilename = argv[1];
  }

  std::shared_ptr<Umati::Util::Configuration> config;
  try {
    config = std::make_shared<Umati::Util::ConfigurationJsonFile>(configFilename);
    auto mqttConfig = config->getMqtt();
    MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_LEVELS::MQTTASYNC_TRACE_MAXIMUM);
    MQTTAsync_setTraceCallback(traceCallback);
    mqtt::async_client cli(getUri(mqttConfig.Protocol, mqttConfig.Hostname, mqttConfig.Port), mqttConfig.ClientId);
    auto mqttasync = &(cli.*get(A_member()));
    mqtt::connect_options connOpts = getOptions(mqttConfig.Username, mqttConfig.Password, mqttConfig.CaCertPath, mqttConfig.CaTrustStorePath);
    connOpts.set_clean_session(false);

    // Install the callback(s) before connecting.
    callback cb(cli, connOpts);
    cli.set_callback(cb);

    // Start the connection.
    // When completed, the callback will subscribe to topic.

    try {
      std::cout << "Connecting to the MQTT server " << getUri(mqttConfig.Protocol, mqttConfig.Hostname, mqttConfig.Port) << " ... " << std::flush;
      cli.connect(connOpts, nullptr, cb);
    } catch (const mqtt::exception& exc) {
      std::cerr << "\nERROR: Unable to connect to MQTT server: '" << SERVER_ADDRESS << "'" << exc << std::endl;
      return 1;
    }

    // Just block till user tells us to quit.

    while (std::tolower(std::cin.get()) != 'q')
      ;

    // Disconnect

    try {
      std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
      cli.disconnect()->wait();
      std::cout << "OK" << std::endl;
    } catch (const mqtt::exception& exc) {
      std::cerr << exc << std::endl;
      return 1;
    }
  } catch (std::exception& ex) {
    std::cout << "Usage <>.exe [ConfigurationFileName]" << ex.what();
    return 1;
  }
  return 0;
}
