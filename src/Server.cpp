#include "Server.h"
Session::Session(tcp::socket socket, std::shared_ptr<CmdReader>& reader)
    : socket_(std::move(socket))
  {
  }

  void Session::Start()
  {
    do_read();
  }

void Session::do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            std::cout << "receive " << length << "=" << std::string{data_, length} << std::endl;
           
          }
        });
  }

Server::Server(boost::asio::io_context& io_context, short port, std::size_t numCmds)
    : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
  {
    m_cmdReader =  CmdReader::Create(static_cast<size_t>(numCmds));
    m_consoleLogger = ConsoleLogger::Create("log",m_cmdReader);
    m_fileLogger = FileLogger::Create("file",m_cmdReader); 
    Do_accept();
  }

void Sever::Do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<Session>(std::move(socket))->Start();
          }

          Do_accept();
        });
  }