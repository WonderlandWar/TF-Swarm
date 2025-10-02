//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef ILOCALIZE_H
#define ILOCALIZE_H

#ifdef _WIN32
#pragma once
#endif

#include "appframework/IAppSystem.h"
#include <tier1/KeyValues.h>

// unicode character type
// for more unicode manipulation functions #include <wchar.h>
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


//-----------------------------------------------------------------------------
// Interface used to query text size so we can choose the longest one
//-----------------------------------------------------------------------------
abstract_class ILocalizeTextQuery
{
public:
	virtual int ComputeTextWidth( const wchar_t *pString ) = 0;
};


//-----------------------------------------------------------------------------
// Callback which is triggered when any localization string changes
// Is not called when a localization string is added
//-----------------------------------------------------------------------------
abstract_class ILocalizationChangeCallback
{
public:
	virtual void OnLocalizationChanged() = 0;
};


//-----------------------------------------------------------------------------
// Purpose: Handles localization of text
//			looks up string names and returns the localized unicode text
//-----------------------------------------------------------------------------
// direct references to localized strings
typedef uint32 LocalizeStringIndex_t;
const uint32 LOCALIZE_INVALID_STRING_INDEX = (LocalizeStringIndex_t)-1;

abstract_class ILocalize : public IAppSystem
{
public:
	// adds the contents of a file to the localization table
	virtual bool AddFile( const char *fileName, const char *pPathID = NULL, bool bIncludeFallbackSearchPaths = false ) = 0;

	// Remove all strings from the table
	virtual void RemoveAll() = 0;

	// Finds the localized text for tokenName. Returns NULL if none is found.
	virtual wchar_t *Find(const char *tokenName) = 0;

	// Like Find(), but as a failsafe, returns an error message instead of NULL if the string isn't found.  
	virtual const wchar_t *FindSafe(const char *tokenName) = 0;

	// converts an english string to unicode
	// returns the number of wchar_t in resulting string, including null terminator
	virtual int ConvertANSIToUnicode(const char *ansi, wchar_t *unicode, int unicodeBufferSizeInBytes) = 0;

	// converts an unicode string to an english string
	// unrepresentable characters are converted to system default
	// returns the number of characters in resulting string, including null terminator
	virtual int ConvertUnicodeToANSI(const wchar_t *unicode, char *ansi, int ansiBufferSize) = 0;

	// finds the index of a token by token name, INVALID_STRING_INDEX if not found
	virtual LocalizeStringIndex_t FindIndex(const char *tokenName) = 0;

	// builds a localized formatted string
	// uses the format strings first: %s1, %s2, ...  unicode strings (wchar_t *)
	virtual void ConstructString(wchar_t *unicodeOuput, int unicodeBufferSizeInBytes, const wchar_t *formatString, int numFormatParameters, ...) = 0;
	
	// gets the values by the string index
	virtual const char *GetNameByIndex(LocalizeStringIndex_t index) = 0;
	virtual wchar_t *GetValueByIndex(LocalizeStringIndex_t index) = 0;

	///////////////////////////////////////////////////////////////////
	// the following functions should only be used by localization editors

	// iteration functions
	virtual LocalizeStringIndex_t GetFirstStringIndex() = 0;
	// returns the next index, or INVALID_STRING_INDEX if no more strings available
	virtual LocalizeStringIndex_t GetNextStringIndex(LocalizeStringIndex_t index) = 0;

	// adds a single name/unicode string pair to the table
	virtual void AddString( const char *tokenName, wchar_t *unicodeString, const char *fileName ) = 0;

	// changes the value of a string
	virtual void SetValueByIndex(LocalizeStringIndex_t index, wchar_t *newValue) = 0;

	// saves the entire contents of the token tree to the file
	virtual bool SaveToFile( const char *fileName ) = 0;

	// iterates the filenames
	virtual int GetLocalizationFileCount() = 0;
	virtual const char *GetLocalizationFileName(int index) = 0;

	// returns the name of the file the specified localized string is stored in
	virtual const char *GetFileNameByIndex(LocalizeStringIndex_t index) = 0;

	// for development only, reloads localization files
	virtual void ReloadLocalizationFiles( ) = 0;

	// need to replace the existing ConstructString with this
	virtual void ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const char *tokenName, KeyValues *localizationVariables) = 0;
	virtual void ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, LocalizeStringIndex_t unlocalizedTextSymbol, KeyValues *localizationVariables) = 0;

	// Used to install a callback to query which localized strings are the longest
	virtual void SetTextQuery( ILocalizeTextQuery *pQuery ) = 0;

	// Is called when any localization strings change
	virtual void InstallChangeCallback( ILocalizationChangeCallback *pCallback ) = 0;
	virtual void RemoveChangeCallback( ILocalizationChangeCallback *pCallback ) = 0;
	
	template < typename T >
	static void ConstructString(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) T *unicodeOutput, int unicodeBufferSizeInBytes, const T *formatString, KeyValues *localizationVariables)
	{
		ConstructStringKeyValuesInternal( unicodeOutput, unicodeBufferSizeInBytes, formatString, localizationVariables );
	}
	
private:
	
	static void ConstructStringKeyValuesInternal(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const wchar_t *formatString, KeyValues *localizationVariables)
	{
		wchar_t *outputPos = unicodeOutput;

		//assumes we can't have %s10
		//assume both are 0 terminated?
		int unicodeBufferSize = unicodeBufferSizeInBytes / sizeof(wchar_t);

		while ( *formatString != '\0' && unicodeBufferSize > 1 )
		{
			bool shouldAdvance = true;

			if ( *formatString == '%' )
			{
				// this is an escape sequence that specifies a variable name
				if ( formatString[1] == 's' && formatString[2] >= '0' && formatString[2] <= '9' )
				{
					// old style escape sequence, ignore
				}
				else if ( formatString[1] == '%' )
				{
					// just a '%' char, just write the second one
					formatString++;
				}
				else if ( localizationVariables )
				{
					// get out the variable name
					const wchar_t *varStart = formatString + 1;
					const wchar_t *varEnd = StringFuncs<wchar_t>::FindChar( varStart, '%' );

					if ( varEnd && *varEnd == '%' )
					{
						shouldAdvance = false;

						// assume variable names must be ascii, do a quick convert
						char variableName[32];
						char *vset = variableName;
						for ( const wchar_t *pws = varStart; pws < varEnd && (vset < variableName + sizeof(variableName) - 1); ++pws, ++vset )
						{
							*vset = (char)*pws;
						}
						*vset = 0;

						// look up the variable name
						const wchar_t *value = localizationVariables->GetWString( variableName, L"[unknown]" );
					
						int paramSize = StringFuncs<wchar_t>::Length( value );
						if (paramSize >= unicodeBufferSize)
						{
							paramSize = MAX( 0, unicodeBufferSize - 1 );
						}

						StringFuncs<wchar_t>::Copy( outputPos, value, paramSize );

						unicodeBufferSize -= paramSize;
						outputPos += paramSize;
						formatString = varEnd + 1;
					}
				}
			}

			if (shouldAdvance)
			{
				//copy it over, char by char
				*outputPos = *formatString;

				outputPos++;
				unicodeBufferSize--;

				formatString++;
			}		
		}

		// ensure null termination
		*outputPos = '\0';
	}
};


	typedef wchar_t locchar_t;

	#define loc_snprintf	V_snwprintf
	#define loc_sprintf_safe V_swprintf_safe
	#define loc_sncat		V_wcsncat
	#define loc_scat_safe	V_wcscat_safe
	#define loc_sncpy		Q_wcsncpy
	#define loc_scpy_safe	V_wcscpy_safe
	#define loc_strlen		Q_wcslen
	#define LOCCHAR(x)		L ## x
	#define LOCCHAR_FMT_LOCPRINTF L"%ls"
	#define LOCCHAR_FMT_PRINTF    "%ls"
	#define LOCCHAR_FMT_WPRINTF   L"%ls"


// --------------------------------------------------------------------------
// Purpose:
// --------------------------------------------------------------------------

template < typename T >
class TypedKeyValuesStringHelper
{
public:
	static const T *Read( KeyValues *pKeyValues, const char *pKeyName, const T *pDefaultValue );
	static void	Write( KeyValues *pKeyValues, const char *pKeyName, const T *pValue );
};

// --------------------------------------------------------------------------

template < >
class TypedKeyValuesStringHelper<char>
{
public:
	static const char *Read( KeyValues *pKeyValues, const char *pKeyName, const char *pDefaultValue ) { return pKeyValues->GetString( pKeyName, pDefaultValue ); }
	static void Write( KeyValues *pKeyValues, const char *pKeyName, const char *pValue ) { pKeyValues->SetString( pKeyName, pValue ); }
};

// --------------------------------------------------------------------------

template < >
class TypedKeyValuesStringHelper<wchar_t>
{
public:
	static const wchar_t *Read( KeyValues *pKeyValues, const char *pKeyName, const wchar_t *pDefaultValue ) { return pKeyValues->GetWString( pKeyName, pDefaultValue ); }
	static void Write( KeyValues *pKeyValues, const char *pKeyName, const wchar_t *pValue ) { pKeyValues->SetWString( pKeyName, pValue ); }
};

// --------------------------------------------------------------------------
// Purpose: CLocalizedStringArg<> is a class that will take a variable of any
//			arbitary type and convert it to a string of whatever character type
//			we're using for localization (locchar_t).
//
//			Independently it isn't very useful, though it can be used to sort-of-
//			intelligently fill out the correct format string. It's designed to be
//			used for the arguments of CConstructLocalizedString, which can be of
//			arbitrary number and type.
//
//			If you pass in a (non-specialized) pointer, the code will assume that
//			you meant that pointer to be used as a localized string. This will
//			still fail to compile if some non-string type is passed in, but will
//			handle weird combinations of const/volatile/whatever automatically.
// --------------------------------------------------------------------------

// The base implementation doesn't do anything except fail to compile if you
// use it. Getting an "incomplete type" error here means that you tried to construct
// a localized string with a type that doesn't have a specialization.
template < typename T >
class CLocalizedStringArg;

// --------------------------------------------------------------------------

template < typename T >
class CLocalizedStringArgStringImpl
{
public:
	enum { kIsValid = true };

	CLocalizedStringArgStringImpl( const locchar_t *pStr ) : m_pStr( pStr ) { }

	const locchar_t *GetLocArg() const { Assert( m_pStr ); return m_pStr; }

private:
	const locchar_t *m_pStr;
};

// --------------------------------------------------------------------------

template < typename T >
class CLocalizedStringArg<T *> : public CLocalizedStringArgStringImpl<T>
{
public:
	CLocalizedStringArg( const locchar_t *pStr ) : CLocalizedStringArgStringImpl<T>( pStr ) { }
};

// --------------------------------------------------------------------------

template < typename T >
class CLocalizedStringArgPrintfImpl
{
public:
	enum { kIsValid = true };

	CLocalizedStringArgPrintfImpl( T value, const locchar_t *loc_Format ) { loc_snprintf( m_cBuffer, kBufferSize, loc_Format, value ); }

	const locchar_t *GetLocArg() const { return m_cBuffer; }

private:
	enum { kBufferSize = 128, };
	locchar_t m_cBuffer[ kBufferSize ];
};

// --------------------------------------------------------------------------

template < >
class CLocalizedStringArg<uint16> : public CLocalizedStringArgPrintfImpl<uint16>
{
public:
	CLocalizedStringArg( uint16 unValue ) : CLocalizedStringArgPrintfImpl<uint16>( unValue, LOCCHAR("%u") ) { }
};

// --------------------------------------------------------------------------

template < >
class CLocalizedStringArg<uint32> : public CLocalizedStringArgPrintfImpl<uint32>
{
public:
	CLocalizedStringArg( uint32 unValue ) : CLocalizedStringArgPrintfImpl<uint32>( unValue, LOCCHAR("%u") ) { }
};

// --------------------------------------------------------------------------

template < >
class CLocalizedStringArg<uint64> : public CLocalizedStringArgPrintfImpl<uint64>
{
public:
	CLocalizedStringArg( uint64 unValue ) : CLocalizedStringArgPrintfImpl<uint64>( unValue, LOCCHAR("%llu") ) { }
};

// --------------------------------------------------------------------------

template < >
class CLocalizedStringArg<float> : public CLocalizedStringArgPrintfImpl<float>
{
public:
	// Display one decimal point if we've got a value less than one, and no point
	// if we're greater than one or are effectively zero.
	CLocalizedStringArg( float fValue )
		: CLocalizedStringArgPrintfImpl<float>( fValue,
												fabsf( fValue ) <= FLT_EPSILON || fabsf( fValue ) >= 1.0f ? LOCCHAR("%.0f") : LOCCHAR("%.1f") )
	{
		//
	}
};

// --------------------------------------------------------------------------
// Purpose:
// --------------------------------------------------------------------------
class CConstructLocalizedString
{
public:
	template < typename T >
	CConstructLocalizedString( const locchar_t *loc_Format, T arg0 )
	{
		COMPILE_TIME_ASSERT( CLocalizedStringArg<T>::kIsValid );

		m_loc_Buffer[0] = '\0';

		if ( loc_Format )
		{
#ifdef CLIENT_DLL
			extern vgui::ILocalize *g_pVGuiLocalize;
			g_pVGuiLocalize->ConstructString( m_loc_Buffer, sizeof( m_loc_Buffer ), loc_Format, 1, CLocalizedStringArg<T>( arg0 ).GetLocArg() );
#else
			::ILocalize::ConstructString( m_loc_Buffer, sizeof( m_loc_Buffer ), loc_Format, 1, CLocalizedStringArg<T>( arg0 ).GetLocArg() );
#endif
		}
	}

	template < typename T, typename U >
	CConstructLocalizedString( const locchar_t *loc_Format, T arg0, U arg1 )
	{
		COMPILE_TIME_ASSERT( CLocalizedStringArg<T>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<U>::kIsValid );

		m_loc_Buffer[0] = '\0';

		if ( loc_Format )
		{
#ifdef CLIENT_DLL
			extern vgui::ILocalize *g_pVGuiLocalize;
			g_pVGuiLocalize->ConstructString( m_loc_Buffer, sizeof( m_loc_Buffer ), loc_Format, 2, CLocalizedStringArg<T>( arg0 ).GetLocArg(), CLocalizedStringArg<U>( arg1 ).GetLocArg() );
#else
			::ILocalize::ConstructString( m_loc_Buffer, sizeof( m_loc_Buffer ), loc_Format, 2, CLocalizedStringArg<T>( arg0 ).GetLocArg(), CLocalizedStringArg<U>( arg1 ).GetLocArg() );
#endif
		}
	}

	template < typename T, typename U, typename V >
	CConstructLocalizedString( const locchar_t *loc_Format, T arg0, U arg1, V arg2 )
	{
		COMPILE_TIME_ASSERT( CLocalizedStringArg<T>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<U>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<V>::kIsValid );

		m_loc_Buffer[0] = '\0';

		if ( loc_Format )
		{
#ifdef CLIENT_DLL
			extern vgui::ILocalize *g_pVGuiLocalize;
			g_pVGuiLocalize->ConstructString( m_loc_Buffer,
										  sizeof( m_loc_Buffer ),
										  loc_Format,
										  3,
										  CLocalizedStringArg<T>( arg0 ).GetLocArg(),
										  CLocalizedStringArg<U>( arg1 ).GetLocArg(),
										  CLocalizedStringArg<V>( arg2 ).GetLocArg() );
#else
			::ILocalize::ConstructString( m_loc_Buffer,
										  sizeof( m_loc_Buffer ),
										  loc_Format,
										  3,
										  CLocalizedStringArg<T>( arg0 ).GetLocArg(),
										  CLocalizedStringArg<U>( arg1 ).GetLocArg(),
										  CLocalizedStringArg<V>( arg2 ).GetLocArg() );
#endif
		}
	}

	template < typename T, typename U, typename V, typename W >
	CConstructLocalizedString( const locchar_t *loc_Format, T arg0, U arg1, V arg2, W arg3 )
	{
		COMPILE_TIME_ASSERT( CLocalizedStringArg<T>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<U>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<V>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<W>::kIsValid );

		m_loc_Buffer[0] = '\0';

		if ( loc_Format )
		{
#ifdef CLIENT_DLL
			extern vgui::ILocalize *g_pVGuiLocalize;
			g_pVGuiLocalize->ConstructString( m_loc_Buffer,
										  sizeof( m_loc_Buffer ),
										  loc_Format,
										  4,
										  CLocalizedStringArg<T>( arg0 ).GetLocArg(),
										  CLocalizedStringArg<U>( arg1 ).GetLocArg(),
										  CLocalizedStringArg<V>( arg2 ).GetLocArg(),
										  CLocalizedStringArg<W>( arg3 ).GetLocArg() );
#else
			::ILocalize::ConstructString( m_loc_Buffer,
										  sizeof( m_loc_Buffer ),
										  loc_Format,
										  4,
										  CLocalizedStringArg<T>( arg0 ).GetLocArg(),
										  CLocalizedStringArg<U>( arg1 ).GetLocArg(),
										  CLocalizedStringArg<V>( arg2 ).GetLocArg(),
										  CLocalizedStringArg<W>( arg3 ).GetLocArg() );
#endif
		}
	}

	template < typename T, typename U, typename V, typename W, typename X >
	CConstructLocalizedString( const locchar_t *loc_Format, T arg0, U arg1, V arg2, W arg3, X arg4 )
	{
		COMPILE_TIME_ASSERT( CLocalizedStringArg<T>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<U>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<V>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<W>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<X>::kIsValid );

		m_loc_Buffer[0] = '\0';

		if ( loc_Format )
		{
#ifdef CLIENT_DLL
			extern vgui::ILocalize *g_pVGuiLocalize;
			g_pVGuiLocalize->ConstructString( m_loc_Buffer,
				sizeof( m_loc_Buffer ),
				loc_Format,
				5,
				CLocalizedStringArg<T>( arg0 ).GetLocArg(),
				CLocalizedStringArg<U>( arg1 ).GetLocArg(),
				CLocalizedStringArg<V>( arg2 ).GetLocArg(),
				CLocalizedStringArg<W>( arg3 ).GetLocArg(),
				CLocalizedStringArg<X>( arg4 ).GetLocArg() );
#else
			::ILocalize::ConstructString( m_loc_Buffer,
				sizeof( m_loc_Buffer ),
				loc_Format,
				5,
				CLocalizedStringArg<T>( arg0 ).GetLocArg(),
				CLocalizedStringArg<U>( arg1 ).GetLocArg(),
				CLocalizedStringArg<V>( arg2 ).GetLocArg(),
				CLocalizedStringArg<W>( arg3 ).GetLocArg(),
				CLocalizedStringArg<X>( arg4 ).GetLocArg() );
#endif
		}
	}

	template < typename T, typename U, typename V, typename W, typename X, typename Y >
	CConstructLocalizedString( const locchar_t *loc_Format, T arg0, U arg1, V arg2, W arg3, X arg4, Y arg5 )
	{
		COMPILE_TIME_ASSERT( CLocalizedStringArg<T>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<U>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<V>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<W>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<X>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<Y>::kIsValid );

		m_loc_Buffer[0] = '\0';

		if ( loc_Format )
		{
#ifdef CLIENT_DLL
			extern vgui::ILocalize *g_pVGuiLocalize;
			g_pVGuiLocalize->ConstructString( m_loc_Buffer,
										  sizeof( m_loc_Buffer ),
										  loc_Format,
										  6,
										  CLocalizedStringArg<T>( arg0 ).GetLocArg(),
										  CLocalizedStringArg<U>( arg1 ).GetLocArg(),
										  CLocalizedStringArg<V>( arg2 ).GetLocArg(),
										  CLocalizedStringArg<W>( arg3 ).GetLocArg(),
										  CLocalizedStringArg<X>( arg4 ).GetLocArg(),
										  CLocalizedStringArg<Y>( arg5 ).GetLocArg() );
#else
			::ILocalize::ConstructString( m_loc_Buffer,
										  sizeof( m_loc_Buffer ),
										  loc_Format,
										  6,
										  CLocalizedStringArg<T>( arg0 ).GetLocArg(),
										  CLocalizedStringArg<U>( arg1 ).GetLocArg(),
										  CLocalizedStringArg<V>( arg2 ).GetLocArg(),
										  CLocalizedStringArg<W>( arg3 ).GetLocArg(),
										  CLocalizedStringArg<X>( arg4 ).GetLocArg(),
										  CLocalizedStringArg<Y>( arg5 ).GetLocArg() );
#endif
		}
	}

	template < typename T, typename U, typename V, typename W, typename X, typename Y, typename Z >
	CConstructLocalizedString( const locchar_t *loc_Format, T arg0, U arg1, V arg2, W arg3, X arg4, Y arg5, Z arg6)
	{
		COMPILE_TIME_ASSERT( CLocalizedStringArg<T>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<U>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<V>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<W>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<X>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<Y>::kIsValid );
		COMPILE_TIME_ASSERT( CLocalizedStringArg<Z>::kIsValid );

		m_loc_Buffer[0] = '\0';

		if ( loc_Format )
		{
#ifdef CLIENT_DLL
			extern vgui::ILocalize *g_pVGuiLocalize;
			g_pVGuiLocalize->ConstructString( m_loc_Buffer,
				sizeof( m_loc_Buffer ),
				loc_Format,
				7,
				CLocalizedStringArg<T>( arg0 ).GetLocArg(),
				CLocalizedStringArg<U>( arg1 ).GetLocArg(),
				CLocalizedStringArg<V>( arg2 ).GetLocArg(),
				CLocalizedStringArg<W>( arg3 ).GetLocArg(),
				CLocalizedStringArg<X>( arg4 ).GetLocArg(),
				CLocalizedStringArg<Y>( arg5 ).GetLocArg(), 
				CLocalizedStringArg<Z>( arg6 ).GetLocArg() );
#else
			::ILocalize::ConstructString( m_loc_Buffer,
				sizeof( m_loc_Buffer ),
				loc_Format,
				7,
				CLocalizedStringArg<T>( arg0 ).GetLocArg(),
				CLocalizedStringArg<U>( arg1 ).GetLocArg(),
				CLocalizedStringArg<V>( arg2 ).GetLocArg(),
				CLocalizedStringArg<W>( arg3 ).GetLocArg(),
				CLocalizedStringArg<X>( arg4 ).GetLocArg(),
				CLocalizedStringArg<Y>( arg5 ).GetLocArg(), 
				CLocalizedStringArg<Z>( arg6 ).GetLocArg() );
#endif
		}
	}

	CConstructLocalizedString( const locchar_t *loc_Format, KeyValues *pKeyValues )
	{
		m_loc_Buffer[0] = '\0';

		if ( loc_Format && pKeyValues )
		{
			::ILocalize::ConstructString( m_loc_Buffer, sizeof( m_loc_Buffer ), loc_Format, pKeyValues );
		}
	}

	operator const locchar_t *() const
	{
		return m_loc_Buffer;
	}

private:
	enum { kBufferSize = 512, };
	locchar_t m_loc_Buffer[ kBufferSize ];
};

#endif // ILOCALIZE_H
