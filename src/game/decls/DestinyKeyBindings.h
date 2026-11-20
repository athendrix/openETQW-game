// SPDX-License-Identifier: MIT
#ifndef __DECLS_DESTINYKEYBINDINGS_H__
#define __DECLS_DESTINYKEYBINDINGS_H__

/*
The settings menu's controls lists (retail's keyBindings decls etqw/movement,
etqw/weapons, etqw/misc), rewritten for the Destiny 2 style controls
(GamepadControls.h): a player remaps which key is the grenade, the melee, the super,
and so on, not retail's weapon banks. sdDeclKeyBinding::Parse puts these in place of
the decl's keys; etqw/vehicles and etqw/dev stay retail's.

A label is a localized string's name, or plain text after a '#' (the actions retail
never had; Game_ui.cpp CreateKeyBindingList shows it as it is).
*/

struct destinyKeyBinding_t {
	const char*		label;
	const char*		command;
};

static const destinyKeyBinding_t DESTINY_BINDINGS_MOVEMENT[] = {
	{ "guis/mainmenu/bindings/forward",			"_forward" },
	{ "guis/mainmenu/bindings/back",			"_back" },
	{ "guis/mainmenu/bindings/move_left",		"_moveleft" },
	{ "guis/mainmenu/bindings/move_right",		"_moveright" },
	{ "guis/mainmenu/bindings/jump",			"_moveup" },
	{ "guis/mainmenu/bindings/crouch",			"_movedown" },
	{ "#Crouch (tap to stay down, hold for class ability)",	"_crouchprone" },
	{ "guis/mainmenu/bindings/prone",			"_prone" },
	{ "#Sprint (runs until you stop)",			"_padsprint" },
	{ "guis/mainmenu/bindings/walkengine_thrust", "_speed" },
	{ NULL, NULL }
};

static const destinyKeyBinding_t DESTINY_BINDINGS_COMBAT[] = {
	{ "guis/mainmenu/bindings/attack",			"_attack" },
	{ "#Aim down sights (hold)",				"_zoomhold" },
	{ "guis/mainmenu/bindings/reload",			"_reload" },
	{ "#Reload (tap) / Use (hold)",				"_reloaduse" },
	{ "#Interact: use, enter vehicles",			"_interact" },
	{ "#Swap weapon",							"_weapnext" },
	{ "#Swap weapon (tap) / Heavy (hold)",		"_swapweapon" },
	{ "#Primary weapon",						"_weapon2" },
	{ "#Sidearm",								"_weapon1" },
	{ "#Heavy: binoculars, artillery",			"_heavy" },
	{ "#Grenade (hold to cook)",				"_quickgrenade" },
	{ "#Melee",									"_quickmelee" },
	{ "#Super: item (long press: second item)",	"_quickitem" },
	{ "#Class ability: deploy tool",			"_quicktool" },
	{ NULL, NULL }
};

static const destinyKeyBinding_t DESTINY_BINDINGS_MISC[] = {
	{ "guis/mainmenu/bindings/activate",		"_activate" },
	{ "guis/mainmenu/bindings/toggle_ready",	"_ready" },
	{ "guis/mainmenu/bindings/show_fireteam",	"_showFireTeam" },
	{ "guis/mainmenu/bindings/command_map",		"_commandmap" },
	{ "guis/mainmenu/bindings/zoom_map_in",		"zoominCommandMap" },
	{ "guis/mainmenu/bindings/zoom_map_out",	"zoomoutCommandMap" },
	{ "guis/mainmenu/bindings/show_waypoints",	"_showwaypoints" },
	{ "guis/mainmenu/bindings/toggle_waypoints", "toggle g_showwaypoints 0 1" },
	{ "guis/mainmenu/bindings/limbo_menu",		"_limbomenu" },
	{ "guis/mainmenu/bindings/fireteam_menu",	"_fireteam" },
	{ "guis/mainmenu/bindings/task_menu",		"_taskmenu" },
	{ "guis/mainmenu/bindings/vote_menu",		"_votemenu" },
	{ "guis/mainmenu/bindings/chat",			"clientMessageMode" },
	{ "guis/mainmenu/bindings/team_chat",		"clientMessageMode 1" },
	{ "guis/mainmenu/bindings/fireteam_chat",	"clientMessageMode 2" },
	{ "guis/mainmenu/bindings/context_menu",	"_context" },
	{ "guis/mainmenu/bindings/quick_chat",		"_quickchat" },
	{ "guis/mainmenu/bindings/stroyent_health",	"_stroydown" },
	{ "guis/mainmenu/bindings/health_stroyent",	"_stroyup" },
	{ "guis/mainmenu/bindings/show_scores",		"_showscores" },
	{ "guis/mainmenu/bindings/gdf_team",		"clientTeam gdf" },
	{ "guis/mainmenu/bindings/strogg_team",		"clientTeam strogg" },
	{ "guis/mainmenu/bindings/spectator",		"clientTeam spectator" },
	{ "guis/mainmenu/bindings/vote_yes",		"vote y" },
	{ "guis/mainmenu/bindings/vote_no",			"vote n" },
	{ NULL, NULL }
};

// the list for a retail keyBindings decl, or NULL to keep the decl's
inline const destinyKeyBinding_t* DestinyKeyBindingsFor( const char* declName ) {
	if ( idStr::Icmp( declName, "etqw/movement" ) == 0 ) {
		return DESTINY_BINDINGS_MOVEMENT;
	}
	if ( idStr::Icmp( declName, "etqw/weapons" ) == 0 ) {
		return DESTINY_BINDINGS_COMBAT;
	}
	if ( idStr::Icmp( declName, "etqw/misc" ) == 0 ) {
		return DESTINY_BINDINGS_MISC;
	}
	return NULL;
}

#endif /* !__DECLS_DESTINYKEYBINDINGS_H__ */
