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
// Author: Pedro Calado                                                     *
//***************************************************************************

#ifndef PLAN_ENGINE_PLAN_HPP_INCLUDED_
#define PLAN_ENGINE_PLAN_HPP_INCLUDED_

// C++ standard library headers.
#include <memory>
#include <string>
#include <vector>

// DUNE headers.
#include <DUNE/Plans.hpp>

// Local headers.
#include "ActionSchedule.hpp"
#include "Calibration.hpp"
#include "FuelPrediction.hpp"
#include "PlanGraph.hpp"
#include "Statistics.hpp"
#include "Timeline.hpp"

namespace Plan
{
  namespace Engine
  {
    // Export DLL Symbol.
    class DUNE_DLL_SYM PlanRuntime;

    //! Depth margin.
    static const float c_depth_margin = 1.0f;

    //! PlanRuntime arguments
    struct PlanArguments
    {
      //! Maximum allowed depth.
      float max_depth;
      //! Minimum calibration time
      std::uint16_t min_cal_time;
      //! Whether or not to compute plan's progress
      bool compute_progress;
      //! Whether or not to compute fuel prediction
      bool fpredict;
    };

    //! Plan runtime manager
    class PlanRuntime
    {
    public:
      //! Exception for errors during plan sequencing
      struct PlanSequenceError : public std::runtime_error
      {
        PlanSequenceError(const std::string& label)
        : std::runtime_error(DTR("sequence error: ") + label)
        {
        }
      };

      //! Exception for invalid plan specifications
      struct InvalidPlanSpec : public std::runtime_error
      {
        InvalidPlanSpec(const std::string& label)
        : std::runtime_error(DTR("invalid plan specification: ") + label)
        {
        }
      };

      //! Default constructor.
      //! @param[in] args plan runtime arguments
      //! @param[in] task pointer to task
      //! @param[in] cfg pointer to config object
      PlanRuntime(PlanArguments const& args, DUNE::Tasks::Task* task,
                  DUNE::Parsers::Config* cfg);

      //! Destructor
      ~PlanRuntime(void) {}

      //! Reset data
      void
      clear(void);

      //! Get the currently loaded plan's id.
      char const*
      getPlanId(void) const noexcept
      {
        if (!m_plan_graph)
          return "";

        return m_plan_graph->getId().c_str();
      }

      //! Load a PlanSpecification message
      //! Parses the plan and initializes the runtime information (ETA
      //! prediction, fuel prediction, ...)
      //! @param[in] spec plan specification
      //! @param[in] supported_maneuvers list of supported maneuvers
      //! @param[in] cinfo map of components info
      //! @param[in] imu_enabled true if imu enabled, false otherwise
      //! @param[in] state pointer to EstimatedState message
      //! @return ps PlanStatistics message
      DUNE::IMC::PlanStatistics
      load(const DUNE::IMC::PlanSpecification& spec,
           const std::set<std::uint16_t>& supported_maneuvers,
           const std::map<std::string, DUNE::IMC::EntityInfo>& cinfo,
           bool imu_enabled = false,
           const DUNE::IMC::EstimatedState* state = NULL);

      //! Signal that the plan has started
      void
      planStarted(void);

      //! Signal that the plan has stopped
      void
      planStopped(void);

      //! Signal that calibration has started
      void
      calibrationStarted(void);

      //! Signal that a maneuver has started
      //! @param[in] id name of the started maneuver
      void
      maneuverStarted(const std::string& id);

      //! Signal that current maneuver is done
      void
      maneuverDone(void);

      //! Get necessary calibration time
      //! @return necessary calibration time
      std::uint16_t
      getEstimatedCalibrationTime(void) const;

      //! Check if plan has been completed
      //! @return true if plan is done
      bool
      isDone(void) const;

      //! Get start maneuver message
      //! @return NULL if start maneuver id is invalid
      DUNE::IMC::PlanManeuver const*
      loadStartManeuver(void);

      //! Get next maneuver message
      //! @return NULL if maneuver id is invalid
      DUNE::IMC::PlanManeuver const*
      loadNextManeuver(void);

      //! Get current maneuver message
      DUNE::IMC::PlanManeuver const*
      getCurrentManeuver(void) const
      {
        if (!m_curr_node)
          return nullptr;

        return m_curr_node->pman;
      }

      //! Get current maneuver id
      //! @return current id string
      inline std::string
      getCurrentId(void) const
      {
        return m_last_id;
      }

      //! Get calibration info string
      //! @return calibration info string
      inline const std::string
      getCalibrationInfo(void) const
      {
        return m_calib.getInfo();
      }

      //! Is calibration done
      //! @return true if so, false otherwise
      inline bool
      isCalibrationDone(void) const
      {
        return m_calib.isDone();
      }

      //! Has calibration failed
      //! @return true if so, false otherwise
      inline bool
      hasCalibrationFailed(void) const
      {
        return m_calib.hasFailed();
      }

      //! Get current plan progress
      //! @param[in] mcs pointer to maneuver control state message
      //! @return progress in percent (-1.0 if unable to compute)
      float
      updateProgress(const DUNE::IMC::ManeuverControlState* mcs);

      //! Update calibration process
      void
      updateCalibration(const DUNE::IMC::VehicleState* vs);

      //! Pass EntityActivationState to scheduler
      //! @param[in] id entity label
      //! @param[in] msg pointer to EntityActivationState message
      //! @return false if something failed to be activated, true otherwise
      bool
      onEntityActivationState(const std::string& id,
                              const DUNE::IMC::EntityActivationState* msg);

      //! Pass FuelLevel to FuelPrediction
      //! @param[in] msg FuelLevel message
      void
      onFuelLevel(const DUNE::IMC::FuelLevel* msg);

      //! Get current estimated time of arrival
      //! @return ETA
      float
      getETA(void) const;

    private:
      //! Check if depth is within limits.
      //! @param[in] maneuver plan maneuver.
      //! @return true if depth is safe, false otherwise.
      bool
      isDepthSafe(const DUNE::IMC::Message* maneuver) const;

      //! Get duration of the execution phase of the plan
      //! (total of maneuver accumulated duration)
      //! @return duration of the execution phase of the plan
      float
      getExecutionDuration(void) const;

      //! Get total duration of the plan
      //! @return total duration of the plan
      inline float
      getTotalDuration(void) const
      {
        return getExecutionDuration() + getEstimatedCalibrationTime();
      }

      //! Get execution percentage
      //! @return percentage of the plan represented by the execution
      inline float
      getExecutionPercentage(void) const
      {
        return getExecutionDuration() / getTotalDuration() * 100.0;
      }

      //! Check if scheduler is waiting for a device
      //! @return true if waiting for device
      bool
      waitingForDevice(void);

      //! Returns calibration time left according to scheduler
      //! @return calibration time left or -1 if no scheduler is active
      float
      scheduledTimeLeft(void) const;

      //! Initialize action scheduling, compute statistics, and other plan
      //! runtime initialization which must be done after a new plan is loaded.
      //! @param[in] cinfo map of components info
      //! @param[in] imu_enabled true if imu enabled, false otherwise
      //! @param[in] state pointer to EstimatedState message
      //! @return PlanStatistics message
      DUNE::IMC::PlanStatistics
      initializeRuntime(
      const std::map<std::string, DUNE::IMC::EntityInfo>& cinfo,
      bool imu_enabled, const DUNE::IMC::EstimatedState* state);

      //! Get maneuver from id
      //! @param[in] id name of the maneuver to load
      //! @return NULL if maneuver id is invalid
      DUNE::IMC::PlanManeuver const*
      loadManeuverFromId(std::string const& id);

      //! Compute current progress
      //! @param[in] pointer to ManeuverControlState message
      //! @return progress in percent (-1.0 if unable to compute)
      float
      progress(const DUNE::IMC::ManeuverControlState* mcs);

      //! Test if plan is linear
      inline bool
      isLinear(void) const
      {
        return !(m_properties & DUNE::IMC::PlanStatistics::PRP_NONLINEAR);
      }

      //! Check if depth is safe
      inline bool
      checkDepth(DUNE::IMC::ZUnits zunits, float z) const
      {
        if (zunits == DUNE::IMC::Z_DEPTH)
        {
          if (z > m_args.max_depth + c_depth_margin)
            return false;
        }

        return true;
      }

      //! Parsed plan.
      std::unique_ptr<PlanGraph> m_plan_graph;
      //! Arguments
      PlanArguments m_args;
      //! Pointer to current node
      PlanGraph::Node const* m_curr_node;
      //! Last maneuver id
      std::string m_last_id;
      //! Current progress if any
      float m_progress;
      //! Estimated required calibration time
      std::uint16_t m_est_cal_time;
      //! Pointer to maneuver durations
      std::unique_ptr<DUNE::Plans::TimeProfile> m_profiles;
      //! Flag to signal that the plan is past the last maneuver with a valid
      //! duration
      bool m_beyond_dur;
      //! Schedule for actions to take during plan
      std::unique_ptr<ActionSchedule> m_sched;
      //! Vector of entity labels to push and pop entity parameters
      std::vector<std::string> m_affected_ents;
      //! Signal that a maneuver has started
      bool m_started_maneuver;
      //! Calibration object pointer
      Calibration m_calib;
      //! Component active time for fuel estimation
      ComponentActiveTime m_cat;
      //! Pointer to speed model for speed conversions
      std::unique_ptr<const DUNE::Plans::SpeedModel> m_speed_model;
      //! Pointer to power model for power conversions and estimations
      std::unique_ptr<const DUNE::Power::Model> m_power_model;
      //! Pointer to Fuel Prediction object
      std::unique_ptr<FuelPrediction> m_fpred;
      //! Pointer to task
      DUNE::Tasks::Task* m_task;
      //! Plan properties
      unsigned m_properties;
      //! Run Time Statistics
      RunTimeStatistics m_rt_stat;
    };
  } // namespace Engine
} // namespace Plan

#endif