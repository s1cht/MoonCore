#pragma once

struct BaseTypes 
{
	/*
										-INTEGERS-
				
					The structure of defining integer types is as follows:
				
									1. 8-bit u/s int
									2. 16-bit u/s int
									3. 32-bit u/s int
									4. 64-bit u/s int
				
								u/s - unsigned / signed
	*/
	
	// SIGNED
	typedef signed char					i8;				// Signed 1.	(Check "INTEGERS" reference)
	typedef signed short				i16;			// Signed 2.	(Check "INTEGERS" reference)
	typedef signed int					i32;			// Signed 3.	(Check "INTEGERS" reference)
	typedef signed long long			i64;			// Signed 4.	(Check "INTEGERS" reference)

	// UNSIGNED
	typedef unsigned char				u8;				// Unsigned 1.	(Check "INTEGERS" reference)
	typedef unsigned short				u16;			// Unsigned 2.	(Check "INTEGERS" reference)
	typedef unsigned int				u32;			// Unsigned 3.	(Check "INTEGERS" reference)
	typedef unsigned long long			u64;			// Unsigned 4.	(Check "INTEGERS" reference)

	/*
									-NUMBER WITH FLOATING POINT-

				The structure of defining number with floating point types is as follows:

								        1. 32-bit float
								        2. 64-bit float
								        2. long 64-bit float
	*/

	typedef float						f32;		// 1.
	typedef double						f64;		// 2.
	typedef long double					lf64;		// 3.


	/*
										-CHARACTERS-

					The structure of defining character types is as follows:

									1. 8-bit character
									2. 16-bit character
									3. 32-bit character
									
									4. ASCII character
									5. Wide character
	*/

	typedef char8_t						char8;			// 1. (Check "CHARACTERS" reference)
	typedef char16_t					char16;			// 2. (Check "CHARACTERS" reference)
	typedef char32_t					char32;			// 3. (Check "CHARACTERS" reference)
	
	typedef char						char7;			// 4. (Check "CHARACTERS" reference)
	typedef wchar_t						wchar;			// 5. (Check "CHARACTERS" reference)

	/* 
										-INT POINTER-
				
					The structure of defining int pointer types is as follows:
				
										1. Signed
										2. Unsigned

	*/

	typedef i64 iptr;	// 1. (Check "INT POINTER" reference)
	typedef u64 uptr;	// 2. (Check "INT POINTER" reference)

	typedef uptr usize; // Unsigned size type
	typedef iptr isize; // Signed size type
};