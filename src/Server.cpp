#include "Server.h"
Session::Session(tcp::socket socket, std::shared_ptr<CmdReader>& reader)
    : m_socket(std::move(socket)),
      m_cmdReader(reader)
{
    auto endpoint = m_socket.remote_endpoint();
    m_clientId = endpoint.address().to_string() + ":"+ std::to_string(endpoint.port());     
}

void Session::Start()
{ 
    m_cmdReader->AddClient(m_clientId);
    Do_read();
}

void Session::Do_read()
  {
    auto self(shared_from_this());
    m_socket.async_read_some(boost::asio::buffer(m_data, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          m_strstream.write(m_data,length);
          if (ec == boost::asio:error::eof || ec==boost::asio::error::connection_reset) {
            std::cout << "receive " << length << "=" << std::string{data_, length} << std::endl;
            Close();
          }
          else {
            

          }
        });
  }
void Session:CloseSession() {
  m_cmdReader->DeleteClient(m_ClientId);
}
Server::Server(boost::asio::io_context& io_context, short port, std::size_t numCmds)
    : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
    m_socket(io_context)
  {
    m_cmdReader =  CmdReader::Create(static_cast<size_t>(numCmds));
    m_consoleLogger = ConsoleLogger::Create("log",m_cmdReader);
    m_fileLogger = FileLogger::Create("file",m_cmdReader); 
    Do_accept();
  }

void Server::Do_accept()
{
    m_acceptor.async_accept(m_socket,
        [this](boost::system::error_code ec)
        {
          if (!ec)
          {
            std::make_shared<Session>(std::move(m_socket), m_cmdReader)->Start();
          }

          Do_accept();
        });
}