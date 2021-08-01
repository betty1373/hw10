#ifndef Server_H
#define Server_H
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include "Logger.h"

using boost::asio::ip::tcp;
class Session
  : public std::enable_shared_from_this<Session>
{
public:
  Session(tcp::socket socket,std::shared_ptr<CmdReader>& reader);
  
  void Start();
private:
  void Do_read();
  
  tcp::socket m_socket;
  std::shared_ptr<CmdReader> m_cmdReader;
  enum { max_length = 1024 };
  char data_[max_length];
}

class Server
{
public:
  Server(boost::asio::io_context& io_context, short port, std::size_t numCmds);
 
private:
  void Do_accept();

  tcp::acceptor m_acceptor;

  std::shared_ptr<CmdReader> m_cmdReader;
  std::shared_ptr<ConsoleLogger> m_consoleLogger;
  std::shared_ptr<FileLogger> m_fileLogger;
}

#endif