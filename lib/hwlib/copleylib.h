
#ifndef COPLEYLIB_H
#define COPLEYLIB_H

// Base communication

int copleyCmd( int *value, char *fmt, ...);
int copleyBinaryCmd( char *buf, int buflen, char *answbuf, int answbuflen, int timeout);

// Low-level Copley commands

int copleySet( char bank, int id, int value);
int copleyGet( char bank, int id, int *value);
int copleyCopy( char bank, int id);
int copleyReset(void);
int copleyTrajectory( int subcommand);
int copleyReadRegister( int regnum, int *value);
int copleyWriteRegister( int regnum, int value);

// Register bits

int copleyRegisterBit( int regnum, int bit); 
int copleyEventStatusBit( int bit);
int copleyTrajectoryStatusBit( int bit);
int copleyFaultStatusBit( int bit);
int isBitSet( int value, int bit); 
int isBitNotSet( int value, int bit);

int copleyValue( char bank, int id, double scale, double *value);

int copleyInputs( int *state);

int copleyInputAddr( int input);
int copleyResetInputFunction( int function);

int copleySetPositionGains(int Pp, int Aff);


// ---------- Sligthly higher-level...

int copleyInit( char *addr, int port);
int copleyShutdown(void);
int copleyCommTest(void);

// Movement triggers

int copleyMove(void);
int copleyAbort(void);
int copleyHome(void);

int copleyMoveTo( int pos);

// Movement type

int copleySetAbsolutePositioning(void);
int copleySetRelativePositioning(void);

// Movement parameters

int copleySetMoveTarget( int pos);
int copleySetMoveSpeed( int speed);

int copleySetAcceleration( int acc);
int copleySetDeceleration( int dec);
int copleySetAbortDeceleration( int dec);

// Brake parameters

int copleySetBrakeDelay( double secs);
int copleySetHandBrake( double secs);


// Enable/Disable

int copleyEnableInPositioningMode(void);
int copleyDisable(void);

// Query momement status

int copleyIsMoving(void);
int copleyIsAborted(void);
int copleyIsHomed(void);
int copleyPositiveLimit(void);
int copleyNegativeLimit(void);

// Homing

int copleySetHomingMethod( int method);
int copleySetFastHomingSpeed( int speed);
int copleySetSlowHomingSpeed( int speed);
int copleySetHomingAcceleration( int acc);
int copleySetHomingOffset( int offset);

// Movement limits

int copleySetPosLimitSwitchInput( int input);
int copleySetNegLimitSwitchInput( int input);
int copleySetHomeSwitchInput( int input);
int copleyEnablePosLimitSwitch(void);
int copleyEnableNegLimitSwitch(void);
int copleyEnableHomeSwitch(void);
int copleyDisablePosLimitSwitch(void);
int copleyDisableNegLimitSwitch(void);
int copleyDisableHomeSwitch(void);
int copleySetPositiveSoftwareLimit( int pos);
int copleySetNegativeSoftwareLimit( int pos);

// Diagnostics
	
int copleyCommandedCurrent( double *value);
int copleyActualCurrent( double *value);
int copleyLimitedCurrent( double *value);
int copleyCommandedVelocity( double *value);
int copleyLimitedVelocity( double *value);
int copleyActualMotorVelocity( double *value);
int copleyActualLoadVelocity( double *value);
int copleyVelocityLoopError( double *value);
int copleyMotorPosition( double *value);
int copleyLoadPosition( double *value);
int copleyFollowingError( double *value);
int copleyBusVoltage( double *value);
int copleyTemperature( double *value);

// Parameter file upload

int copleyLoadParameters( char *filename);
int copleyCurStatus( char *errstr, int len);

   

#define COPLEY_HOMING_CURPOS (512)
#define COPLEY_HOMING_NEXT_INDEX_POS (544)
#define COPLEY_HOMING_NEXT_INDEX_NEG (560)
#define COPLEY_HOMING_LIMIT_SWITCH_POS (513)
#define COPLEY_HOMING_LIMIT_SWITCH_NEG (529)
#define COPLEY_HOMING_LIMIT_SWITCH_OUT_INDEX_POS (545)
#define COPLEY_HOMING_LIMIT_SWITCH_OUT_INDEX_NEG (561)
#define COPLEY_HOMING_HOME_SWITCH_POS (514)
#define COPLEY_HOMING_HOME_SWITCH_NEG (530)



#endif // COPLEYLIB_H

