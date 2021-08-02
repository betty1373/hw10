#include "../inc/CmdReader.h"
#include <tuple>
std::shared_ptr<CmdReader> CmdReader::Create(size_t num_cmds,std::istream& istream) {
    auto ptr = std::shared_ptr<CmdReader>{ new CmdReader{num_cmds,istream}};
    return ptr;
}
    
void CmdReader::Subscribe(const std::shared_ptr<Observer>& obs) {
     m_observers.emplace_back(obs);
}
     
void CmdReader::Notify(std::vector<std::string>& cmds) {
    if (cmds.empty()) return;   
        
    for (auto it=m_observers.begin();it!=m_observers.end();++it) {
        auto ptr = it->lock();
        if (ptr) {
            std::stringstream ss = FormBatch(cmds);
            ptr->Update(ss);
        }
        else
            m_observers.erase(it);
    }
 //   cmds.resize(0);
}
    
void CmdReader::NewCmd(const std::string& clientId, const std::string& cmd)
{ 
    if (cmd.empty()) return;
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        auto context = GetContext(clientId);
        
        if (cmd=="{") {
            context = AddContext(clientId);
            context->second.first++;
        }
        else if (cmd=="}") {
            if (context->second.first>0) {
                context->second.first--;
            }
            else {
                context->second.second.resize(0);
            }
        }
        else {
            context->second.second.emplace_back(cmd);
        }
    }
}
void CmdReader::CmdLog(bool to_log) {
  std::unique_lock<std::mutex> locker(m_mutex);

  auto it = m_contexts.begin();
  while (it != m_contexts.end() ) {
    auto& context = it->second; 
    if (!context.first && (to_log || context.second.size() >= m_num_cmds) ) {
      if (context.second.size() > 0) {
        Notify(context.second);    
        context.second.resize(0);       
      }
      it = m_contexts.erase(it);
    }
    else {
      ++it;
    }
  }
}
void CmdReader::AddClient(const std::string& client) {
    m_clients.emplace(client);
}
void CmdReader::DeleteClient(const std::string& client){
    m_clients.erase(client);
    if (m_clients.size()==0) {
        CmdLog(true);
    }
}   
CmdReader::CmdReader(size_t num_cmds,std::istream& istream) 
    : m_num_cmds{num_cmds},
        m_istream(istream)
{
    //m_cmds.reserve(m_num_cmds);
}

std::stringstream CmdReader::FormBatch(std::vector<std::string>& cmds) {
    std::stringstream ss;
    for (auto it_cmd = cmds.cbegin();it_cmd!=cmds.cend();it_cmd++) {
        if (it_cmd !=cmds.cbegin())
            ss<< ", "<<*it_cmd;
        else
            ss<<"bulk: "<<*it_cmd;
    }
    return ss;     
}
ContextIt CmdReader::GetContext(const std::string& clientId) {
    auto it = m_contexts.find(clientId);
    if (it==m_contexts.end()) {
        it = AddContext("main");
    }
    return it;
}
ContextIt CmdReader::AddContext(const std::string& clientId){
    auto it = m_contexts.find(clientId);
    if (it==m_contexts.end()) {
         auto [insIt, inserted] = m_contexts.try_emplace(clientId);
         if (inserted) {
             it = insIt;
         }
    }
    return it;
}