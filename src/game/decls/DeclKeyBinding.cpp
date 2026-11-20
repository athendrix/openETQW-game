// Copyright (C) 2007 Id Software, Inc.
//


#include "precompiled.h"
#pragma hdrstop

#include "DeclKeyBinding.h"
#include "../../framework/DeclParseHelper.h"
#include "DestinyKeyBindings.h"

/*
===============================================================================

	sdDeclKeyBinding

===============================================================================
*/

/*
================
sdDeclKeyBinding::sdDeclKeyBinding
================
*/
sdDeclKeyBinding::sdDeclKeyBinding( void ) {
}

/*
================
sdDeclKeyBinding::~sdDeclKeyBinding
================
*/
sdDeclKeyBinding::~sdDeclKeyBinding( void ) {
	FreeData();
}

/*
================
sdDeclKeyBinding::DefaultDefinition
================
*/
const char* sdDeclKeyBinding::DefaultDefinition( void ) const {
	return		\
		"{\n"	\
		"}\n";
}

/*
================
sdDeclKeyBinding::Parse
================
*/
bool sdDeclKeyBinding::Parse( const char *text, const int textLength ) {
	idToken token;
	idParser src;

	src.SetFlags( DECL_LEXER_FLAGS );
	//src.LoadMemory( text, textLength, GetFileName(), GetLineNum() );
	sdDeclParseHelper declHelper( this, text, textLength, src );

	src.SkipUntilString( "{", &token );

	while( true ) {
		if( !src.ReadToken( &token )) {
			src.Error( "sdDeclKeyBinding::Parse: unexpected end of file." );
			break;
		}
		if( !token.Cmp( "}" )) {
			break;
		}

		if( !token.Icmp( "keys" )) {
			if( !ParseKeys( src )) {
				src.Error( "sdDeclKeyBinding::Parse: failed to parse keys" );
				break;
			}
			continue;
		}

		if( !token.Icmp( "title" )) {
			if( !src.ReadToken( &token )) {
				src.Error( "sdDeclKeyBinding::Parse: failed to title" );
				break;
			}
			title = declHolder.declLocStrType.LocalFind( token );
			continue;
		}
	}

	// the Destiny 2 style controls replace retail's lists (DestinyKeyBindings.h)
	const destinyKeyBinding_t* destiny = DestinyKeyBindingsFor( GetName() );
	if ( destiny != NULL ) {
		keys.Clear();
		for ( int i = 0; destiny[ i ].label != NULL; i++ ) {
			keys.Set( destiny[ i ].label, destiny[ i ].command );
		}
	}
	
	return true;
}

/*
================
sdDeclKeyBinding::FreeData
================
*/
void sdDeclKeyBinding::FreeData( void ) {
	title = NULL;
	keys.Clear();
}


/*
============
sdDeclKeyBinding::ParseKeys
============
*/
bool sdDeclKeyBinding::ParseKeys( idParser& src ) {
	return keys.Parse( src );
}
