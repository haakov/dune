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
// Author: Håkon Vågsether                                                  *
//***************************************************************************

// DUNE headers.
#include <DUNE/DUNE.hpp>

namespace Monitors
{
  //! Insert short task description here.
  //!
  //! Insert explanation on task behaviour here.
  //! @author Håkon Vågsether
  namespace Activatable
  {
    using DUNE_NAMESPACES;

    struct Arguments
    {
      int limit;
    };
    struct Task: public DUNE::Tasks::Task
    {
      

      //! Announce message counter
      int m_counter;
      //! Task arguments
      Arguments m_args;

      //! Constructor.
      //! @param[in] name task name.
      //! @param[in] ctx context.
      Task(const std::string& name, Tasks::Context& ctx):
        DUNE::Tasks::Task(name, ctx)
      {
        param("Announces to print", m_args.limit)
        .defaultValue("")
        .description("Announce message limit");
        
        m_counter = 0;
        bind<IMC::Announce>(this);
        paramActive(Tasks::Parameter::SCOPE_GLOBAL,
                Tasks::Parameter::VISIBILITY_USER, true);
      }

      void
      onActivation(void) {
          setEntityState(IMC::EntityState::ESTA_NORMAL, Status::CODE_ACTIVE);
      }

      void
      onDeactivation(void) {
          setEntityState(IMC::EntityState::ESTA_NORMAL, Status::CODE_IDLE);
          spew("------- Deactivating Activatable! -------");
      }

      void
      consume(const IMC::Announce* msg) {
        m_counter++;
      }

      //! Update internal state with new parameter values.
      void
      onUpdateParameters(void)
      {
      }

      //! Reserve entity identifiers.
      void
      onEntityReservation(void)
      {
      }

      //! Resolve entity names.
      void
      onEntityResolution(void)
      {
      }

      //! Acquire resources.
      void
      onResourceAcquisition(void)
      {
      }

      //! Initialize resources.
      void
      onResourceInitialization(void)
      {
      }

      //! Release resources.
      void
      onResourceRelease(void)
      {
      }

      //! Main loop.
      void
      onMain(void)
      {
        while (!stopping())
        {
          waitForMessages(1.0);
          if (isActive()) {
            if (m_counter < m_args.limit)
            {
              war("Announce messages: %d", m_counter);
            } else {
              requestDeactivation();
            }
          }
        }
      }
    };
  }
}

DUNE_TASK
