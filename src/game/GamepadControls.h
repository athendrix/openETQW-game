// SPDX-License-Identifier: MIT
#ifndef __GAME_GAMEPADCONTROLS_H__
#define __GAME_GAMEPADCONTROLS_H__

/*
Gamepad play in the style of modern console shooters (Destiny 2 is the reference):

  - Stick aiming (on foot, and in seats that aim like a player): a radial dead zone, a
    response curve, separate yaw and pitch speeds, a turn ramp that speeds up yaw when
    the stick is held at the edge, a zoom multiplier on top of the weapon's own fov
    scaling, pitch inversion, and optional gyro aiming.
  - Aim assist for gamepad players on foot: the look speed slows while the crosshair is
    on an enemy player (friction), and while the player is aiming or moving, the view
    follows part of the target's motion across the screen (adhesion). Only for
    targets that are visible, alive, on the other team and not disguised. It only
    changes the local view angles in the usercmd, as stick input does.
  - Hold to zoom (_zoomhold): held, the weapon goes to its sights or scope; released, it
    comes back. The retail weapon scripts toggle sights on each alt-fire press, so this
    sends an alt-fire press whenever the wanted and the actual state differ, and only for
    weapons that have sights or a scope (on the grenades alt-fire throws underhand).
  - Quick grenade (_quickgrenade): selects the grenade bank, holds fire while the button
    is held (cooking), throws on release, then switches back to the previous weapon.
  - Quick melee (_quickmelee): selects the melee bank, swings once, switches back.
  - Pad sprint (_padsprint): a press starts sprinting, which lasts until the left stick
    comes back from forward or the player fires or zooms. In a vehicle it is boost, held.
  - Tap to reload, hold to use (_reloaduse): a tap reloads; held for in_padHoldTime it
    uses (enter or leave a vehicle, and the use button held: arm, disarm, pick up).
  - In a vehicle, _zoomhold is alt-fire held.

Everything here runs on the local client while it builds its usercmds. Nothing is sent
that a player could not send with the stock buttons, and the server is unchanged.
*/

class idPlayer;

class sdGamepadControls {
public:
	static sdGamepadControls&	GetInstance( void );

	// sdInputModePlayer::ControllerMove: aiming and moving on foot and in player-like seats
	void						PlayerControllerMove( bool doGameCallback, const int controllerNum, const float* controllerAxis, idVec3& viewAngles, usercmd_t& cmd );

	// local impulses (idGameLocal::HandleLocalImpulse)
	void						OnZoomHold( bool down );
	void						OnQuickGrenade( bool down );
	void						OnQuickMelee( bool down );
	void						OnPadSprint( bool down );
	void						OnReloadUse( bool down );

	// idPlayer::UsercommandCallback, for the local player
	void						UsercommandCallback( idPlayer* player, usercmd_t& cmd );

	void						Reset( void );

	// the pure math, public for the self test (gamepadSelfTest)
	static void					ApplyRadialDeadZone( float x, float y, float deadZone, float outerZone, float curve, float& outX, float& outY );
	static float				AngleBetween( const idVec3& a, const idVec3& b );

private:
								sdGamepadControls( void );

	int							Now( void ) const;

	// aim assist: the enemy player nearest the crosshair inside the assist cone, or NULL
	idPlayer*					FindAssistTarget( idPlayer* self, const idVec3& eye, const idVec3& forward, float& closeness, idVec3& targetPoint ) const;

	void						UpdateZoomHold( idPlayer* player, usercmd_t& cmd, int now );
	void						StartQuick( int bank, bool melee, bool down );
	void						UpdateQuick( idPlayer* player, usercmd_t& cmd, int now );

	enum quickState_t {
		QG_IDLE,
		QG_START,
		QG_SWITCHING,
		QG_FIRING,
		QG_DONE
	};

	// turn ramp
	int							lastAimTime;
	float						rampHeld;

	// aim assist adhesion: the target followed last frame and its bearing then
	int							assistTarget;
	int							assistSpawnId;
	idAngles					assistBearing;
	int							assistTime;

	// hold to zoom
	bool						zoomWanted;
	int							zoomPulseUntil;
	int							zoomNextPulse;
	int							zoomPulses;

	// quick grenade and quick melee
	bool						quickHeld;
	bool						quickMelee;
	int							quickBank;
	quickState_t				quickState;
	int							quickPrevWeapon;
	int							quickDeadline;
	int							quickFireStart;

	// pad sprint: the latch on foot, the button held in a vehicle (boost)
	bool						padSprint;
	bool						padSprintHeld;

	// tap to reload, hold to use
	bool						reloadUseHeld;
	bool						reloadUseActed;
	bool						reloadPending;
	int							reloadUseStart;
};

#endif /* !__GAME_GAMEPADCONTROLS_H__ */
