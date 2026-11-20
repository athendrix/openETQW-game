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
  - Quick tool and item (_quicktool, _quickitem): select the tools (_weapon4) or items
    (_weapon5) bank, hold fire while the button is held (hack, arm, place, throw a pack),
    then switch back, so the player is only ever on a gun unless a button is held.
    Each quick action switches itself, without the weapon menu and (where the game
    runs here) without the put-away animation (QuickSwitchTo). in_quickDebug prints
    what they do.
  - Guns only (in_gunsOnly, on): the weapon keys, the wheel and next/previous weapon
    only select the sidearm and the primary; next/previous swaps between the two.
    Everything else is in hand only while its own button is held.
  - Pad sprint (_padsprint): a press starts sprinting, which lasts until the player stops
    moving forward (the left stick comes back, or the forward key is let go) or fires
    or zooms. In a vehicle it is boost, held.
  - Tap to reload, hold to use (_reloaduse): a tap reloads; held for in_padHoldTime it
    uses (enter or leave a vehicle, and the use button held: arm, disarm, pick up).
  - In a vehicle, _zoomhold is alt-fire held, and in a vehicle X taps to switch seat or
    launch decoys (activate) and holds to get out.
  - _padfire: fire, except for the driver of a ground vehicle who drives with the
    triggers (Sparrow style: right trigger throttle, left trigger brake and reverse; the
    car and hovertank input modes in InputMode.cpp, in_vehicle_triggerThrottle).
  - _crouchprone: a tap crouches and stays crouched until the next tap, a jump or
    sprint; held, the player goes prone (a tap stands up again).
  - _swapweapon: a tap swaps guns, a hold uses the item (_quickitem) while held.
  - _interact (keyboard E, Destiny style): use at once and held; in a vehicle a tap
    switches seat and a hold gets out.

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
	void						OnQuickTool( bool down );
	void						OnQuickItem( bool down );
	void						OnHeavy( bool down );
	void						OnPadSprint( bool down );
	void						OnReloadUse( bool down );
	void						OnInteract( bool down );
	void						OnPadFire( bool down );
	void						OnCrouchProne( bool down );
	void						OnSwapWeapon( bool down );

	// the car and hovertank input modes drive with the triggers this frame (Sparrow style)
	void						NoteTriggerDriving( void );
	bool						TriggerDriving( void ) const;

	// in_gunsOnly: idPlayer::SelectWeapon (a bank) and NextWeapon/PrevWeapon (-1) for the
	// local player; true when it took the request
	bool						SelectGun( idPlayer* player, int bank );
	bool						GunsOnly( void ) const;

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
	static void					QuickSwitchTo( idPlayer* player, int weaponIndex );
	static int					BankWeapon( idPlayer* player, int bank );
	static int					BankWeaponAt( idPlayer* player, int bank, int n );
	static int					BankWeaponNamed( idPlayer* player, int bank, const char* text );
	static bool					WeaponNamed( idPlayer* player, int index, const char* text );
	static bool					ThirdEyePlaced( idPlayer* player );
	void						StartQuickOn( int bank, int weaponIndex, bool alt, bool held, int returnTo );
	void						UpdateItemButton( idPlayer* player, int now );
	void						UpdateClassAbility( idPlayer* player, int now );
	void						UpdateSticky( idPlayer* player, int now );
	void						SelectHeavy( idPlayer* player );

	struct chordButton_t {
		bool					down;
		bool					pending;
		int						pressTime;
	};
	void						PressChord( chordButton_t& button, bool down );
	void						UpdateChord( int now );
	void						SetQuickState( int state, const char* why );

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
	int							quickTarget;		// the weapon to use, -1: the bank's first
	bool						quickAlt;			// press alt-fire instead of fire
	int							quickReturn;		// the weapon to come back to when the target was in hand

	// the item button: press and long press
	bool						itemHeld;
	bool						itemActed;
	bool						itemShortPending;
	int							itemStart;

	// the class ability button: the deploy tool toggle
	bool						toolHeld;
	bool						toolPressPending;
	int							deployWeapon;		// out by the toggle, -1: none
	int							deployReturn;
	int							deployStart;
	bool						deploySawPlacing;

	// grenade + melee together: the super
	chordButton_t				grenadeButton;
	chordButton_t				meleeButton;
	bool						superChord;
	bool						heavyPending;

	// an item kept in hand for a remote view (third eye, flyer drone)
	int							stickyWeapon;
	int							stickyReturn;
	int							stickyStart;

	// pad sprint: the latch on foot, the button held in a vehicle (boost)
	bool						padSprint;
	bool						padSprintHeld;

	// tap and hold buttons: X (_reloaduse) and E (_interact)
	enum tapHoldMode_t {
		TH_RELOAD_USE,		// tap reloads, hold uses
		TH_INTERACT			// press uses at once and holds use
	};
	void						OnTapHold( tapHoldMode_t mode, bool down );
	void						UpdateTapHold( idPlayer* player, usercmd_t& cmd, int now, bool onFoot );
	tapHoldMode_t				tapHoldMode;
	bool						reloadUseHeld;
	bool						reloadUseActed;
	bool						reloadPending;
	int							reloadUseStart;
	int							activatePulseUntil;

	// right trigger as fire, which a driver using the triggers to drive does not send
	bool						padFireHeld;
	int							triggerDrivingTime;

	// B: tap to crouch (a latch), hold to go prone
	bool						crouchHeld;
	bool						crouchActed;
	bool						crouchLatched;
	bool						crouchStandPending;
	int							crouchStart;

	// Y: tap for the next weapon, hold for the items bank
	bool						swapHeld;
	bool						swapActed;
	bool						swapPending;
	int							swapStart;
	bool						usedVehicleThisPress;
};

#endif /* !__GAME_GAMEPADCONTROLS_H__ */
