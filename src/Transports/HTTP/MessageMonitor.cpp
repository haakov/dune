//***************************************************************************
// Copyright 2007-2020 Universidade do Porto - Faculdade de Engenharia      *
// Laboratório de Sistemas e Tecnologia Subaquática (LSTS)                  *
//***************************************************************************
// This file is part of DUNE: Unified Navigation Environment.               *
//                                                                          *
// Commercial Licence Usage                                                 *
// Licencees holding valid commercial DUNE licences may use this file in    *
// accordance with the commercial licence agreement provided with the       *
// Software or, alternatively, in accordance with the terms contained in a  *
// written agreement between you and Faculdade de Engenharia da             *
// Universidade do Porto. For licensing terms, conditions, and further      *
// information contact lsts@fe.up.pt.                                       *
//                                                                          *
// Modified European Union Public Licence - EUPL v.1.1 Usage                *
// Alternatively, this file may be used under the terms of the Modified     *
// EUPL, Version 1.1 only (the "Licence"), appearing in the file LICENCE.md *
// included in the packaging of this file. You may not use this work        *
// except in compliance with the Licence. Unless required by applicable     *
// law or agreed to in writing, software distributed under the Licence is   *
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF     *
// ANY KIND, either express or implied. See the Licence for the specific    *
// language governing permissions and limitations at                        *
// https://github.com/LSTS/dune/blob/master/LICENCE.md and                  *
// http://ec.europa.eu/idabc/eupl.html.                                     *
//***************************************************************************
// Author: Ricardo Martins                                                  *
//***************************************************************************

// DUNE headers.
#include <DUNE/DUNE.hpp>

// Local headers.
#include "MessageMonitor.hpp"

namespace fs = std::filesystem;

namespace Transports
{
  namespace HTTP
  {
    using DUNE_NAMESPACES;

    MessageMonitor::MessageMonitor(const std::string& system, uint64_t uid, DUNE::FileSystem::Path dir_www, DUNE::FileSystem::Path dir_log):
      m_uid(uid),
      m_last_msgs_json(0),
      m_last_logbook_json(0),
      m_log_entry(100),
      m_dir_www(dir_www),
      m_dir_log(dir_log)
    {
      // Initialize meta information.
      std::ostringstream os;
      os << "var data = {\n"
         << "  'dune_version': '" << getFullVersion() << " - " << getCompileDate() << "',\n"
         << "  'dune_uid': '" << m_uid << "',\n"
         << "  'dune_time_start': '" << std::setprecision(12) << Clock::getSinceEpoch() << "',\n"
         << "  'dune_system': '" << system << "',\n";
      m_meta = os.str();
      m_last_wrote = Clock::getMsec();
    }

    MessageMonitor::~MessageMonitor(void)
    {
      ScopedMutex l(m_mutex);

      {
        std::map<unsigned, IMC::Message*>::iterator itr = m_msgs.begin();
        for (; itr != m_msgs.end(); ++itr)
          delete itr->second;
      }

      {
        for (PowerChannelMap::iterator itr = m_power_channels.begin(); itr != m_power_channels.end(); ++itr)
          delete itr->second;
      }

      {
        for(unsigned int itr = 0; itr < m_logbook.size(); ++itr)
          delete m_logbook[itr];
      }
      m_msg_file.close();
      m_lbook_file.close();
    }

    void
    MessageMonitor::setEntities(const std::map<unsigned, std::string>& entities)
    {
      ScopedMutex l(m_mutex);
      m_entities = entities;
    }

    std::string
    MessageMonitor::messagesJSON(void)
    {
      ScopedMutex l(m_mutex);

      std::ostringstream os;
      os << m_meta
         << "  'dune_time_current': '" << std::setprecision(12) << Clock::getSinceEpoch() << "',\n";

      if (m_entities.empty())
      {
        os << "  'dune_entities': { },\n";
      }
      else
      {
        os << "  'dune_entities': {\n";
        EntityMap::iterator itr = m_entities.begin();
        os << itr->first << " : {" << "\"label\": \"" << itr->second << "\"}";
        ++itr;
        for (; itr != m_entities.end(); ++itr)
          os << ",\n" << itr->first << " : {" << "\"label\": \"" << itr->second << "\"}";
        os << "\n},";
      }

      os << "  'dune_messages': [\n";

      std::map<unsigned, IMC::Message*>::iterator itr = m_msgs.begin();
      itr->second->toJSON(os);
      ++itr;

      for (; itr != m_msgs.end(); ++itr)
      {
        os << ",\n";
        itr->second->toJSON(os);
      }

      for (PowerChannelMap::iterator pitr = m_power_channels.begin(); pitr != m_power_channels.end(); ++pitr)
      {
        os << ",\n";
        pitr->second->toJSON(os);
      }

      os << "\n]"
         << "\n};";

      return os.str();
    }

    void
    MessageMonitor::updateMessage(const IMC::Message* msg)
    {
      //ScopedMutex l(m_mutex);

      if (msg->getId() == DUNE_IMC_POWERCHANNELSTATE)
        updatePowerChannel(static_cast<const IMC::PowerChannelState*>(msg));

      IMC::Message* tmsg = msg->clone();
      unsigned key = tmsg->getId() << 24 | tmsg->getSubId() << 8 | tmsg->getSourceEntity();

      if (m_msgs[key])
        delete m_msgs[key];

      m_msgs[key] = tmsg;

      uint64_t now = Clock::getMsec();
      if (now - m_last_wrote  > 2000)
      {
        m_msg_file.open(m_dir_www.str() + "/state/messages.js", std::ios::out);
        m_msg_file << messagesJSON();
        m_msg_file.close();
        m_lbook_file.open(m_dir_www.str() + "/state/logbook.js", std::ios::out);
        m_lbook_file << logbookJSON();
        m_lbook_file.close();
        m_logs_file.open(m_dir_www.str() + "/state/logs.js", std::ios::out);
        m_logs_file << logsJSON();
        m_logs_file.close();

        m_last_wrote = now;
      }
    }

    std::string
    MessageMonitor::logsJSON(void)
    {
      ScopedMutex l(m_mutex);

      std::ostringstream os;

      os << "var logs = {\n"
         <<"'dune_logs': [\n";

      for (const auto & date : fs::directory_iterator(m_dir_log.str()))
      {
        if (fs::is_empty(date.path()))
        {
          std::cout << date.path() << " is empty!" << std::endl;
          continue;
        }
        os << "{\n" << "  'date': '" << date << "',\n";
        os << "  'times': \n  [\n";
        for (const auto & time : fs::directory_iterator(date.path()))
        {
          double size = 0.0;

          for (const auto & file : fs::directory_iterator(time.path()))
          {
            size += fs::file_size(file);
          }

          std::string postfix;
          if (size > 1000000000)
          {
            size = size / 1000000000;
            postfix = " GB";
          }
          else if (size > 1000000)
          {
            size = size / 1000000;
            postfix = " MB";
          }
          else if (size > 1000)
          {
            size = size / 1000;
            postfix = " KB";
          }
          else
          {
            postfix = " B";
          }
          os << "    {'time': '" << time << "', 'size': '" << std::setprecision(3) << size << postfix << "'},\n";
        }
        os << "  ]\n},\n";
      }

      os << "\n]"
         << "\n};";

      return os.str();
    }

    std::string
    MessageMonitor::logbookJSON(void)
    {
      ScopedMutex l(m_mutex);

      std::ostringstream os;
      unsigned int itr = 0;

      os << "var logbook = {\n"
         <<"'dune_logbook': [\n";
      m_logbook[itr]->toJSON(os);
      ++itr;

      for (; itr != m_logbook.size(); ++itr)
      {
        os << ",\n";
        m_logbook[itr]->toJSON(os);
      }

      os << "\n]"
         << "\n};";

      return os.str();
    }

    void
    MessageMonitor::addLogEntry(const IMC::LogBookEntry* msg)
    {
      ScopedMutex l(m_mutex);

      if (m_logbook.size() >= m_log_entry)
        m_logbook.erase(m_logbook.begin());

      m_logbook.push_back(new IMC::LogBookEntry(*msg));
    }

    void
    MessageMonitor::updatePowerChannel(const IMC::PowerChannelState* msg)
    {
      std::map<std::string, IMC::PowerChannelState*>::iterator itr = m_power_channels.find(msg->name);
      if (itr != m_power_channels.end())
        *itr->second = *msg;
      else
        m_power_channels[msg->name] = new IMC::PowerChannelState(*msg);
    }
  }
}
