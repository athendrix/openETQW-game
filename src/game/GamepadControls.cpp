// SPDX-License-Identifier: MIT

#include "precompiled.h"
#pragma hdrstop

#include "GamepadControls.h"
#include "Player.h"
#include "Weapon.h"
#include "roles/Inventory.h"
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
idCVar in_padSprintStick( "in_padSprintStick", "0.3", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT | CVAR_PROFILE, "_padsprint: sprint stops when the move stick's forward push drops under this", 0.0f, 0.95f );

namespace {

// the grenade bank, as the bots use it (botai/BotThreadData.h GRENADE_SLOT); _weapon3
const int GAMEPAD_GRENADE_BANK = 3;

// how long a synthetic alt-fire or fire press lasts: two usercmd ticks, so the server sees it
const int GAMEPAD_PRESS_MSEC = 2 * USERCMD_MSEC + 10;

// after an alt-fire press, how long to wait for the weapon to change state before trying again
const int GAMEPAD_ZOOM_RETRY_MSEC = 800;

// the melee bank (_weapon0)
const int GAMEPAD_MELEE_BANK = 0;

// quick grenade and melee hold fire at least this long (a grenade's GRENADE_MINRELEASETIME is 0.1 s)
const int GAMEPAD_QUICK_MIN_FIRE_MSEC = 150;

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
	padSprint = false;
	padSprintHeld = false;
	reloadUseHeld = false;
	reloadUseActed = false;
	reloadPending = false;
	reloadUseStart = 0;
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
		quickState = QG_START;
	}
}

void sdGamepadControls::OnQuickGrenade( bool down ) {
	if ( quickState == QG_IDLE || !quickMelee ) {
		StartQuick( GAMEPAD_GRENADE_BANK, false, down );
	}
}

void sdGamepadControls::OnQuickMelee( bool down ) {
	if ( quickState == QG_IDLE || quickMelee ) {
		StartQuick( GAMEPAD_MELEE_BANK, true, down );
	}
}

void sdGamepadControls::OnPadSprint( bool down ) {
	padSprintHeld = down;
	if ( down ) {
		padSprint = !padSprint;
	}
}

void sdGamepadControls::OnReloadUse( bool down ) {
	const int now = Now();
	if ( down ) {
		reloadUseHeld = true;
		reloadUseStart = now;
		reloadUseActed = false;
	} else {
		if ( reloadUseHeld && !reloadUseActed ) {
			reloadPending = true;	// a tap
		}
		reloadUseHeld = false;
	}
}

void sdGamepadControls::UsercommandCallback( idPlayer* player, usercmd_t& cmd ) {
	const int now = Now();

	// X: a tap reloads, a hold uses (enter or leave a vehicle, arm, disarm, pick up)
	if ( reloadPending ) {
		reloadPending = false;
		player->PerformImpulse( UCI_RELOAD );
	}
	if ( reloadUseHeld && now - reloadUseStart >= in_padHoldTime.GetInteger() ) {
		cmd.buttons.btn.activate = true;
		if ( !reloadUseActed ) {
			reloadUseActed = true;
			player->PerformImpulse( UCI_USE_VEHICLE );
		}
	}

	if ( !OnFoot( player ) ) {
		// in a vehicle the triggers and the sprint click act as plain buttons: alt-fire held
		// (a vehicle's second weapon or zoom) and boost held
		zoomPulseUntil = 0;
		padSprint = false;
		quickState = QG_IDLE;
		if ( zoomWanted && player->GetProxyEntity() != NULL ) {
			cmd.buttons.btn.altAttack = true;
		}
		if ( padSprintHeld && player->GetProxyEntity() != NULL ) {
			cmd.buttons.btn.sprint = true;
		}
		return;
	}
	UpdateQuick( player, cmd, now );
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
	const int bankSlot = quickBank >= 0 && quickBank < inventory.GetMaxBanks() ? inventory.GetSlotForBank( quickBank ) : -1;

	switch ( quickState ) {
		case QG_IDLE:
			return;

		case QG_START: {
			if ( bankSlot == -1 ) {
				quickState = QG_IDLE;
				return;
			}
			quickPrevWeapon = inventory.GetCurrentWeapon();
			if ( inventory.GetCurrentSlot() == bankSlot ) {
				quickPrevWeapon = -1;	// already in hand: just use it
			} else {
				player->PerformImpulse( UCI_WEAP0 + quickBank );
			}
			quickDeadline = now + 1500;
			quickState = QG_SWITCHING;
			// fall through: it may already be up
		}

		case QG_SWITCHING: {
			idWeapon* weapon = player->GetWeapon();
			if ( inventory.GetCurrentSlot() == bankSlot && !inventory.IsSwitchActive() && weapon != NULL && weapon->IsReady() ) {
				quickState = QG_FIRING;
				quickFireStart = now;
			} else if ( now > quickDeadline ) {
				quickState = QG_IDLE;	// nothing in that bank (no ammo, or the class has none)
				return;
			} else {
				return;
			}
			// fall through
		}

		case QG_FIRING: {
			cmd.buttons.btn.attack = true;
			const bool minimumHeld = now - quickFireStart >= GAMEPAD_QUICK_MIN_FIRE_MSEC;
			if ( minimumHeld && ( quickMelee || !quickHeld ) ) {
				quickState = QG_DONE;
				quickDeadline = now + 1500;
				cmd.buttons.btn.attack = false;
			}
			return;
		}

		case QG_DONE: {
			idWeapon* weapon = player->GetWeapon();
			const bool done = inventory.GetCurrentSlot() != bankSlot || ( weapon != NULL && weapon->IsReady() && now - quickFireStart > 300 );
			if ( !done && now <= quickDeadline ) {
				return;
			}
			quickState = QG_IDLE;
			if ( quickPrevWeapon >= 0 && inventory.GetCurrentWeapon() != quickPrevWeapon && !inventory.IsSwitchActive() ) {
				inventory.SetSwitchingWeapon( quickPrevWeapon );
				player->ShowWeaponMenu( sdWeaponSelectionMenu::AT_DIRECT_SELECTION );
				player->AcceptWeaponSwitch( false );
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
