// SPDX-License-Identifier: MIT
#ifndef __GAME_RULES_LOGINTHROTTLE_H__
#define __GAME_RULES_LOGINTHROTTLE_H__

/*
Password attempts per address, for the server's join password (ServerAllowClient) and
the admin login (sdUserGroupManagerLocal::Login): three wrong guesses are free, then each
wrong guess doubles the wait before the next is checked (2 s, 4 s, ... up to 5 minutes).
A right one clears the address. Addresses not seen for ten minutes are forgotten, and
when the table is full the stalest entry makes room.
*/
class sdLoginThrottle {
public:
	sdLoginThrottle() { memset( entries, 0, sizeof( entries ) ); }

	// may this address try a password now
	bool Allowed( unsigned int address, int now ) const {
		const entry_t* entry = Find( address );
		return entry == NULL || now >= entry->blockedUntil;
	}

	void Failed( unsigned int address, int now ) {
		entry_t* entry = FindOrAdd( address, now );
		entry->failures++;
		entry->lastSeen = now;
		if ( entry->failures > FREE_FAILURES ) {
			const int doublings = entry->failures - FREE_FAILURES - 1;
			int wait = doublings >= 8 ? MAX_WAIT_MSEC : ( 2000 << doublings );
			if ( wait > MAX_WAIT_MSEC ) {
				wait = MAX_WAIT_MSEC;
			}
			entry->blockedUntil = now + wait;
		}
	}

	void Succeeded( unsigned int address ) {
		entry_t* entry = Find( address );
		if ( entry != NULL ) {
			memset( entry, 0, sizeof( *entry ) );
		}
	}

	// compares in time that does not depend on where the strings first differ
	static bool PasswordsMatch( const char* expected, const char* given ) {
		if ( expected == NULL || given == NULL ) {
			return false;
		}
		const size_t expectedLength = strlen( expected );
		const size_t givenLength = strlen( given );
		unsigned char diff = expectedLength == givenLength ? 0 : 1;
		for ( size_t i = 0; i < givenLength; i++ ) {
			const unsigned char e = expectedLength > 0 ? static_cast< unsigned char >( expected[ i % expectedLength ] ) : 0;
			diff |= static_cast< unsigned char >( e ^ static_cast< unsigned char >( given[ i ] ) );
		}
		return diff == 0 && expectedLength > 0;
	}

private:
	static const int NUM_ENTRIES = 128;
	static const int FREE_FAILURES = 3;
	static const int MAX_WAIT_MSEC = 300000;
	static const int FORGET_MSEC = 600000;

	struct entry_t {
		bool			used;
		unsigned int	address;
		int				failures;
		int				blockedUntil;
		int				lastSeen;
	};

	const entry_t* Find( unsigned int address ) const {
		for ( int i = 0; i < NUM_ENTRIES; i++ ) {
			if ( entries[ i ].used && entries[ i ].address == address ) {
				return &entries[ i ];
			}
		}
		return NULL;
	}

	entry_t* Find( unsigned int address ) {
		return const_cast< entry_t* >( static_cast< const sdLoginThrottle* >( this )->Find( address ) );
	}

	entry_t* FindOrAdd( unsigned int address, int now ) {
		entry_t* entry = Find( address );
		if ( entry != NULL ) {
			return entry;
		}
		entry_t* stalest = &entries[ 0 ];
		for ( int i = 0; i < NUM_ENTRIES; i++ ) {
			entry_t& candidate = entries[ i ];
			if ( !candidate.used || now - candidate.lastSeen > FORGET_MSEC ) {
				stalest = &candidate;
				break;
			}
			if ( candidate.lastSeen < stalest->lastSeen && now >= candidate.blockedUntil ) {
				stalest = &candidate;
			}
		}
		memset( stalest, 0, sizeof( *stalest ) );
		stalest->used = true;
		stalest->address = address;
		stalest->lastSeen = now;
		return stalest;
	}

	entry_t	entries[ NUM_ENTRIES ];
};

// the server's one throttle (Game_network.cpp)
sdLoginThrottle& LoginThrottle( void );

#endif /* !__GAME_RULES_LOGINTHROTTLE_H__ */
