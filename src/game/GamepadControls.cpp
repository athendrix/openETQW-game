// SPDX-License-Identifier: MIT

#include "precompiled.h"
#pragma hdrstop

#include "GamepadControls.h"
#include "Player.h"
#include "Weapon.h"
#include "roles/Inventory.h"
#include "roles/HudModule.h"
#include "decls/DeclInvItem.h"
#include "ContentMask.h"
#include "script/Script_ScriptObject.h"

/*
===============================================================================

	CVars

===============================================================================
*/

// stick aiming
idCVar in_aim_yawSpeed( "in_aim_yawSpeed", "200", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: turn speed at full stick, degrees a second", 10.0f, 1000.0f );
idCVar in_aim_pitchSpeed( "in_aim_pitchSpeed", "140", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: look up and down speed at full stick, degrees a second", 10.0f, 1000.0f );
idCVar in_aim_deadZone( "in_aim_deadZone", "0.1", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: look stick dead zone, as a share of the stick's travel (radial)", 0.0f, 0.9f );
idCVar in_aim_outerZone( "in_aim_outerZone", "0.03", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: the look stick counts as fully pushed this close to its edge", 0.0f, 0.5f );
idCVar in_aim_curve( "in_aim_curve", "2.2", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: look response curve exponent; 1 is linear, higher is finer near the centre", 1.0f, 5.0f );
idCVar in_aim_rampDelay( "in_aim_rampDelay", "0.15", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: seconds the look stick is held at the edge before the turn speeds up", 0.0f, 2.0f );
idCVar in_aim_rampTime( "in_aim_rampTime", "0.35", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: seconds the turn takes to reach its full extra speed", 0.0f, 2.0f );
idCVar in_aim_rampBoost( "in_aim_rampBoost", "0.8", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: extra turn speed at the end of the ramp, as a share of in_aim_yawSpeed (0 turns the ramp off)", 0.0f, 3.0f );
idCVar in_aim_zoomScale( "in_aim_zoomScale", "1", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: look speed while zoomed, on top of the weapon's own fov scaling", 0.1f, 3.0f );
idCVar in_aim_invertPitch( "in_aim_invertPitch", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL | CVAR_PROFILE, "gamepad: pushing the look stick up looks down" );
idCVar in_aim_lookX( "in_aim_lookX", "2", CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER | CVAR_PROFILE, "gamepad: axis that turns (2 right stick, 0 left stick for southpaw)", 0, 7 );
idCVar in_aim_lookY( "in_aim_lookY", "3", CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER | CVAR_PROFILE, "gamepad: axis that looks up and down (3 right stick, 1 left stick for southpaw)", 0, 7 );
idCVar in_move_moveX( "in_move_moveX", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER | CVAR_PROFILE, "gamepad: axis that strafes (0 left stick, 2 right stick for southpaw)", 0, 7 );
idCVar in_move_moveY( "in_move_moveY", "1", CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER | CVAR_PROFILE, "gamepad: axis that moves forward and back (1 left stick, 3 right stick for southpaw)", 0, 7 );
idCVar in_move_deadZone( "in_move_deadZone", "0.15", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad: move stick dead zone (radial)", 0.0f, 0.9f );

// gyro (the engine sends it on axes 6 and 7 with in_gamepadGyro 1)
idCVar in_gyro_speed( "in_gyro_speed", "1.5", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "gamepad gyro: view turn per degree the pad turns (1 is one to one)", 0.0f, 10.0f );
idCVar in_gyro_invertPitch( "in_gyro_invertPitch", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL | CVAR_PROFILE, "gamepad gyro: tilting the pad up looks down" );
idCVar in_gyro_zoomedOnly( "in_gyro_zoomedOnly", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL | CVAR_PROFILE, "gamepad gyro: only aim with the gyro while zoomed" );

// aim assist
idCVar in_aim_assist( "in_aim_assist", "1", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL | CVAR_PROFILE, "gamepad: aim assist on foot (slows the look speed on an enemy and follows a moving target)" );
idCVar in_aim_assistFriction( "in_aim_assistFriction", "0.5", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "aim assist: look speed with the crosshair right on a target (1 is no slow down)", 0.1f, 1.0f );
idCVar in_aim_assistAdhesion( "in_aim_assistAdhesion", "0.4", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "aim assist: share of a target's motion across the view that the view follows while you aim or move", 0.0f, 0.9f );
idCVar in_aim_assistRadius( "in_aim_assistRadius", "40", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "aim assist: size of the area around a target, in world units, where assist acts", 4.0f, 200.0f );
idCVar in_aim_assistRange( "in_aim_assistRange", "4000", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "aim assist: farthest target, in world units", 256.0f, 16384.0f );
idCVar in_aim_assistDebug( "in_aim_assistDebug", "0", CVAR_GAME | CVAR_BOOL, "aim assist: print the target and the assist applied" );

// tap and hold
idCVar in_padHoldTime( "in_padHoldTime", "250", CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER | CVAR_PROFILE, "_reloaduse: milliseconds a press must last to use instead of reload", 100, 1000 );

// pad sprint
idCVar in_gunsOnly( "in_gunsOnly", "1", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL | CVAR_PROFILE, "weapon keys, the wheel and next/previous weapon only select the guns (sidearm and primary); grenades, melee, tools and items are used by their own buttons" );
idCVar in_quickDebug( "in_quickDebug", "0", CVAR_GAME | CVAR_BOOL, "print what the quick grenade, melee, tool and item actions do" );
idCVar in_padSprintStick( "in_padSprintStick", "0.3", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "_padsprint: sprint stops when the move stick's forward push drops under this", 0.0f, 0.95f );

namespace {

// the grenade bank, as the bots use it (botai/BotThreadData.h GRENADE_SLOT); _weapon3
const int GAMEPAD_GRENADE_BANK = 3;

// how long a synthetic alt-fire or fire press lasts: two usercmd ticks, so the server sees it
const int GAMEPAD_PRESS_MSEC = 2 * USERCMD_MSEC + 10;

// after an alt-fire press, how long to wait for the weapon to change state before trying again
const int GAMEPAD_ZOOM_RETRY_MSEC = 800;

// the melee bank (_weapon0) and the items bank (_weapon5), as retail's defaultbinds.cfg names them
// (the inventory counts banks from 0: decls/DeclInvSlot.cpp takes one off the decl's number)
const int GAMEPAD_MELEE_BANK = 0;
const int GAMEPAD_ITEMS_BANK = 5;
// the guns: the sidearm (_weapon1) and the two-handed primary (_weapon2)
const int GAMEPAD_SIDEARM_BANK = 1;
const int GAMEPAD_PRIMARY_BANK = 2;
// the binoculars (_weapon6): Destiny's heavy weapon here
const int GAMEPAD_BINOCS_BANK = 6;
// the longest the super holds the pliers on a mine (arming takes about 2 s)
const int GAMEPAD_ARM_MSEC = 6000;
// how long something other than a gun may sit ready in hand before the gun comes back
const int GAMEPAD_RETURN_MSEC = 2000;
// grenade and melee pressed this close together are the super (the item)
const int GAMEPAD_CHORD_MSEC = 100;

// quick grenade and melee hold fire at least this long (a grenade's GRENADE_MINRELEASETIME is 0.1 s)
const int GAMEPAD_QUICK_MIN_FIRE_MSEC = 150;
// the tools bank (_weapon4: the class tool) as retail's defaultbinds.cfg names it
const int GAMEPAD_TOOLS_BANK = 4;
// how long a quick action waits for the weapon to come up (a slow lower and raise)
const int GAMEPAD_QUICK_SWITCH_MSEC = 4000;

// does the weapon's script have sights or a scope (weapons/base.script)
bool WeaponHasSights( idWeapon* weapon ) {
	if ( weapon == NULL ) {
		return false;
	}
	idScriptObject* object = weapon->GetScriptObject();
	if ( object == NULL ) {
		return false;
	}
	const char* names[] = { "hasIronSights", "hasScope" };
	for ( int i = 0; i < 2; i++ ) {
		const int* value = reinterpret_cast< const int* >( object->GetVariable( names[ i ], ev_boolean ) );
		if ( value != NULL && *value != 0 ) {
			return true;
		}
	}
	return false;
}

bool WeaponIsZoomed( idPlayer* player, idWeapon* weapon ) {
	if ( weapon == NULL ) {
		return false;
	}
	return weapon->IsIronSightsEnabled() || player->IsSniperScopeUp();
}

bool OnFoot( idPlayer* player ) {
	return player != NULL && player->GetProxyEntity() == NULL && player->GetHealth() > 0 &&
		!player->IsSpectator() && !player->IsInLimbo();
}

} // namespace

/*
===============================================================================

	sdGamepadControls

===============================================================================
*/

sdGamepadControls& sdGamepadControls::GetInstance( void ) {
	static sdGamepadControls instance;
	return instance;
}

sdGamepadControls::sdGamepadControls( void ) {
	Reset();
}

void sdGamepadControls::Reset( void ) {
	lastAimTime = 0;
	rampHeld = 0.0f;
	assistTarget = -1;
	assistSpawnId = 0;
	assistBearing.Zero();
	assistTime = 0;
	zoomWanted = false;
	zoomPulseUntil = 0;
	zoomNextPulse = 0;
	zoomPulses = 0;
	quickHeld = false;
	quickMelee = false;
	quickBank = -1;
	quickState = QG_IDLE;
	quickPrevWeapon = -1;
	quickDeadline = 0;
	quickFireStart = 0;
	quickTarget = -1;
	quickAlt = false;
	quickReturn = -1;
	quickArm = NULL;
	quickArmWas = false;
	quickSawAction = false;
	itemHeld = false;
	itemActed = false;
	itemShortPending = false;
	itemStart = 0;
	toolHeld = false;
	toolPressPending = false;
	deployWeapon = -1;
	deployReturn = -1;
	deployStart = 0;
	deploySawPlacing = false;
	stickyWeapon = -1;
	stickyReturn = -1;
	stickyStart = 0;
	memset( &grenadeButton, 0, sizeof( grenadeButton ) );
	memset( &meleeButton, 0, sizeof( meleeButton ) );
	superChord = false;
	heavyPending = false;
	lastGun = -1;
	offGunSince = 0;
	padSprint = false;
	padSprintHeld = false;
	tapHoldMode = TH_RELOAD_USE;
	reloadUseHeld = false;
	reloadUseActed = false;
	reloadPending = false;
	reloadUseStart = 0;
	activatePulseUntil = 0;
	padFireHeld = false;
	triggerDrivingTime = 0;
	crouchHeld = false;
	crouchActed = false;
	crouchLatched = false;
	crouchStandPending = false;
	crouchStart = 0;
	swapHeld = false;
	swapActed = false;
	swapPending = false;
	swapStart = 0;
	usedVehicleThisPress = false;
}

int sdGamepadControls::Now( void ) const {
	return sys->Milliseconds();
}

/*
================
sdGamepadControls::ApplyRadialDeadZone

The stick's length past the dead zone, rescaled to 0..1 (the outer zone counts as full),
raised to the curve exponent; the direction is kept, so diagonals are as fast as
straight pushes and the dead zone is a circle, not a cross.
================
*/
void sdGamepadControls::ApplyRadialDeadZone( float x, float y, float deadZone, float outerZone, float curve, float& outX, float& outY ) {
	outX = 0.0f;
	outY = 0.0f;
	// NaN fails both comparisons
	if ( !( idMath::Fabs( x ) <= 1e6f ) || !( idMath::Fabs( y ) <= 1e6f ) ) {
		return;
	}
	const float length = idMath::Sqrt( x * x + y * y );
	if ( length <= deadZone || length <= 0.0f ) {
		return;
	}
	const float span = 1.0f - deadZone - outerZone;
	const float t = span > 0.0f ? idMath::ClampFloat( 0.0f, 1.0f, ( length - deadZone ) / span ) : 1.0f;
	const float scaled = curve == 1.0f ? t : idMath::Pow( t, curve );
	outX = x / length * scaled;
	outY = y / length * scaled;
}

float sdGamepadControls::AngleBetween( const idVec3& a, const idVec3& b ) {
	const float lengths = a.Length() * b.Length();
	if ( lengths <= 0.0f ) {
		return 180.0f;
	}
	const float cosine = idMath::ClampFloat( -1.0f, 1.0f, ( a * b ) / lengths );
	return RAD2DEG( idMath::ACos( cosine ) );
}

/*
================
sdGamepadControls::FindAssistTarget

The cone around each target is the angle its assist radius covers at its distance,
between 1.5 and 10 degrees. closeness is 1 with the crosshair on the target and 0 at
the cone's edge.
================
*/
idPlayer* sdGamepadControls::FindAssistTarget( idPlayer* self, const idVec3& eye, const idVec3& forward, float& closeness, idVec3& targetPoint ) const {
	closeness = 0.0f;
	idPlayer* best = NULL;
	float bestAngle = idMath::INFINITY;
	const float range = in_aim_assistRange.GetFloat();
	const float radius = in_aim_assistRadius.GetFloat();

	for ( int i = 0; i < MAX_CLIENTS; i++ ) {
		idPlayer* other = gameLocal.GetClient( i );
		if ( other == NULL || other == self ) {
			continue;
		}
		if ( other->GetHealth() <= 0 || other->IsSpectator() || other->IsInLimbo() || other->IsDisguised() ) {
			continue;
		}
		if ( other->GetProxyEntity() != NULL ) {
			continue;	// in a vehicle or on a mounted gun
		}
		if ( self->GetEntityAllegiance( other ) != TA_ENEMY ) {
			continue;
		}

		// aim between the chest and the eyes
		const idVec3 otherEye = other->GetEyePosition();
		const idVec3 otherOrigin = other->GetPhysics()->GetOrigin();
		const idVec3 point = otherOrigin + ( otherEye - otherOrigin ) * 0.8f;
		const idVec3 toTarget = point - eye;
		const float distance = toTarget.Length();
		if ( distance < 1.0f || distance > range ) {
			continue;
		}
		const float cone = idMath::ClampFloat( 1.5f, 10.0f, RAD2DEG( idMath::ATan( radius, distance ) ) );
		const float angle = AngleBetween( forward, toTarget );
		if ( angle > cone || angle >= bestAngle ) {
			continue;
		}

		trace_t trace;
		gameLocal.clip.TracePoint( CLIP_DEBUG_PARMS trace, eye, point, CONTENTS_SOLID | CONTENTS_FORCEFIELD, self );
		if ( trace.fraction < 1.0f && trace.c.entityNum != other->entityNumber ) {
			continue;
		}

		best = other;
		bestAngle = angle;
		closeness = 1.0f - angle / cone;
		targetPoint = point;
	}
	return best;
}

/*
================
sdGamepadControls::PlayerControllerMove

The engine turns this call's angle change into a per-second rate (UsercmdGen.cpp
TurnTimeScale), so every turn here is a rate times the game's per-tick angle speed,
as the SDK's input modes do.
================
*/
void sdGamepadControls::PlayerControllerMove( bool doGameCallback, const int controllerNum, const float* controllerAxis, idVec3& viewAngles, usercmd_t& cmd ) {
	const int now = Now();
	float seconds = 0.0f;
	if ( lastAimTime != 0 && now > lastAimTime ) {
		seconds = idMath::ClampFloat( 0.0f, 0.1f, MS2SEC( now - lastAimTime ) );
	}
	lastAimTime = now;

	const float* axes = controllerAxis;
	const int lookXAxis = idMath::ClampInt( 0, MAX_CONTROLLER_AXIS - 1, in_aim_lookX.GetInteger() );
	const int lookYAxis = idMath::ClampInt( 0, MAX_CONTROLLER_AXIS - 1, in_aim_lookY.GetInteger() );
	const int moveXAxis = idMath::ClampInt( 0, MAX_CONTROLLER_AXIS - 1, in_move_moveX.GetInteger() );
	const int moveYAxis = idMath::ClampInt( 0, MAX_CONTROLLER_AXIS - 1, in_move_moveY.GetInteger() );

	// move: linear past a radial dead zone
	float moveX = 0.0f;
	float moveY = 0.0f;
	ApplyRadialDeadZone( axes[ moveXAxis ], axes[ moveYAxis ], in_move_deadZone.GetFloat(), 0.05f, 1.0f, moveX, moveY );
	const bool moving = moveX != 0.0f || moveY != 0.0f;
	if ( moving ) {
		cmd.rightmove = idMath::ClampChar( cmd.rightmove + idMath::Ftoi( moveX * 127.0f ) );
		cmd.forwardmove = idMath::ClampChar( cmd.forwardmove - idMath::Ftoi( moveY * 127.0f ) );
	}

	// look: radial dead zone and curve
	float lookX = 0.0f;
	float lookY = 0.0f;
	ApplyRadialDeadZone( axes[ lookXAxis ], axes[ lookYAxis ], in_aim_deadZone.GetFloat(), in_aim_outerZone.GetFloat(), in_aim_curve.GetFloat(), lookX, lookY );
	const bool looking = lookX != 0.0f || lookY != 0.0f;

	// turn ramp: held at the edge, mostly sideways
	const float lookLength = idMath::Sqrt( lookX * lookX + lookY * lookY );
	if ( lookLength >= 0.98f && idMath::Fabs( lookX ) >= 0.7f ) {
		rampHeld += seconds;
	} else {
		rampHeld = 0.0f;
	}
	float ramp = 0.0f;
	if ( in_aim_rampBoost.GetFloat() > 0.0f && rampHeld > in_aim_rampDelay.GetFloat() ) {
		const float rampTime = in_aim_rampTime.GetFloat();
		ramp = rampTime > 0.0f ? idMath::ClampFloat( 0.0f, 1.0f, ( rampHeld - in_aim_rampDelay.GetFloat() ) / rampTime ) : 1.0f;
		ramp *= in_aim_rampBoost.GetFloat();
	}

	// the per-tick angle speed, scaled by the game for zoom and vehicles (fov / 90 when zoomed)
	float angleSpeedYaw = MS2SEC( USERCMD_MSEC );
	float angleSpeedPitch = MS2SEC( USERCMD_MSEC );
	bool zoomed = false;
	if ( doGameCallback ) {
		gameLocal.GetSensitivity( angleSpeedYaw, angleSpeedPitch );
		idPlayer* local = gameLocal.GetLocalPlayer();
		if ( local != NULL ) {
			float fov;
			idWeapon* weapon = local->GetWeapon();
			zoomed = weapon != NULL && weapon->GetFov( fov );
		}
	}
	if ( zoomed ) {
		angleSpeedYaw *= in_aim_zoomScale.GetFloat();
		angleSpeedPitch *= in_aim_zoomScale.GetFloat();
	}

	float yawRate = -lookX * in_aim_yawSpeed.GetFloat() * ( 1.0f + ramp );
	float pitchRate = lookY * in_aim_pitchSpeed.GetFloat() * ( in_aim_invertPitch.GetBool() ? -1.0f : 1.0f );

	// aim assist, on foot and only while the player aims or moves
	idPlayer* player = doGameCallback ? gameLocal.GetLocalPlayer() : NULL;
	idPlayer* target = NULL;
	float closeness = 0.0f;
	idVec3 targetPoint = vec3_origin;
	if ( in_aim_assist.GetBool() && ( looking || moving ) && OnFoot( player ) && player->GetWeapon() != NULL ) {
		const idVec3 eye = player->GetEyePosition();
		const idVec3 forward = idAngles( viewAngles[ PITCH ], viewAngles[ YAW ], 0.0f ).ToForward();
		target = FindAssistTarget( player, eye, forward, closeness, targetPoint );
		if ( target != NULL ) {
			// friction: slow down on the target
			const float friction = Lerp( 1.0f, in_aim_assistFriction.GetFloat(), closeness );
			yawRate *= friction;
			pitchRate *= friction;

			// adhesion: follow part of the target's motion across the view
			const idAngles bearing = ( targetPoint - eye ).ToAngles();
			const int spawnId = gameLocal.GetSpawnId( target );
			if ( assistTarget == target->entityNumber && assistSpawnId == spawnId && now > assistTime && now - assistTime < 200 ) {
				const float dt = MS2SEC( now - assistTime );
				const float adhesion = in_aim_assistAdhesion.GetFloat() * ( 0.5f + 0.5f * closeness );
				yawRate += idMath::AngleDelta( bearing.yaw, assistBearing.yaw ) / dt * adhesion;
				pitchRate += idMath::AngleDelta( bearing.pitch, assistBearing.pitch ) / dt * adhesion;
			}
			assistTarget = target->entityNumber;
			assistSpawnId = spawnId;
			assistBearing = bearing;
			assistTime = now;
			if ( in_aim_assistDebug.GetBool() ) {
				gameLocal.Printf( "aim assist: %s closeness %.2f friction %.2f\n", target->GetUserInfo().name.c_str(), closeness, friction );
			}
		}
	}
	if ( target == NULL ) {
		assistTarget = -1;
	}

	viewAngles[ ROLL ] = 0.0f;
	viewAngles[ YAW ] += yawRate * angleSpeedYaw;
	viewAngles[ PITCH ] += pitchRate * angleSpeedPitch;

	// gyro: a rate in degrees a second on axes 6 and 7, turned at in_gyro_speed
	const float gyroYaw = axes[ 6 ];
	const float gyroPitch = axes[ 7 ];
	if ( ( gyroYaw != 0.0f || gyroPitch != 0.0f ) && ( !in_gyro_zoomedOnly.GetBool() || zoomed ) ) {
		const float scale = 1000.0f * in_gyro_speed.GetFloat();	// GamepadMapping.h: 1.0 is 1000 degrees a second
		viewAngles[ YAW ] -= gyroYaw * scale * angleSpeedYaw;
		viewAngles[ PITCH ] += gyroPitch * scale * angleSpeedPitch * ( in_gyro_invertPitch.GetBool() ? -1.0f : 1.0f );
	}

	// pad sprint lasts while the stick is pushed forward, and not while firing or zooming
	if ( padSprint ) {
		if ( -moveY < in_padSprintStick.GetFloat() || cmd.buttons.btn.attack || cmd.buttons.btn.altAttack || zoomWanted ) {
			padSprint = false;
		} else {
			cmd.buttons.btn.sprint = true;
		}
	}
}

void sdGamepadControls::OnZoomHold( bool down ) {
	zoomWanted = down;
	zoomPulses = 0;
	zoomNextPulse = 0;
	if ( down ) {
		padSprint = false;
	}
}

void sdGamepadControls::StartQuick( int bank, bool melee, bool down ) {
	quickHeld = down;
	if ( down && quickState == QG_IDLE ) {
		quickBank = bank;
		quickMelee = melee;
		quickTarget = -1;
		quickAlt = false;
		quickReturn = -1;
		quickArm = NULL;
		quickSawAction = false;
		quickState = QG_START;
	}
}

// grenade and melee: resolved in UpdateChord, which waits GAMEPAD_CHORD_MSEC for the other
// (both together are the super, as Destiny's LB + RB)
void sdGamepadControls::OnQuickGrenade( bool down ) {
	PressChord( grenadeButton, down );
}

void sdGamepadControls::OnQuickMelee( bool down ) {
	PressChord( meleeButton, down );
}

void sdGamepadControls::PressChord( chordButton_t& button, bool down ) {
	if ( down && !button.down ) {
		button.down = true;
		button.pending = true;
		button.pressTime = Now();
	} else if ( !down ) {
		button.down = false;
	}
}

/*
================
sdGamepadControls::UpdateChord

Grenade and melee pressed within GAMEPAD_CHORD_MSEC of each other are the item button
(the super): held together for a long press, the second item. Either alone starts its
quick action once the window has passed, or at once when it is let go first.
================
*/
void sdGamepadControls::UpdateChord( int now ) {
	if ( superChord ) {
		grenadeButton.pending = meleeButton.pending = false;
		if ( !grenadeButton.down || !meleeButton.down ) {
			superChord = false;
			OnQuickItem( false );
		}
		return;
	}
	if ( grenadeButton.pending && meleeButton.pending &&
		idMath::Abs( grenadeButton.pressTime - meleeButton.pressTime ) <= GAMEPAD_CHORD_MSEC ) {
		grenadeButton.pending = meleeButton.pending = false;
		superChord = true;
		OnQuickItem( true );
		return;
	}
	if ( grenadeButton.pending && ( !grenadeButton.down || now - grenadeButton.pressTime >= GAMEPAD_CHORD_MSEC ) ) {
		grenadeButton.pending = false;
		if ( quickState == QG_IDLE ) {
			StartQuick( GAMEPAD_GRENADE_BANK, false, true );
		}
	}
	if ( meleeButton.pending && ( !meleeButton.down || now - meleeButton.pressTime >= GAMEPAD_CHORD_MSEC ) ) {
		meleeButton.pending = false;
		if ( quickState == QG_IDLE ) {
			StartQuick( GAMEPAD_MELEE_BANK, true, true );
		}
	}
	// the grenade cooks while its button is held
	if ( quickBank == GAMEPAD_GRENADE_BANK && !grenadeButton.down ) {
		quickHeld = false;
	}
}

void sdGamepadControls::OnHeavy( bool down ) {
	if ( down ) {
		heavyPending = true;
	}
}

// the class ability: resolved in UpdateClassAbility, which knows the class
void sdGamepadControls::OnQuickTool( bool down ) {
	if ( down && !toolHeld ) {
		toolPressPending = true;
	}
	toolHeld = down;
	if ( !down && quickBank == GAMEPAD_TOOLS_BANK ) {
		quickHeld = false;
	}
}

// the item: a press uses the first, a long press the second (UpdateItemButton)
void sdGamepadControls::OnQuickItem( bool down ) {
	if ( down ) {
		if ( !itemHeld ) {
			itemHeld = true;
			itemActed = false;
			itemStart = Now();
		}
	} else {
		if ( itemHeld && !itemActed ) {
			itemShortPending = true;
		}
		itemHeld = false;
		if ( quickBank == GAMEPAD_ITEMS_BANK ) {
			quickHeld = false;
		}
	}
}

/*
================
sdGamepadControls::BankWeaponAt

The nth weapon of a bank the player can equip, in the order the class's kit lists them
(Medic: the health pack, then the supply-crate marker), or -1
================
*/
int sdGamepadControls::BankWeaponAt( idPlayer* player, int bank, int n ) {
	sdInventory& inventory = player->GetInventory();
	if ( bank < 0 || bank >= inventory.GetMaxBanks() ) {
		return -1;
	}
	const int slot = inventory.GetSlotForBank( bank );
	if ( slot == -1 ) {
		return -1;
	}
	for ( int i = 0; i < inventory.GetNumItems(); i++ ) {
		const sdDeclInvItem* item = inventory.GetItem( i );
		if ( item == NULL || !item->UsesSlot( slot ) || !inventory.CanEquip( i, true ) ) {
			continue;
		}
		if ( n-- == 0 ) {
			return i;
		}
	}
	return -1;
}

// the first weapon of a bank whose inventory name has this text, or -1
int sdGamepadControls::BankWeaponNamed( idPlayer* player, int bank, const char* text ) {
	for ( int n = 0; ; n++ ) {
		const int index = BankWeaponAt( player, bank, n );
		if ( index == -1 ) {
			return -1;
		}
		const sdDeclInvItem* item = player->GetInventory().GetItem( index );
		if ( item != NULL && idStr::FindText( item->GetName(), text, false ) != -1 ) {
			return index;
		}
	}
}

bool sdGamepadControls::WeaponNamed( idPlayer* player, int index, const char* text ) {
	const sdDeclInvItem* item = player->GetInventory().GetItem( index );
	return item != NULL && idStr::FindText( item->GetName(), text, false ) != -1;
}

// the player has a third-eye camera out in the world (items/third_eye.script FindCamera)
bool sdGamepadControls::ThirdEyePlaced( idPlayer* player ) {
	for ( int i = 0; i < player->GetBinSize(); i++ ) {
		idEntity* other = player->GetBinEntity( i );
		if ( other != NULL && idStr::FindText( other->GetEntityDefName(), "third_eye", false ) != -1 ) {
			return true;
		}
	}
	return false;
}

/*
================
sdGamepadControls::ArmableInView

A mine or charge of the player's team under the crosshair, in reach, that still needs
arming (a mine's script keeps LM_NONE, 0, in "mode" until it is armed), or NULL
================
*/
idEntity* sdGamepadControls::ArmableInView( idPlayer* player ) {
	const sdCrosshairInfo& info = player->GetCrosshairInfo( true );
	idEntity* other = info.IsValid() ? info.GetEntity() : NULL;
	if ( other == NULL || other->GetGameTeam() == NULL || other->GetGameTeam() != player->GetGameTeam() ) {
		return NULL;
	}
	if ( ( other->GetPhysics()->GetOrigin() - player->GetPhysics()->GetOrigin() ).LengthSqr() > Square( 160.0f ) ) {
		return NULL;
	}
	const char* defName = other->GetEntityDefName();
	if ( idStr::FindText( defName, "landmine", false ) != -1 ) {
		idScriptObject* object = other->GetScriptObject();
		const float* mode = object != NULL ? reinterpret_cast< const float* >( object->GetVariable( "mode", ev_float ) ) : NULL;
		return mode == NULL || *mode == 0.0f ? other : NULL;
	}
	if ( idStr::FindText( defName, "charge", false ) != -1 ) {
		return other;
	}
	return NULL;
}

// the class's arming tool: the pliers (not the hacking pair) or the Aggressor's spike
int sdGamepadControls::ArmingTool( idPlayer* player ) {
	for ( int n = 0; ; n++ ) {
		const int index = BankWeaponAt( player, GAMEPAD_TOOLS_BANK, n );
		if ( index == -1 ) {
			return -1;
		}
		if ( ( WeaponNamed( player, index, "pliers" ) && !WeaponNamed( player, index, "hack" ) ) ||
			WeaponNamed( player, index, "spike/charge" ) ) {
			return index;
		}
	}
}

// a quick action on this weapon, pressing fire (or alt-fire) once or while the button is held
void sdGamepadControls::StartQuickOn( int bank, int weaponIndex, bool alt, bool held, int returnTo ) {
	quickBank = bank;
	quickMelee = !held;
	quickHeld = held;
	quickTarget = weaponIndex;
	quickAlt = alt;
	quickReturn = returnTo;
	quickArm = NULL;
	quickSawAction = false;
	quickState = QG_START;
}

/*
================
sdGamepadControls::UpdateItemButton

The item button (Destiny's super):
  - a press uses the class's first item (health pack, ammo pack, third eye, force
    shield, flyer drone, charge, mine), a long press the second (supply-crate marker,
    airstrike marker, smoke, teleporter), held while the button is; a class with one
    item uses it either way;
  - the third eye, once placed: a press looks through it (the item stays in hand while
    the view is on); flying the flyer drone, likewise;
  - looking through a camera or flying the drone: a press leaves the view (recalls the
    drone), a long press detonates it; then back to the gun.
================
*/
void sdGamepadControls::UpdateItemButton( idPlayer* player, int now ) {
	bool press = false, longPress = false;
	if ( itemShortPending ) {
		itemShortPending = false;
		press = true;
	} else if ( itemHeld && !itemActed && now - itemStart >= in_padHoldTime.GetInteger() ) {
		itemActed = true;
		press = longPress = true;
	}
	if ( !press || quickState != QG_IDLE ) {
		return;
	}
	sdInventory& inventory = player->GetInventory();

	if ( stickyWeapon >= 0 && inventory.GetCurrentWeapon() == stickyWeapon && player->GetRemoteCamera() != NULL ) {
		// in the view: a press leaves it (alt-fire), a long press detonates (fire)
		const int back = stickyReturn;
		stickyWeapon = -1;
		StartQuickOn( GAMEPAD_ITEMS_BANK, inventory.GetCurrentWeapon(), !longPress, false, back );
		return;
	}

	// facing the team's mine or charge that still needs arming: the pliers arm it
	idEntity* armable = ArmableInView( player );
	const int armingTool = armable != NULL ? ArmingTool( player ) : -1;
	if ( armingTool != -1 ) {
		StartQuickOn( GAMEPAD_TOOLS_BANK, armingTool, false, true, -1 );
		quickArm = armable;
		return;
	}

	int target = BankWeaponAt( player, GAMEPAD_ITEMS_BANK, longPress ? 1 : 0 );
	if ( target == -1 ) {
		target = BankWeaponAt( player, GAMEPAD_ITEMS_BANK, 0 );
	}
	if ( target == -1 ) {
		if ( in_quickDebug.GetBool() ) {
			gameLocal.Printf( "item: none\n" );
		}
		return;
	}
	// a placed third eye: alt-fire looks through it
	const bool view = WeaponNamed( player, target, "third_eye" ) && ThirdEyePlaced( player );
	StartQuickOn( GAMEPAD_ITEMS_BANK, target, view, longPress && !view, -1 );
}

/*
================
sdGamepadControls::UpdateClassAbility

The class ability button: a class with a deploy tool (Engineer, Field Ops, Covert Ops,
Constructor, Oppressor, Infiltrator) toggles it: a press brings it out, and it stays
until the thing is placed or the button is pressed again, then the gun comes back. The
others hold their tool while the button is held (Soldier and Aggressor arm, Medic
revives, Technician), as before.
================
*/
void sdGamepadControls::UpdateClassAbility( idPlayer* player, int now ) {
	sdInventory& inventory = player->GetInventory();
	sdDeployMenu* menu = gameLocal.localPlayerProperties.GetDeployMenu();
	const bool placing = menu != NULL && menu->Active();

	if ( deployWeapon >= 0 ) {
		if ( inventory.GetCurrentWeapon() != deployWeapon && !inventory.IsSwitchActive() && now - deployStart > 1500 ) {
			deployWeapon = -1;	// something else took it away
		} else if ( placing ) {
			deploySawPlacing = true;
		} else if ( deploySawPlacing ) {
			// placed (or the placement was cancelled): back to the gun
			const int back = deployReturn;
			deployWeapon = -1;
			QuickSwitchTo( player, back );
		}
	}

	if ( !toolPressPending ) {
		return;
	}
	toolPressPending = false;
	if ( quickState != QG_IDLE ) {
		return;
	}
	const int deployTool = BankWeaponNamed( player, GAMEPAD_TOOLS_BANK, "deploytool" );
	if ( deployTool == -1 ) {
		if ( toolHeld ) {
			StartQuick( GAMEPAD_TOOLS_BANK, false, true );
		}
		return;
	}
	if ( deployWeapon >= 0 && inventory.GetCurrentWeapon() == deployWeapon ) {
		const int back = deployReturn;
		deployWeapon = -1;
		QuickSwitchTo( player, back );
		return;
	}
	deployReturn = inventory.GetCurrentWeapon();
	deployWeapon = deployTool;
	deployStart = now;
	deploySawPlacing = false;
	QuickSwitchTo( player, deployTool );
}

/*
================
sdGamepadControls::UpdateSticky

An item left in hand for a remote view (the third eye, the flyer drone) goes back to
the gun once the view ends by itself (the camera destroyed, the drone shot down)
================
*/
void sdGamepadControls::UpdateSticky( idPlayer* player, int now ) {
	if ( stickyWeapon < 0 || quickState != QG_IDLE ) {
		return;
	}
	sdInventory& inventory = player->GetInventory();
	if ( inventory.GetCurrentWeapon() != stickyWeapon && !inventory.IsSwitchActive() ) {
		stickyWeapon = -1;	// the player moved on
		return;
	}
	if ( player->GetRemoteCamera() == NULL && now - stickyStart > 1500 ) {
		const int back = stickyReturn;
		stickyWeapon = -1;
		QuickSwitchTo( player, back );
	}
}

/*
================
sdGamepadControls::UpdateReturnToGun

in_gunsOnly: the player is only on a gun unless a button holds something else. Retail
still puts other things in hand by itself (using a damaged vehicle brings out the
pliers, an objective the hacking tool, running out of grenades picks a weapon); once
nothing holds it (no quick action, deploy toggle, remote view or heavy; fire and use
let go) and it has sat ready for GAMEPAD_RETURN_MSEC, the last gun comes back.
================
*/
void sdGamepadControls::UpdateReturnToGun( idPlayer* player, usercmd_t& cmd, int now ) {
	sdInventory& inventory = player->GetInventory();
	const int bank = inventory.GetCurrentSlot();
	if ( bank == GAMEPAD_PRIMARY_BANK || bank == GAMEPAD_SIDEARM_BANK ) {
		lastGun = inventory.GetCurrentWeapon();
		offGunSince = 0;
		return;
	}
	idWeapon* weapon = player->GetWeapon();
	const bool held = !in_gunsOnly.GetBool() || quickState != QG_IDLE || deployWeapon >= 0 || stickyWeapon >= 0 ||
		bank == GAMEPAD_BINOCS_BANK || bank == -1 || player->GetRemoteCamera() != NULL ||
		cmd.buttons.btn.attack || cmd.buttons.btn.altAttack || cmd.buttons.btn.activate ||
		inventory.IsSwitchActive() || weapon == NULL || !weapon->IsReady() || lastGun < 0;
	if ( held ) {
		offGunSince = 0;
		return;
	}
	if ( offGunSince == 0 ) {
		offGunSince = now;
		return;
	}
	if ( now - offGunSince >= GAMEPAD_RETURN_MSEC ) {
		offGunSince = 0;
		if ( in_quickDebug.GetBool() ) {
			gameLocal.Printf( "back to the gun from bank %d\n", bank );
		}
		QuickSwitchTo( player, lastGun );
	}
}

// Y held: the binoculars, Destiny's heavy weapon (Field Ops and Oppressor aim their
// artillery with them); it stays in hand like a gun until the next swap
void sdGamepadControls::SelectHeavy( idPlayer* player ) {
	const int binoculars = BankWeaponAt( player, GAMEPAD_BINOCS_BANK, 0 );
	if ( binoculars != -1 && quickState == QG_IDLE ) {
		QuickSwitchTo( player, binoculars );
	}
}

void sdGamepadControls::SetQuickState( int state, const char* why ) {
	if ( in_quickDebug.GetBool() && state != quickState ) {
		static const char* const names[] = { "idle", "start", "switching", "firing", "done" };
		gameLocal.Printf( "quick bank %d: %s -> %s (%s)\n", quickBank, names[ quickState ], names[ state ], why );
	}
	quickState = static_cast< quickState_t >( state );
}

/*
================
sdGamepadControls::QuickSwitchTo

Puts this inventory weapon in the player's hands at once: no weapon menu (the retail
switch waits for it, idPlayer::AcceptWeaponSwitch) and, where the game runs here, no
put-away animation either (idWeapon::InstantSwitch, as the scripts' setWeapon does).
================
*/
void sdGamepadControls::QuickSwitchTo( idPlayer* player, int weaponIndex ) {
	sdInventory& inventory = player->GetInventory();
	if ( weaponIndex < 0 || weaponIndex == inventory.GetCurrentWeapon() ) {
		return;
	}
	idWeapon* held = player->GetWeapon();
	inventory.SetSwitchingWeapon( weaponIndex );
	player->SwitchWeapon();
	player->ShowWeaponMenu( sdWeaponSelectionMenu::AT_DISABLED );
	if ( !gameLocal.isClient && held != NULL && held->IsReady() ) {
		held->InstantSwitch();
	}
}

/*
================
sdGamepadControls::BankWeapon

The weapon a quick action takes from a bank: its first one with ammo, or -1
================
*/
int sdGamepadControls::BankWeapon( idPlayer* player, int bank ) {
	sdInventory& inventory = player->GetInventory();
	if ( bank < 0 || bank >= inventory.GetMaxBanks() || inventory.GetSlotForBank( bank ) == -1 ) {
		return -1;
	}
	if ( inventory.GetCurrentSlot() == bank ) {
		return inventory.GetCurrentWeapon();
	}
	bool looped = false;
	return inventory.CycleWeaponByPosition( bank, true, looped, false, true );
}

/*
================
sdGamepadControls::SelectGun

in_gunsOnly: the weapon keys and next/previous weapon (idPlayer::SelectWeapon,
NextWeapon, PrevWeapon) only reach the sidearm and the primary. Returns false for
a bank that is not a gun, which the caller then ignores; true when it handled the
request (bank -1: the other gun, as Destiny's swap).
================
*/
bool sdGamepadControls::SelectGun( idPlayer* player, int bank ) {
	if ( !in_gunsOnly.GetBool() ) {
		return false;
	}
	if ( bank == -1 ) {
		const int current = player->GetInventory().GetCurrentSlot();
		bank = current == GAMEPAD_PRIMARY_BANK ? GAMEPAD_SIDEARM_BANK : GAMEPAD_PRIMARY_BANK;
		if ( BankWeapon( player, bank ) == -1 ) {
			bank = bank == GAMEPAD_PRIMARY_BANK ? GAMEPAD_SIDEARM_BANK : GAMEPAD_PRIMARY_BANK;
		}
	}
	if ( bank != GAMEPAD_PRIMARY_BANK && bank != GAMEPAD_SIDEARM_BANK ) {
		return true;	// not a gun: nothing happens
	}
	if ( quickState != QG_IDLE ) {
		quickPrevWeapon = BankWeapon( player, bank );	// a quick action comes back to it
		return true;
	}
	QuickSwitchTo( player, BankWeapon( player, bank ) );
	return true;
}

/*
================
sdGamepadControls::DestinyCommandFor

The action that does what a retail command did, for the key names tooltips and GUIs
show ("press [key] to throw a grenade"): retail's weapon banks are reached by the
Destiny style buttons now, and alt-fire by the aim button. NULL when it is unchanged.
================
*/
const char* sdGamepadControls::DestinyCommandFor( const char* command ) {
	if ( command == NULL || !in_gunsOnly.GetBool() ) {
		return NULL;
	}
	static const char* const map[][ 2 ] = {
		{ "_weapon0",					"_quickmelee" },
		{ "_weapon1",					"_weapon1" },
		{ "_weapon2",					"_weapon2" },
		{ "_weapon3",					"_quickgrenade" },
		{ "_weapon4",					"_quicktool" },
		{ "_weapon5",					"_quickitem" },
		{ "_weapon6",					"_heavy" },
		{ "useweapon weapon_binocs",	"_heavy" },
		{ "useweapon weapon_tool1",		"_quicktool" },
		{ "useweapon weapon_tool2",		"_quicktool" },
		{ "useweapon weapon_item1",		"_quickitem" },
		{ "useweapon weapon_item2",		"_quickitem" },
		{ "_altattack",					"_zoomhold" },
		{ "_sprint",					"_padsprint" },
		{ "_usevehicle",				"_interact" },
	};
	for ( int i = 0; i < static_cast< int >( sizeof( map ) / sizeof( map[ 0 ] ) ); i++ ) {
		if ( idStr::Icmp( command, map[ i ][ 0 ] ) == 0 ) {
			return map[ i ][ 1 ];
		}
	}
	return NULL;
}

bool sdGamepadControls::GunsOnly( void ) const {
	return in_gunsOnly.GetBool();
}

void sdGamepadControls::OnPadSprint( bool down ) {
	padSprintHeld = down;
	if ( down ) {
		padSprint = !padSprint;
	}
}

void sdGamepadControls::OnTapHold( tapHoldMode_t mode, bool down ) {
	const int now = Now();
	if ( down ) {
		tapHoldMode = mode;
		reloadUseHeld = true;
		reloadUseStart = now;
		reloadUseActed = false;
	} else if ( mode == tapHoldMode ) {
		if ( reloadUseHeld && !reloadUseActed ) {
			reloadPending = true;	// a tap
		}
		reloadUseHeld = false;
	}
}

void sdGamepadControls::OnReloadUse( bool down ) {
	OnTapHold( TH_RELOAD_USE, down );
}

void sdGamepadControls::OnInteract( bool down ) {
	OnTapHold( TH_INTERACT, down );
}

void sdGamepadControls::OnPadFire( bool down ) {
	padFireHeld = down;
}

void sdGamepadControls::OnCrouchProne( bool down ) {
	const int now = Now();
	if ( down ) {
		crouchHeld = true;
		crouchActed = false;
		crouchStart = now;
	} else {
		if ( crouchHeld && !crouchActed ) {
			crouchStandPending = true;	// a tap: resolved in UsercommandCallback, which knows the stance
		} else if ( crouchHeld && crouchActed ) {
			OnQuickTool( false );		// the class ability's hold ends
		}
		crouchHeld = false;
	}
}

void sdGamepadControls::OnSwapWeapon( bool down ) {
	if ( down ) {
		swapHeld = true;
		swapActed = false;
		swapStart = Now();
	} else {
		if ( swapHeld && !swapActed ) {
			swapActed = true;
			swapPending = true;
		}
		swapHeld = false;
	}
}

void sdGamepadControls::NoteTriggerDriving( void ) {
	triggerDrivingTime = Now();
}

bool sdGamepadControls::TriggerDriving( void ) const {
	return triggerDrivingTime != 0 && Now() - triggerDrivingTime < 250;
}

/*
================
sdGamepadControls::UpdateTapHold

_reloaduse on foot: a tap reloads; held, use is held and the vehicle in front is entered.
_interact on foot: use is held from the press, and the vehicle in front is entered.
Either in a vehicle: a tap is a short use press (switch seat, launch decoys); held, the
player gets out.
================
*/
void sdGamepadControls::UpdateTapHold( idPlayer* player, usercmd_t& cmd, int now, bool onFoot ) {
	const int holdTime = in_padHoldTime.GetInteger();
	if ( reloadPending ) {
		reloadPending = false;
		if ( !onFoot ) {
			activatePulseUntil = now + GAMEPAD_PRESS_MSEC;
		} else if ( tapHoldMode == TH_RELOAD_USE ) {
			player->PerformImpulse( UCI_RELOAD );
		} else {
			activatePulseUntil = now + GAMEPAD_PRESS_MSEC;	// _interact: a short press is a short use
		}
	}
	if ( reloadUseHeld ) {
		const bool held = now - reloadUseStart >= holdTime;
		if ( onFoot && tapHoldMode == TH_INTERACT ) {
			// use held from the press, and the vehicle in front entered; the release is
			// not also a tap (in the vehicle it would switch seat)
			cmd.buttons.btn.activate = true;
			reloadUseActed = true;
			if ( !usedVehicleThisPress ) {
				usedVehicleThisPress = true;
				if ( in_quickDebug.GetBool() ) {
					gameLocal.Printf( "interact: use (enter a vehicle)\n" );
				}
				player->PerformImpulse( UCI_USE_VEHICLE );
			}
		} else if ( held ) {
			if ( onFoot ) {
				cmd.buttons.btn.activate = true;
			}
			if ( !reloadUseActed ) {
				reloadUseActed = true;
				player->PerformImpulse( UCI_USE_VEHICLE );
			}
		}
	} else {
		usedVehicleThisPress = false;
	}
	if ( now < activatePulseUntil ) {
		cmd.buttons.btn.activate = true;
	}
}

void sdGamepadControls::UsercommandCallback( idPlayer* player, usercmd_t& cmd ) {
	const int now = Now();
	const bool onFoot = OnFoot( player );
	idEntity* proxy = player->GetProxyEntity();

	UpdateTapHold( player, cmd, now, onFoot );

	// right trigger: fire, unless this player drives a ground vehicle with the triggers
	const bool drivingWithTriggers = proxy != NULL && TriggerDriving();
	if ( padFireHeld && !drivingWithTriggers ) {
		cmd.buttons.btn.attack = true;
	}

	if ( !onFoot ) {
		// in a vehicle the left trigger is alt-fire held (brake for a trigger driver) and the
		// sprint click is boost held
		zoomPulseUntil = 0;
		padSprint = false;
		crouchLatched = false;
		crouchStandPending = false;
		quickState = QG_IDLE;
		itemShortPending = false;
		itemActed = true;
		toolPressPending = false;
		grenadeButton.pending = meleeButton.pending = false;
		superChord = false;
		heavyPending = false;
		deployWeapon = -1;
		stickyWeapon = -1;
		if ( proxy != NULL ) {
			if ( zoomWanted && !drivingWithTriggers ) {
				cmd.buttons.btn.altAttack = true;
			}
			if ( padSprintHeld ) {
				cmd.buttons.btn.sprint = true;
			}
		}
		swapPending = false;
		return;
	}

	// Y: a tap is the next weapon, a hold the items bank
	if ( swapPending ) {
		swapPending = false;
		player->PerformImpulse( UCI_WEAPNEXT );
	}
	if ( swapHeld && !swapActed && now - swapStart >= in_padHoldTime.GetInteger() ) {
		swapActed = true;
		SelectHeavy( player );
	}

	// B: crouch while held, a tap toggles the crouch latch (and stands up from prone)
	const bool prone = player->GetPlayerPhysics().IsProne();
	if ( crouchStandPending ) {
		crouchStandPending = false;
		if ( prone ) {
			player->PerformImpulse( UCI_PRONE );	// up from prone
			crouchLatched = false;
		} else {
			crouchLatched = !crouchLatched;
		}
	}
	// B held: the class ability (Destiny's hold B), as the class ability button
	if ( crouchHeld && !crouchActed && now - crouchStart >= in_padHoldTime.GetInteger() + 100 ) {
		crouchActed = true;
		crouchLatched = false;
		OnQuickTool( true );
		UpdateClassAbility( player, now );
	}
	if ( cmd.upmove > 0 || padSprint ) {
		crouchLatched = false;	// a jump or a sprint stands up
	}
	if ( !prone && ( crouchLatched || ( crouchHeld && !crouchActed ) ) && cmd.upmove <= 0 ) {
		cmd.upmove = -127;
	}

	// _padsprint from the keyboard lasts while the player moves forward (the pad's stick is
	// checked in PlayerControllerMove, which runs first and may already have stopped it)
	if ( padSprint ) {
		if ( cmd.forwardmove <= 0 || cmd.buttons.btn.attack || cmd.buttons.btn.altAttack || zoomWanted ) {
			padSprint = false;
		} else {
			cmd.buttons.btn.sprint = true;
		}
	}

	UpdateChord( now );
	if ( heavyPending ) {
		heavyPending = false;
		SelectHeavy( player );
	}
	UpdateItemButton( player, now );
	UpdateClassAbility( player, now );
	UpdateQuick( player, cmd, now );
	UpdateSticky( player, now );
	UpdateReturnToGun( player, cmd, now );
	UpdateZoomHold( player, cmd, now );
}

/*
================
sdGamepadControls::UpdateZoomHold

The scripts toggle sights on an alt-fire press (weapons/base.script CheckIronSights), so
a press is sent whenever the wanted and the actual state differ. A press the weapon did
not act on (during a reload, say) is retried after GAMEPAD_ZOOM_RETRY_MSEC, at most three
times for one hold or release.
================
*/
void sdGamepadControls::UpdateZoomHold( idPlayer* player, usercmd_t& cmd, int now ) {
	if ( now < zoomPulseUntil ) {
		cmd.buttons.btn.altAttack = true;
		return;
	}
	if ( quickState != QG_IDLE ) {
		return;
	}
	idWeapon* weapon = player->GetWeapon();
	if ( !WeaponHasSights( weapon ) ) {
		// tools and items: the aim button is their alt-fire, held (rotate a deployable,
		// look through the third eye, throw underhand)
		if ( zoomWanted && weapon != NULL && player->GetInventory().GetCurrentSlot() != GAMEPAD_PRIMARY_BANK &&
			player->GetInventory().GetCurrentSlot() != GAMEPAD_SIDEARM_BANK ) {
			cmd.buttons.btn.altAttack = true;
		}
		return;
	}
	const bool zoomed = WeaponIsZoomed( player, weapon );
	if ( zoomed == zoomWanted || now < zoomNextPulse || zoomPulses >= 3 ) {
		return;
	}
	zoomPulseUntil = now + GAMEPAD_PRESS_MSEC;
	zoomNextPulse = now + GAMEPAD_ZOOM_RETRY_MSEC;
	zoomPulses++;
	cmd.buttons.btn.altAttack = true;
}

/*
================
sdGamepadControls::UpdateQuick

Quick grenade and quick melee: select the bank, wait until the weapon is up, hold fire
(a grenade cooks while the button is held and goes on release; the melee weapon swings
once), then switch back to the weapon held before.
================
*/
void sdGamepadControls::UpdateQuick( idPlayer* player, usercmd_t& cmd, int now ) {
	sdInventory& inventory = player->GetInventory();

	switch ( quickState ) {
		case QG_IDLE:
			return;

		case QG_START: {
			const int target = quickTarget >= 0 ? quickTarget : BankWeapon( player, quickBank );
			if ( target == -1 ) {
				SetQuickState( QG_IDLE, "nothing usable in that bank" );
				return;
			}
			quickPrevWeapon = inventory.GetCurrentWeapon();
			if ( target == quickPrevWeapon ) {
				quickPrevWeapon = quickReturn;	// already in hand: just use it
			} else {
				QuickSwitchTo( player, target );
			}
			quickDeadline = now + GAMEPAD_QUICK_SWITCH_MSEC;
			SetQuickState( QG_SWITCHING, "selected" );
			// fall through: it may already be up
		}

		case QG_SWITCHING: {
			// GetCurrentSlot is the bank of the weapon being brought up
			idWeapon* weapon = player->GetWeapon();
			const int wanted = quickTarget >= 0 ? quickTarget : inventory.GetCurrentWeapon();
			const bool inHand = inventory.GetCurrentSlot() == quickBank && !inventory.IsSwitchActive() &&
				inventory.GetCurrentWeapon() == wanted && wanted >= 0 && weapon != NULL && weapon->IsReady();
			if ( inHand ) {
				SetQuickState( QG_FIRING, "up" );
				quickFireStart = now;
			} else if ( now > quickDeadline ) {
				SetQuickState( QG_DONE, inventory.GetCurrentSlot() == quickBank ? "never ready" : "never selected" );
				quickDeadline = now;
				return;
			} else {
				return;
			}
			// fall through
		}

		case QG_FIRING: {
			{
				idWeapon* firing = player->GetWeapon();
				if ( firing != NULL && idStr::FindText( firing->GetScriptState(), "Idle", false ) == -1 ) {
					quickSawAction = true;	// the throw or swing started
				}
			}
			if ( quickAlt ) {
				cmd.buttons.btn.altAttack = true;
			} else {
				cmd.buttons.btn.attack = true;
			}
			const bool minimumHeld = now - quickFireStart >= GAMEPAD_QUICK_MIN_FIRE_MSEC;
			if ( quickArm.IsValid() || quickArmWas ) {
				// arming: fire stays held until the mine is armed, the crosshair leaves it or
				// GAMEPAD_ARM_MSEC passes (a charge: until the button is let go)
				quickArmWas = true;
				idEntity* armable = ArmableInView( player );
				const bool stillArming = quickArm.IsValid() && armable == quickArm.GetEntity() &&
					now - quickFireStart < GAMEPAD_ARM_MSEC &&
					( quickHeld || idStr::FindText( armable->GetEntityDefName(), "landmine", false ) != -1 );
				if ( !stillArming && minimumHeld ) {
					quickArmWas = false;
					quickArm = NULL;
					SetQuickState( QG_DONE, "armed" );
					quickDeadline = now + 2000;
					cmd.buttons.btn.attack = false;
				}
				return;
			}
			if ( minimumHeld && ( quickMelee || !quickHeld ) ) {
				SetQuickState( QG_DONE, quickMelee ? "swung" : "released" );
				quickDeadline = now + 3000;
				cmd.buttons.btn.attack = false;
				cmd.buttons.btn.altAttack = false;
			}
			return;
		}

		case QG_DONE: {
			// back once the throw or swing is over and the weapon is ready again
			idWeapon* weapon = player->GetWeapon();
			// the weapon stays "ready" (WP_READY) through its fire state, so the throw or
			// swing is over once its script is back in an idle state; a camera view comes
			// on 0.4 s after alt-fire (items/third_eye.script), which no state change shows
			const char* scriptState = weapon != NULL ? weapon->GetScriptState() : "";
			if ( idStr::FindText( scriptState, "Idle", false ) == -1 ) {
				quickSawAction = true;
			}
			const bool idle = weapon != NULL && weapon->IsReady() && idStr::FindText( scriptState, "Idle", false ) != -1;
			const bool settled = quickAlt ? now - quickFireStart > 900 : ( quickSawAction && now - quickFireStart > 300 );
			const bool done = inventory.GetCurrentSlot() != quickBank || ( idle && settled );
			if ( !done && now <= quickDeadline ) {
				return;
			}
			if ( player->GetRemoteCamera() != NULL && inventory.GetCurrentSlot() == quickBank ) {
				// looking through the third eye or flying the drone: the item stays in hand
				SetQuickState( QG_IDLE, "remote view" );
				stickyWeapon = inventory.GetCurrentWeapon();
				stickyReturn = quickPrevWeapon;
				stickyStart = now;
				quickPrevWeapon = -1;
				return;
			}
			const int slotNow = inventory.GetCurrentSlot();
			if ( slotNow != quickBank && slotNow != GAMEPAD_PRIMARY_BANK && slotNow != GAMEPAD_SIDEARM_BANK &&
				!( slotNow == GAMEPAD_TOOLS_BANK && ArmingTool( player ) == inventory.GetCurrentWeapon() ) ) {
				// the item's script put the next tool in hand (a placed mine or charge
				// selects the pliers to arm it): leave it there; UpdateReturnToGun brings
				// the gun back once fire and use have been let go a while
				SetQuickState( QG_IDLE, "the script picked the next tool" );
				if ( quickPrevWeapon >= 0 ) {
					lastGun = quickPrevWeapon;
				}
				quickPrevWeapon = -1;
				return;
			}
			if ( in_quickDebug.GetBool() ) {
				gameLocal.Printf( "quick: weapon state '%s', saw action %d\n", scriptState, quickSawAction ? 1 : 0 );
			}
			SetQuickState( QG_IDLE, "back" );
			if ( quickPrevWeapon >= 0 ) {
				QuickSwitchTo( player, quickPrevWeapon );
			}
			quickPrevWeapon = -1;
			return;
		}
	}
}

/*
================
GamepadSelfTest_f

gamepadSelfTest: checks the stick math on any build, without a pad.
================
*/
static void GamepadSelfTest_f( const idCmdArgs& args ) {
	int failures = 0;
	float x, y;
	sdGamepadControls::ApplyRadialDeadZone( 0.07f, 0.07f, 0.1f, 0.0f, 2.0f, x, y );
	if ( x != 0.0f || y != 0.0f ) { gameLocal.Printf( "FAIL: inside the radial dead zone\n" ); failures++; }
	sdGamepadControls::ApplyRadialDeadZone( 1.0f, 0.0f, 0.1f, 0.03f, 2.2f, x, y );
	if ( idMath::Fabs( x - 1.0f ) > 1e-4f || y != 0.0f ) { gameLocal.Printf( "FAIL: full stick\n" ); failures++; }
	sdGamepadControls::ApplyRadialDeadZone( 0.707f, 0.707f, 0.1f, 0.03f, 1.0f, x, y );
	if ( idMath::Fabs( idMath::Sqrt( x * x + y * y ) - 1.0f ) > 1e-3f ) { gameLocal.Printf( "FAIL: diagonal\n" ); failures++; }
	sdGamepadControls::ApplyRadialDeadZone( 0.55f, 0.0f, 0.1f, 0.0f, 2.0f, x, y );
	if ( idMath::Fabs( x - 0.25f ) > 1e-3f ) { gameLocal.Printf( "FAIL: curve (%f)\n", x ); failures++; }
	if ( idMath::Fabs( sdGamepadControls::AngleBetween( idVec3( 1, 0, 0 ), idVec3( 0, 1, 0 ) ) - 90.0f ) > 1e-3f ) { gameLocal.Printf( "FAIL: angle\n" ); failures++; }
	gameLocal.Printf( "gamepadSelfTest: %s\n", failures == 0 ? "all checks passed" : "FAILED" );
}

void Gamepad_RegisterCommands( void ) {
	cmdSystem->AddCommand( "gamepadSelfTest", GamepadSelfTest_f, CMD_FL_GAME, "checks the gamepad stick math" );
}
