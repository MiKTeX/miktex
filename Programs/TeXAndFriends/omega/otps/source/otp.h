/* otp.h: Instructions found in OCP files.

This file is part of the Omega project, which
is based in the web2c distribution of TeX.

Copyright (c) 1994--2000 John Plaice and Yannis Haralambous

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


#define OTP_RIGHT_OUTPUT	 1
#define OTP_RIGHT_NUM		 2
#define OTP_RIGHT_CHAR		 3
#define OTP_RIGHT_LCHAR		 4
#define OTP_RIGHT_SOME		 5

#define OTP_PBACK_OUTPUT	 6
#define OTP_PBACK_NUM		 7
#define OTP_PBACK_CHAR		 8
#define OTP_PBACK_LCHAR		 9
#define OTP_PBACK_SOME		10

#define OTP_ADD			11
#define OTP_SUB			12
#define OTP_MULT		13
#define OTP_DIV			14
#define OTP_MOD			15
#define OTP_LOOKUP		16
#define OTP_PUSH_NUM		17
#define OTP_PUSH_CHAR		18
#define OTP_PUSH_LCHAR		19

#define OTP_STATE_CHANGE	20
#define OTP_STATE_PUSH		21
#define OTP_STATE_POP		22

#define OTP_LEFT_START		23
#define OTP_LEFT_RETURN		24
#define OTP_LEFT_BACKUP		25

#define OTP_GOTO		26
#define OTP_GOTO_NE		27
#define OTP_GOTO_EQ		28
#define OTP_GOTO_LT		29
#define OTP_GOTO_LE		30
#define OTP_GOTO_GT		31
#define OTP_GOTO_GE		32
#define OTP_GOTO_NO_ADVANCE	33
#define OTP_GOTO_BEG		34
#define OTP_GOTO_END		35

#define OTP_STOP		36

#define OTP_PBACK_OFFSET (OTP_PBACK_OUTPUT - OTP_RIGHT_OUTPUT)
