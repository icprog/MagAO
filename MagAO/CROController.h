/************************************************************
 *    CROController.h
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * The declarations for the CRO stage controller
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file CROController.h
 * \author Jared R. Males
 * \brief Declarations for the CRO stage controller
 * 
 *
 */
//- moves done in doFSM.
//- positions -, and XYZ averages.

#ifndef __CROController_h__
#define __CROController_h__

#include <string>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <list>

#include "AOApp.h"
#include "AOStates.h"

#include "CROlib/dcn.h"

#define DEBUG 1

namespace MagAO
{

struct pending_move
{
   int type;
   int dpos;
};

class CROController : public AOApp
{
   public:
      CROController(int argc, char **argv) throw(AOException);
      ~CROController();

   protected:
      std::string dcn_addr; ///< Ip address or the file system name of the tty (serial device/com port)
      int dcn_port; ///<The port of the controller, 0 if using tty
            
      struct DCN_HEADER *head;  ///< The main header structure.  If 0 then hardware not initialized.
      
      ///The number of motors under control
      int n_motors;
      
      ///Vector of positions, relative to startup
      int *position;
      
      ///Array of motor modes.
      int *mode;
      
      ///Array of motor velocities
      int *vel;
      
      ///Array motor accelerations
      int *acc;
      
      ///The maximum number of steps to move one motor in the X or Z direction before moving the other.
      int swap_steps;

      int Aborted;
      
      std::list<pending_move> moveList;

      RTDBvar A1_pos_cur;
      RTDBvar A1_dpos_req;
      RTDBvar A1_stat_cur;
      RTDBvar B1_pos_cur;
      RTDBvar B1_dpos_req;
      RTDBvar B1_stat_cur;
      RTDBvar C1_pos_cur;
      RTDBvar C1_dpos_req;
      RTDBvar C1_stat_cur;
      RTDBvar A2_pos_cur;
      RTDBvar A2_dpos_req;
      RTDBvar A2_stat_cur;
      RTDBvar B2_pos_cur;
      RTDBvar B2_dpos_req;
      RTDBvar B2_stat_cur;

      RTDBvar X_pos_cur;
      RTDBvar X_dpos_req;

      RTDBvar Y_pos_cur;
      RTDBvar Y_dpos_req;

      RTDBvar Z_pos_cur;
      RTDBvar Z_dpos_req;

      RTDBvar abort_req;

      RTDBvar pending_moves;
      RTDBvar var_stepmode_req;
      RTDBvar var_stepmode_cur;
   protected:
      ///Common initialization.  tty_name should be set first.
      void Create();

      ///Load the configuration details from the file
      int LoadConfig();

      ///Setup variables in RTDB (overridden virtual)
      void SetupVars();
      
      int init_controllers();
            
   public:
      ///Set the step mode of the motor.
      /** \param mot is the motor "A1", "B1", etc.
        * \param mode is 1, 2, 4, or 8 - the stepsize for the motor.
        */
      int set_motor_mode(const char *mot, int mode);

      ///Set the velocty of the motor.int CROController::DoFSM()
      /** \param mot is the motor "A1", "B1", etc.
       * \param vel is the velocity, 1 <= vel <= 250
       */
      int set_motor_vel(const char *mot, int vel);

      ///Set the acceleration of the motor.
      /** \param mot is the motor "A1", "B1", etc.
       * \param acc is the acceleration, 1 <= acc <= 250
       */
      int set_motor_acc(const char *mot, int acc);

      /*Status*/
      ///Retrieves the specified motor status from the controller.
      /** \param addr is the controller address of the motor
        * \param unit is the unit number of the motorint CROController::DoFSM()
        * \retval 0 on success
        * \retval -1 on error
        */ 
      int check_motor_status(int addr, int unit);
      
      ///Retrieves the specified motor status from the controller.
      /** \param mot is the name of the motor
        * \retval 0 on success
        * \retval -1 on error
        */ 
      int check_motor_status(const char *mot);
            
      ///Get the specified motor status.  Does not call \ref check_motor_status so the return value may be out of date.
      /** \param addr is the controller address of the motor
        * \param unit is the unit number of the motorint CROController::DoFSM()
        * \retval -1 on error
        * \retval >=0 otherwise (the motor status)
        */ 
      int get_motor_status(int addr, int unit);
      
      ///Get the specified motor status.  Does not call \ref check_motor_status so the return value may be out of date.
      /** \param mot is the name of the motor
        * \retval -1 on error
        * \retval >=0 otherwise (the motor status)
        */ 
      int get_motor_status(const char *mot);

      ///Get the specified motor position.  Does not call \ref check_motor_status so the return value may be out of date.
      /** \param addr is the controller address of the motor
        * \param unit is the unit number of the motor
        * \retval -1 on error
        * \retval >=0 otherwise (the motor position)
        */ 
      int get_motor_position(int add, int unit);
      
      ///Get the specified motor position.  Does not call \ref check_motor_status so the return value may be out of date.
      /** \param mot is the name of the motor
        * \retval -1 on error
        * \retval >=0 otherwise (the motor position)
        */ 
      int get_motor_position(const char *mot);

      int update_RTDB(int i, int state);
      
      int move_motor(int addr, int unit, int steps);
      int move_motor(const char *mot, int steps);


      ///Move the X axis a number of steps.
      /** Moves motors A1 and B1.  Alternates in chunks of swap_steps to avoid developing too much angle.
        */
      int move_X(int steps);

      ///Move the X axis a number of steps.
      int move_Y(int steps);

      ///Move the X axis a number of steps.
      int move_Z(int steps);
      
      /*Utilities*/
      
      ///Utility function to convert a motor address and unit to its array index (for velocity, etc.)
      int motor_index(int addr, int unit);
      
      ///Utility function to convert a motor name to its array index (for velocity, etc.)
      int motor_index(const char *mot);
      
      ///Utility function to convert a motor name to its controller address
      int motor_addr(const char *mot);
      
      ///Utility function to convert a motor name to its unit number
      int motor_unit(const char *mot);
      
      ///Utility funciton to convert a status integer to a string description
      std::string status_string(int stat);

      ///Abort an ongoing move.
      int abort_move();

      virtual void Run();
      int DoFSM();
      
      /* handlers */
      static int A1_dpos_req_changed(void *pt, Variable *msgb);
      static int B1_dpos_req_changed(void *pt, Variable *msgb);
      static int C1_dpos_req_changed(void *pt, Variable *msgb);
      static int A2_dpos_req_changed(void *pt, Variable *msgb);
      static int B2_dpos_req_changed(void *pt, Variable *msgb);
      static int X_dpos_req_changed(void *pt, Variable *msgb);
      static int Y_dpos_req_changed(void *pt, Variable *msgb);
      static int Z_dpos_req_changed(void *pt, Variable *msgb);
      static int abort_req_changed(void *pt, Variable *msgb);
      static int stepmode_req_changed(void *pt, Variable *msgb);
};

} //namespace MagAO

void dcn_error_message(const char *, const char *, int);
void dcn_warning_message(const char *, const char *, int);

#endif //__CROController_h__
