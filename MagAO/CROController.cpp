
#include "CROController.h"

namespace MagAO
{
   
CROController::CROController(int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   n_motors = 5;
   Create();
}

CROController::~CROController()
{
   abort_move();
   
   if(n_motors)
   {
      delete position;
      delete mode;
      delete vel;
      delete acc;
   }
   
   if(head)
   {
      if(dcn_port) serial_close(head->fd);
      else close(head->fd);
      dcn_shutdown();
   }

   setCurState(STATE_UNDEFINED);
   A1_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
   B1_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
   C1_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
   A2_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
   B2_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
}

void CROController::Create()
{
   head = 0;

   position = new int[n_motors];
   mode = new int[n_motors];
   vel = new int[n_motors];
   acc = new int[n_motors];
   
   //Fill in the parameter vectors with defaults
   for(int i=0; i< n_motors; i++)
   {
      position[i] = 0;
      mode[i] = DCN_STEP_MODE1X;
      vel[i] = DCN_STEP_DEFVEL;
      acc[i] = DCN_STEP_DEFACC;
   }
   swap_steps = 1000; //default value

   LoadConfig();
   
}

int CROController::LoadConfig()
{
   Config_File *cfg = &ConfigDictionary();
   try
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Reading vars.");
      dcn_addr  = (std::string) (*cfg)["dcn_addr"];
      dcn_port = (int)(*cfg)["dcn_port"];
      _logger->log(Logger::LOG_LEV_INFO, "Config: ip %s port %i", dcn_addr.c_str(), dcn_port);
   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing serial configuration data: %s", e.what().c_str());
      throw(e);
   }

   int arg;
   //Optional config file params
   try
   {
      arg = (int)(*cfg)["swap_steps"];
      swap_steps = arg;
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }
   
   try
   {
      arg = (int)(*cfg)["A1_vel"];
      set_motor_vel("A1", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }

   try
   {
      arg = (int)(*cfg)["B1_vel"];
      set_motor_vel("B1", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }

   try
   {
      arg = (int)(*cfg)["C1_vel"];
      set_motor_vel("C1", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }

   try
   {
      arg = (int)(*cfg)["A2_vel"];
      set_motor_vel("A2", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }

   try
   {
      arg = (int)(*cfg)["B2_vel"];
      set_motor_vel("B2", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }

   try
   {
      arg = (int)(*cfg)["A1_acc"];
      set_motor_acc("A1", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }
   
   try
   {
      arg = (int)(*cfg)["B1_acc"];
      set_motor_acc("B1", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }
   
   try
   {
      arg = (int)(*cfg)["C1_acc"];
      set_motor_acc("C1", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }
   
   try
   {
      arg = (int)(*cfg)["A2_acc"];
      set_motor_acc("A2", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }
   
   try
   {
      arg = (int)(*cfg)["B2_acc"];
      set_motor_acc("B2", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }

   try
   {
      arg = (int)(*cfg)["step_mode"];
      set_motor_mode("A1", arg);
      set_motor_mode("B1", arg);
      set_motor_mode("C1", arg);
      set_motor_mode("A2", arg);
      set_motor_mode("B2", arg);
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing
   }
   

   return NO_ERROR;
}

void CROController::SetupVars()
{
   //Setup the CROController RTDB variables
   try
   {
      A1_pos_cur = RTDBvar(this->MyFullName(), "A1_pos", CUR_VAR, INT_VARIABLE, 1,1);
      A1_dpos_req = RTDBvar(this->MyFullName(), "A1_dpos", REQ_VAR, INT_VARIABLE, 1,1);
      A1_stat_cur = RTDBvar(this->MyFullName(), "A1_stat", CUR_VAR, INT_VARIABLE, 1,1);
      A1_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
      
      B1_pos_cur = RTDBvar(this->MyFullName(), "B1_pos", CUR_VAR, INT_VARIABLE, 1,1);
      B1_dpos_req = RTDBvar(this->MyFullName(), "B1_dpos", REQ_VAR, INT_VARIABLE, 1,1);
      B1_stat_cur = RTDBvar(this->MyFullName(), "B1_stat", CUR_VAR, INT_VARIABLE, 1,1);
      B1_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
      
      C1_pos_cur = RTDBvar(this->MyFullName(), "C1_pos", CUR_VAR, INT_VARIABLE, 1,1);
      C1_dpos_req = RTDBvar(this->MyFullName(), "C1_dpos", REQ_VAR, INT_VARIABLE, 1,1);
      C1_stat_cur = RTDBvar(this->MyFullName(), "C1_stat", CUR_VAR, INT_VARIABLE, 1,1);
      C1_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
      
      A2_pos_cur = RTDBvar(this->MyFullName(), "A2_pos", CUR_VAR, INT_VARIABLE, 1,1);
      A2_dpos_req = RTDBvar(this->MyFullName(), "A2_dpos", REQ_VAR, INT_VARIABLE, 1,1);
      A2_stat_cur = RTDBvar(this->MyFullName(), "A2_stat", CUR_VAR, INT_VARIABLE, 1,1);
      A2_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
      
      B2_pos_cur = RTDBvar(this->MyFullName(), "B2_pos", CUR_VAR, INT_VARIABLE, 1,1);
      B2_dpos_req = RTDBvar(this->MyFullName(), "B2_dpos", REQ_VAR, INT_VARIABLE, 1,1);
      B2_stat_cur = RTDBvar(this->MyFullName(), "B2_stat", CUR_VAR, INT_VARIABLE, 1,1);
      B2_stat_cur.Set(STATE_NOCONNECTION, 0, FORCE_SEND);
      
      X_pos_cur = RTDBvar(this->MyFullName(), "X_pos", CUR_VAR, INT_VARIABLE, 1,1);
      X_dpos_req = RTDBvar(this->MyFullName(), "X_dpos", REQ_VAR, INT_VARIABLE, 1,1);
      
      Y_pos_cur = RTDBvar(this->MyFullName(), "Y_pos", CUR_VAR, INT_VARIABLE, 1,1);
      Y_dpos_req = RTDBvar(this->MyFullName(), "Y_dpos", REQ_VAR, INT_VARIABLE, 1,1);
      
      Z_pos_cur = RTDBvar(this->MyFullName(), "Z_pos", CUR_VAR, INT_VARIABLE, 1,1);
      Z_dpos_req = RTDBvar(this->MyFullName(), "Z_dpos", REQ_VAR, INT_VARIABLE, 1,1);

      abort_req = RTDBvar(this->MyFullName(), "abort", REQ_VAR, INT_VARIABLE, 1,1);
      pending_moves = RTDBvar(this->MyFullName(), "pendingmoves", CUR_VAR, INT_VARIABLE, 1,1);
      var_stepmode_cur = RTDBvar(this->MyFullName(), "stepmode", CUR_VAR, INT_VARIABLE, 1,1);
      var_stepmode_cur.Set(mode[0], 0, FORCE_SEND);
      var_stepmode_req = RTDBvar(this->MyFullName(), "stepmode", REQ_VAR, INT_VARIABLE, 1,1);
   }
   catch (AOVarException &e)
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
   
   Notify(A1_dpos_req, A1_dpos_req_changed);
   Notify(B1_dpos_req, B1_dpos_req_changed);
   Notify(C1_dpos_req, C1_dpos_req_changed);
   Notify(A2_dpos_req, A2_dpos_req_changed);
   Notify(B2_dpos_req, B2_dpos_req_changed);
   Notify(X_dpos_req, X_dpos_req_changed);
   Notify(Y_dpos_req, Y_dpos_req_changed);
   Notify(Z_dpos_req, Z_dpos_req_changed);
   Notify(abort_req, abort_req_changed);
   Notify(var_stepmode_req, stepmode_req_changed);
}


int CROController::init_controllers()
{
   struct DCN *motor;
   int i;
   
   //Initialize the controllers.  Should find the 2 pico controllers.
   head = dcn_init(0, dcn_addr.c_str(), dcn_port);
   
   if(!head) return -1;

   if(head->num_addrs == 0)
   {
      dcn_shutdown();
      return 0;
   }
   motor = head->dcn; /* stat all controllers */
   
   i = 0;
   while( motor ) 
   {
      dcn_motor_status(motor); /* really controller status */
               
      #ifdef DEBUG
         std::cout << "Motor " << i << " status: " << motor->status << " " << status_string(motor->status) << ". ";
         std::cout << "position: " << motor->position << ".\n";
      #endif
         
      motor = motor->next;
      i++;
   }
   
   return 0;

}

int CROController::set_motor_mode(const char *mot, int newmode)
{
   int i = motor_index(mot);
   if(i < 0 || i>=n_motors)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "unkown motor (%s) in set_motor_mode.  %s  %i", mot, __FILE__, __LINE__);
      return -1;
   }

   if(newmode != DCN_STEP_MODE1X && newmode !=DCN_STEP_MODE2X && newmode != DCN_STEP_MODE4X && newmode != DCN_STEP_MODE8X)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "unknown mode (%i) in set_motor_mode.  %s  %i", newmode,  __FILE__, __LINE__);
      return -1;
   }
   
   mode[i] = newmode;
   return 0;
}

int CROController::set_motor_vel(const char *mot, int newvel)
{
   int i = motor_index(mot);
   if(i < 0 || i>=n_motors)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "unkown motor (%s) in set_motor_vel.  %s %i", mot, __FILE__, __LINE__);
      return -1;
   }
   
   if(newvel < 1 || newvel > 250)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "bad velocity (%i) in set_motor_vel.  %s  %i", newvel,  __FILE__, __LINE__);
      return -1;
   }
   
   vel[i] = newvel;
   return 0;
}

int CROController::set_motor_acc(const char *mot, int newacc)
{
   int i = motor_index(mot);
   if(i < 0 || i>=n_motors)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "unkown motor (%s) in set_motor_acc.  %s %i", mot, __FILE__, __LINE__);
      return -1;
   }
   
   if(newacc < 1 || newacc > 250)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "bad acceleration (%i) in set_motor_acc.  %s  %i", newacc,  __FILE__, __LINE__);
      return -1;
   }
   
   acc[i] = newacc;
   return 0;
}

int CROController::check_motor_status(int addr, int unit)
{
   struct DCN *motor;
   if(!head)
   {
      if(init_controllers() != 0)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "hardware not initialized.  %s  %i",  __FILE__, __LINE__);
         return -1;
      }
   }
   
   motor = find_dcn_motor( head, addr, unit );
   
   if(!motor)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "could not find motor %i %i.  %s  %i", addr, unit,  __FILE__, __LINE__);
      return -1;
   }
   return dcn_motor_status(motor);
   
   //return 0;
}

int CROController::check_motor_status(const char *mot)
{
   return check_motor_status(motor_addr(mot), motor_unit(mot));
}
      
int CROController::get_motor_status(int addr, int unit)
{
   struct DCN *motor;
   
   if(!head)
   {
      if(init_controllers() != 0)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "hardware not initialized.  %s  %i",  __FILE__, __LINE__);
         return -1;
      }
   }
   
   motor = find_dcn_motor( head, addr, unit );
      
   return motor->status;
}

int CROController::get_motor_status(const char *mot)
{
   return get_motor_status(motor_addr(mot), motor_unit(mot));
}


int CROController::get_motor_position(int addr, int unit)
{
   struct DCN *motor;
   
   if(!head)
   {
      if(init_controllers() != 0)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "hardware not initialized.  %s  %i",  __FILE__, __LINE__);
         return -1;
      }
   }
   
   motor = find_dcn_motor( head, addr, unit );
      
   return motor->position;
}

int CROController::get_motor_position(const char *mot)
{
   return get_motor_status(motor_addr(mot), motor_unit(mot));
}

int CROController::update_RTDB(int i, int state)
{
   try
   {
   switch(i)
   {
      case 0:
         A1_pos_cur.Set(position[i], 0, CHECK_SEND);
         X_pos_cur.Set((int)(-.5*(position[0] + position[1])), 0, CHECK_SEND);
         A1_stat_cur.Set(state, 0, CHECK_SEND);
         break;
      case 1:
         B1_pos_cur.Set(position[i], 0, CHECK_SEND);
         X_pos_cur.Set((int)(-.5*(position[0] + position[1])), 0, CHECK_SEND);
         B1_stat_cur.Set(state, 0, CHECK_SEND);
         break;
      case 2:
         C1_pos_cur.Set(position[i], 0, CHECK_SEND);
         Y_pos_cur.Set(-position[2], 0, CHECK_SEND);
         C1_stat_cur.Set(state, 0, CHECK_SEND);
         break;
      case 3:
         A2_pos_cur.Set(position[i], 0, CHECK_SEND);
         Z_pos_cur.Set((int)(.5*(position[3] + position[4])), 0, CHECK_SEND);
         A2_stat_cur.Set(state, 0, CHECK_SEND);
         break;
      case 4:
         B2_pos_cur.Set(position[i], 0, CHECK_SEND);
         Z_pos_cur.Set((int)(.5*(position[3] + position[4])), 0, CHECK_SEND);
         B2_stat_cur.Set(state, 0, CHECK_SEND);
         break;
      default:
         break;
   }

      return 0;
   }
   catch(...)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Exception caught updating RTDB",  __FILE__, __LINE__);
      throw;
   }
}

int CROController::move_motor(int addr, int unit, int steps)
{
   int rv = 0;
   struct DCN *motor;
   
   if(!head)
   {
      if(init_controllers() != 0)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "hardware not initialized.  %s  %i",  __FILE__, __LINE__);
         return -1;
      }
   }
   
   motor = find_dcn_motor( head, addr, unit );

   int i = motor_index(addr, unit);

   if(i < 0 || i>=n_motors)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "unknown motor (%i).  %s  %i", i,  __FILE__, __LINE__);
      return -1;
   }
   
   #ifdef DEBUG
      std::cout << "Starting move of Motor " << i << " status: " <<  "position: " << position[i] << ".\n";
   #endif
   
   dcn_motor_set_params( motor, mode[i], vel[i], acc[i] );

   dcn_motor_move_rel( motor, steps );
   update_RTDB(i, STATE_OPERATING);
   
   if( dcn_motor_wait_stop( motor ) )
   {
      _logger->log(Logger::LOG_LEV_ERROR, "motor timeout.  %s  %i",  __FILE__, __LINE__);
      rv = -1;
   }
   else position[i] += steps;
   dcn_motor_disable( motor );

   update_RTDB(i, STATE_READY);
   
   #ifdef DEBUG
      check_motor_status(addr, unit);
      std::cout << "completed move of Motor " << i << " status: " <<  "position: " << position[i] << ".\n";
   #endif
   
   return rv;
}

int CROController::move_motor(const char *mot, int steps)
{
   return move_motor(motor_addr(mot), motor_unit(mot), steps);
}

int CROController::move_X(int steps)
{
   int nswitches, remsteps;
   int sign = -1;
   if(steps < 0) sign = 1;
   
   nswitches = abs(steps)/swap_steps;
   remsteps = abs(steps) - nswitches*swap_steps;

   //std::cout << nswitches << "\n";
   //std::cout << remsteps << "\n";
   for(int i=0; i < nswitches; i++)
   {
      if(Aborted) return 0;
      move_motor("A1", sign*swap_steps);
      if(Aborted) return 0;
      move_motor("B1", sign*swap_steps);
   }
   if(Aborted) return 0;
   move_motor("A1", sign*remsteps);
   if(Aborted) return 0;
   move_motor("B1", sign*remsteps);

   return 0;
}

int CROController::move_Y(int steps)
{
   int nswitches, remsteps;
   int sign = -1;
   if(steps < 0) sign = 1;
   
   nswitches = abs(steps)/swap_steps;
   remsteps = abs(steps) - nswitches*swap_steps;

   for(int i=0; i < nswitches; i++)
   {
      if(Aborted) return 0;
      move_motor("C1", sign*swap_steps);
   }

   if(Aborted) return 0;
   move_motor("C1", sign*remsteps);

   return 0;
}

int CROController::move_Z(int steps)
{
   int nswitches, remsteps;
   int sign = 1;
   if(steps < 0) sign = -1;
   
   nswitches = abs(steps)/swap_steps;
   remsteps = abs(steps) - nswitches*swap_steps;
   
   //std::cout << nswitches << "\n";
   //std::cout << remsteps << "\n";
   for(int i=0; i < nswitches; i++)
   {
      if(Aborted) return 0;
      move_motor("A2", sign*swap_steps);
      if(Aborted) return 0;
      move_motor("B2", sign*swap_steps);
   }
   if(Aborted) return 0;
   move_motor("A2", sign*remsteps);
   if(Aborted) return 0;
   move_motor("B2", sign*remsteps);
 
   return 0;
}

int CROController::motor_index(int addr, int unit)
{
   return (addr-1)*3+unit;
}

int CROController::motor_index(const char *mot)
{
   return motor_index(motor_addr(mot), motor_unit(mot));
}

int CROController::motor_addr(const char *mot)
{
   int addr = mot[1] - '1' + 1;
   return addr;
}     


int CROController::motor_unit(const char *mot)
{
   int unit = mot[0] - 'A';
   return unit;
   
}     

std::string CROController::status_string(int stat)
{
   switch(stat)
   {
      case DCN_MOTOR_MOVING: return "MOTOR MOVING";
      case DCN_CKSUM_ERROR:  return "CHECKSUM_ERROR"; 
      case DCN_AMP_ENABLED:  return "AMP ENABLED"; 
      case DCN_POWER_ON:     return "POWER ON"; 
      case DCN_AT_SPEED:     return "AT SPEED"; 
      case DCN_VEL_MODE:     return "VELOCITY MODE";
      case DCN_TRAP_MODE:    return "TRAP MODE";
      case DCN_HOME_IN_PROG: return "HOME IN PROGRESS";
      default:               return "UNKOWN STATUS";
   }
}

int CROController::abort_move()
{
   struct DCN *motor;

   Aborted = 1;
   moveList.clear(); //erase all pending moves.
   if(!head)
   {
      if(init_controllers() != 0)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "hardware not initialized.  %s  %i",  __FILE__, __LINE__);
         return -1;
      }
   }

   
   motor = find_dcn_motor( head, motor_addr("A1"), motor_unit("A1"));
   dcn_motor_stop(motor, false);

   motor = find_dcn_motor( head, motor_addr("B1"), motor_unit("B1"));
   dcn_motor_stop(motor, false);

   motor = find_dcn_motor( head, motor_addr("C1"), motor_unit("C1"));
   dcn_motor_stop(motor, false);

   motor = find_dcn_motor( head, motor_addr("A2"), motor_unit("A2"));
   dcn_motor_stop(motor, false);

   motor = find_dcn_motor( head, motor_addr("B2"), motor_unit("B2"));
   dcn_motor_stop(motor, false);

   return 0;
}

int CROController::A1_dpos_req_changed(void *pt, Variable *msgb)
{
   CROController * cro = (CROController *) pt;
   pending_move pm;
   pm.type = 0;
   pm.dpos = msgb->Value.Lv[0];

   cro->Aborted = 0;
   cro->moveList.push_back(pm);
   cro->pending_moves.Set((int)cro->moveList.size(), 0, CHECK_SEND);
   
   return 0;
}

int CROController::B1_dpos_req_changed(void *pt, Variable *msgb)
{
   CROController * cro = (CROController *) pt;
   pending_move pm;
   pm.type = 1;
   pm.dpos = msgb->Value.Lv[0];

   cro->Aborted = 0;
   cro->moveList.push_back(pm);
   cro->pending_moves.Set((int)cro->moveList.size(), 0, CHECK_SEND);
   
   return 0;
}

int CROController::C1_dpos_req_changed(void *pt, Variable *msgb)
{
   CROController * cro = (CROController *) pt;
   pending_move pm;
   pm.type = 2;
   pm.dpos = msgb->Value.Lv[0];

   cro->Aborted = 0;
   cro->moveList.push_back(pm);
   cro->pending_moves.Set((int)cro->moveList.size(), 0, CHECK_SEND);
   
   return 0;
}

int CROController::A2_dpos_req_changed(void *pt, Variable *msgb)
{
   CROController * cro = (CROController *) pt;
   pending_move pm;
   pm.type = 3;
   pm.dpos = msgb->Value.Lv[0];

   cro->Aborted = 0;
   cro->moveList.push_back(pm);
   cro->pending_moves.Set((int)cro->moveList.size(), 0, CHECK_SEND);
   
   return 0;
}

int CROController::B2_dpos_req_changed(void *pt, Variable *msgb)
{
   CROController * cro = (CROController *) pt;
   pending_move pm;
   pm.type = 4;
   pm.dpos = msgb->Value.Lv[0];

   cro->Aborted = 0;
   cro->moveList.push_back(pm);
   cro->pending_moves.Set((int)cro->moveList.size(), 0, CHECK_SEND);
   
   return 0;
}

int CROController::X_dpos_req_changed(void *pt, Variable *msgb)
{
   CROController * cro = (CROController *) pt;
   pending_move pm;
   pm.type = 5;
   pm.dpos = msgb->Value.Lv[0];

   cro->Aborted = 0;
   cro->moveList.push_back(pm);
   cro->pending_moves.Set((int)cro->moveList.size(), 0, CHECK_SEND);
   
   return 0;
}

int CROController::Y_dpos_req_changed(void *pt, Variable *msgb)
{
   CROController * cro = (CROController *) pt;
   pending_move pm;
   pm.type = 6;
   pm.dpos = msgb->Value.Lv[0];

   cro->Aborted = 0;
   cro->moveList.push_back(pm);
   cro->pending_moves.Set((int)cro->moveList.size(), 0, CHECK_SEND);
   
   return 0;
}

int CROController::Z_dpos_req_changed(void *pt, Variable *msgb)
{
   CROController * cro = (CROController *) pt;
   pending_move pm;
   pm.type = 7;
   pm.dpos = msgb->Value.Lv[0];

   cro->Aborted = 0;
   cro->moveList.push_back(pm);
   cro->pending_moves.Set((int)cro->moveList.size(), 0, CHECK_SEND);
   return 0;
}

int CROController::abort_req_changed(void *pt, Variable *msgb)
{
   int dpos;
   CROController * cro = (CROController *) pt;
   
   dpos = msgb->Value.Lv[0];
   _logger->log(Logger::LOG_LEV_INFO, "Abort requested.");

   cro->abort_move();
   
   return 0;
}

int CROController::stepmode_req_changed(void *pt, Variable *msgb)
{
   int mode;
   CROController * cro = (CROController *) pt;
   
   mode = msgb->Value.Lv[0];
   _logger->log(Logger::LOG_LEV_INFO, "Changing step mode to %i", mode);

   cro->set_motor_mode("A1", mode);
   cro->set_motor_mode("B1", mode);
   cro->set_motor_mode("C1", mode);
   cro->set_motor_mode("A2", mode);
   cro->set_motor_mode("B2", mode);

   cro->var_stepmode_cur.Set(mode, 0, CHECK_SEND);
   
   return 0;
}

void CROController::Run()
{
   _logger->log( Logger::LOG_LEV_INFO, "Running...");
   
   while(!TimeToDie())
   {
      try
      {
         DoFSM();
      }
      catch (AOException &e)
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());
         
         // When the exception is thrown, the mutex was held!
         //pthread_mutex_unlock(&threadMutex);
      }
   }
}


int CROController::DoFSM()
{
   int status;//, stat;
   static float delay=1.;
      
   status = getCurState();

   if(status != STATE_READY && status != STATE_OPERATING)
   {
      status = STATE_NOCONNECTION;
      if(init_controllers() == 0)
      {
         _logger->log(Logger::LOG_LEV_INFO, "found %i motors.  %s  %i", head->num_addrs,  __FILE__, __LINE__);
         //Make sure we are actually talking to the controllers.
         if(head->num_addrs == 2) status = STATE_READY;
      }
   }
   else
   {
      //Check for communications.
      if(check_motor_status("A1") == 255)
      {
         std::cerr << "Failed comms check.\n";
         status = STATE_NOCONNECTION;
         if(head) serial_close(head->fd);
         dcn_shutdown();
      }
   }
   setCurState(status);
   
   if(status == STATE_READY || status == STATE_OPERATING)
   {
      pending_moves.Set((int)moveList.size(), 0, CHECK_SEND);
      
      if(moveList.size() > 0)
      {
         switch(moveList.front().type)
         {
            case 0:
               move_motor("A1", moveList.front().dpos);
               break;
            case 1:
               move_motor("B1", moveList.front().dpos);
               break;
            case 2:
               move_motor("C1", moveList.front().dpos);
               break;
            case 3:
               move_motor("A2", moveList.front().dpos);
               break;
            case 4:
               move_motor("B2", moveList.front().dpos);
               break;
            case 5:
               move_X(moveList.front().dpos);
               break;
            case 6:
               move_Y(moveList.front().dpos);
               break;
            case 7:
               move_Z(moveList.front().dpos);
               break;
            default:
               break;
         }
         if(moveList.size() > 0) moveList.pop_front(); //If aborted, moveList will be cleared.
      }
      else nusleep( (unsigned int)(delay * 1e6)); //only sleep if no move
   }
   else nusleep( (unsigned int)(delay * 1e6));

   
   
   return NO_ERROR;
}






}//namespace MagAO

void dcn_error_message(const char *msg, const char *file, int lineno)
{
   Logger::get()->log(Logger::LOG_LEV_ERROR, "%s. %s, %i", msg, file, lineno);
}

void dcn_warning_message(const char *msg, const char *file, int lineno)
{
   Logger::get()->log(Logger::LOG_LEV_WARNING, "%s. %s, %i", msg, file, lineno);
}

