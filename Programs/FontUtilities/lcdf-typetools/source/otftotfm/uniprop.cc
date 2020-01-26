/* uniprop.{cc,hh} -- code for Unicode character properties
 *
 * Copyright (c) 2004-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#include <config.h>
#include "uniprop.hh"
#include <lcdf/string.hh>

const unsigned char UnicodeProperty::property_pages[] = {
    0, P_Cn,
    0, P_Cc, 32, P_Zs, 33, P_Po, 36, P_Sc, 37, P_Po, 40, P_Ps, 41, P_Pe, 42, P_Po, 43, P_Sm, 44, P_Po, 45, P_Pd, 46, P_Po, 48, P_Nd, 58, P_Po, 60, P_Sm, 63, P_Po, 65, P_Lu, 91, P_Ps, 92, P_Po, 93, P_Pe, 94, P_Sk, 95, P_Pc, 96, P_Sk, 97, P_Ll, 123, P_Ps, 124, P_Sm, 125, P_Pe, 126, P_Sm, 127, P_Cc, 160, P_Zs, 161, P_Po, 162, P_Sc, 166, P_So, 167, P_Po, 168, P_Sk, 169, P_So, 170, P_Lo, 171, P_Pi, 172, P_Sm, 173, P_Cf, 174, P_So, 175, P_Sk, 176, P_So, 177, P_Sm, 178, P_No, 180, P_Sk, 181, P_Ll, 182, P_Po, 184, P_Sk, 185, P_No, 186, P_Lo, 187, P_Pf, 188, P_No, 191, P_Po, 192, P_Lu, 215, P_Sm, 216, P_Lu, 223, P_Ll, 247, P_Sm, 248, P_Ll,
    0, P_Lul, 56, P_Ll, 57, P_Lul, 73, P_Ll, 74, P_Lul, 120, P_Lu, 122, P_Ll, 123, P_Lul, 127, P_Ll, 129, P_Lu, 131, P_Ll, 132, P_Lul, 134, P_Lu, 136, P_Ll, 137, P_Lu, 140, P_Ll, 142, P_Lu, 146, P_Ll, 147, P_Lu, 149, P_Ll, 150, P_Lu, 153, P_Ll, 156, P_Lu, 158, P_Ll, 159, P_Lu, 161, P_Ll, 162, P_Lul, 166, P_Lu, 168, P_Ll, 169, P_Lul, 171, P_Ll, 172, P_Lul, 174, P_Lu, 176, P_Ll, 177, P_Lu, 180, P_Ll, 181, P_Lul, 183, P_Lu, 185, P_Ll, 187, P_Lo, 188, P_Lul, 190, P_Ll, 192, P_Lo, 196, P_Lu, 197, P_Lt, 198, P_Ll, 199, P_Lu, 200, P_Lt, 201, P_Ll, 202, P_Lu, 203, P_Lt, 204, P_Ll, 205, P_Lul, 221, P_Ll, 222, P_Lul, 240, P_Ll, 241, P_Lu, 242, P_Lt, 243, P_Ll, 244, P_Lul, 246, P_Lu, 249, P_Ll, 250, P_Lul,
    0, P_Lul, 52, P_Ll, 58, P_Lu, 60, P_Ll, 61, P_Lu, 63, P_Ll, 65, P_Lul, 67, P_Lu, 71, P_Ll, 72, P_Lul, 80, P_Ll, 148, P_Lo, 149, P_Ll, 176, P_Lm, 194, P_Sk, 198, P_Lm, 210, P_Sk, 224, P_Lm, 229, P_Sk, 236, P_Lm, 237, P_Sk, 238, P_Lm, 239, P_Sk,
    0, P_Mn, 112, P_Lul, 116, P_Lm, 117, P_Sk, 118, P_Lul, 120, P_Cn, 122, P_Lm, 123, P_Ll, 126, P_Po, 127, P_Lu, 128, P_Cn, 132, P_Sk, 134, P_Lu, 135, P_Po, 136, P_Lu, 139, P_Cn, 140, P_Lu, 141, P_Cn, 142, P_Lu, 144, P_Ll, 145, P_Lu, 162, P_Cn, 163, P_Lu, 172, P_Ll, 207, P_Lul, 209, P_Ll, 210, P_Lu, 213, P_Ll, 216, P_Lul, 240, P_Ll, 244, P_Lul, 246, P_Sm, 247, P_Lul, 249, P_Lu, 251, P_Ll, 253, P_Lu,
    0, P_Lu, 48, P_Ll, 96, P_Lul, 130, P_So, 131, P_Mn, 136, P_Me, 138, P_Lul, 192, P_Lu, 194, P_Ll, 195, P_Lul, 207, P_Ll, 208, P_Lul,
    0, P_Lul, 48, P_Cn, 49, P_Lu, 87, P_Cn, 89, P_Lm, 90, P_Po, 96, P_Cn, 97, P_Ll, 136, P_Cn, 137, P_Po, 138, P_Pd, 139, P_Cn, 141, P_So, 143, P_Sc, 144, P_Cn, 145, P_Mn, 190, P_Pd, 191, P_Mn, 192, P_Po, 193, P_Mn, 195, P_Po, 196, P_Mn, 198, P_Po, 199, P_Mn, 200, P_Cn, 208, P_Lo, 235, P_Cn, 240, P_Lo, 243, P_Po, 245, P_Cn,
    0, P_Cf, 6, P_Sm, 9, P_Po, 11, P_Sc, 12, P_Po, 14, P_So, 16, P_Mn, 27, P_Po, 28, P_Cf, 29, P_Cn, 30, P_Po, 32, P_Lo, 64, P_Lm, 65, P_Lo, 75, P_Mn, 96, P_Nd, 106, P_Po, 110, P_Lo, 112, P_Mn, 113, P_Lo, 212, P_Po, 213, P_Lo, 214, P_Mn, 221, P_Cf, 222, P_So, 223, P_Mn, 229, P_Lm, 231, P_Mn, 233, P_So, 234, P_Mn, 238, P_Lo, 240, P_Nd, 250, P_Lo, 253, P_So, 255, P_Lo,
    0, P_Po, 14, P_Cn, 15, P_Cf, 16, P_Lo, 17, P_Mn, 18, P_Lo, 48, P_Mn, 75, P_Cn, 77, P_Lo, 166, P_Mn, 177, P_Lo, 178, P_Cn, 192, P_Nd, 202, P_Lo, 235, P_Mn, 244, P_Lm, 246, P_So, 247, P_Po, 250, P_Lm, 251, P_Cn,
    0, P_Lo, 22, P_Mn, 26, P_Lm, 27, P_Mn, 36, P_Lm, 37, P_Mn, 40, P_Lm, 41, P_Mn, 46, P_Cn, 48, P_Po, 63, P_Cn, 64, P_Lo, 89, P_Mn, 92, P_Cn, 94, P_Po, 95, P_Cn, 160, P_Lo, 181, P_Cn, 182, P_Lo, 190, P_Cn, 212, P_Mn, 226, P_Cf, 227, P_Mn,
    0, P_Mn, 3, P_Mc, 4, P_Lo, 58, P_Mn, 59, P_Mc, 60, P_Mn, 61, P_Lo, 62, P_Mc, 65, P_Mn, 73, P_Mc, 77, P_Mn, 78, P_Mc, 80, P_Lo, 81, P_Mn, 88, P_Lo, 98, P_Mn, 100, P_Po, 102, P_Nd, 112, P_Po, 113, P_Lm, 114, P_Lo, 129, P_Mn, 130, P_Mc, 132, P_Cn, 133, P_Lo, 141, P_Cn, 143, P_Lo, 145, P_Cn, 147, P_Lo, 169, P_Cn, 170, P_Lo, 177, P_Cn, 178, P_Lo, 179, P_Cn, 182, P_Lo, 186, P_Cn, 188, P_Mn, 189, P_Lo, 190, P_Mc, 193, P_Mn, 197, P_Cn, 199, P_Mc, 201, P_Cn, 203, P_Mc, 205, P_Mn, 206, P_Lo, 207, P_Cn, 215, P_Mc, 216, P_Cn, 220, P_Lo, 222, P_Cn, 223, P_Lo, 226, P_Mn, 228, P_Cn, 230, P_Nd, 240, P_Lo, 242, P_Sc, 244, P_No, 250, P_So, 251, P_Sc, 252, P_Cn,
    0, P_Cn, 1, P_Mn, 3, P_Mc, 4, P_Cn, 5, P_Lo, 11, P_Cn, 15, P_Lo, 17, P_Cn, 19, P_Lo, 41, P_Cn, 42, P_Lo, 49, P_Cn, 50, P_Lo, 52, P_Cn, 53, P_Lo, 55, P_Cn, 56, P_Lo, 58, P_Cn, 60, P_Mn, 61, P_Cn, 62, P_Mc, 65, P_Mn, 67, P_Cn, 71, P_Mn, 73, P_Cn, 75, P_Mn, 78, P_Cn, 81, P_Mn, 82, P_Cn, 89, P_Lo, 93, P_Cn, 94, P_Lo, 95, P_Cn, 102, P_Nd, 112, P_Mn, 114, P_Lo, 117, P_Mn, 118, P_Cn, 129, P_Mn, 131, P_Mc, 132, P_Cn, 133, P_Lo, 142, P_Cn, 143, P_Lo, 146, P_Cn, 147, P_Lo, 169, P_Cn, 170, P_Lo, 177, P_Cn, 178, P_Lo, 180, P_Cn, 181, P_Lo, 186, P_Cn, 188, P_Mn, 189, P_Lo, 190, P_Mc, 193, P_Mn, 198, P_Cn, 199, P_Mn, 201, P_Mc, 202, P_Cn, 203, P_Mc, 205, P_Mn, 206, P_Cn, 208, P_Lo, 209, P_Cn, 224, P_Lo, 226, P_Mn, 228, P_Cn, 230, P_Nd, 240, P_Po, 241, P_Sc, 242, P_Cn, 249, P_Lo, 250, P_Cn,
    0, P_Cn, 1, P_Mn, 2, P_Mc, 4, P_Cn, 5, P_Lo, 13, P_Cn, 15, P_Lo, 17, P_Cn, 19, P_Lo, 41, P_Cn, 42, P_Lo, 49, P_Cn, 50, P_Lo, 52, P_Cn, 53, P_Lo, 58, P_Cn, 60, P_Mn, 61, P_Lo, 62, P_Mc, 63, P_Mn, 64, P_Mc, 65, P_Mn, 69, P_Cn, 71, P_Mc, 73, P_Cn, 75, P_Mc, 77, P_Mn, 78, P_Cn, 86, P_Mn, 87, P_Mc, 88, P_Cn, 92, P_Lo, 94, P_Cn, 95, P_Lo, 98, P_Mn, 100, P_Cn, 102, P_Nd, 112, P_So, 113, P_Lo, 114, P_No, 120, P_Cn, 130, P_Mn, 131, P_Lo, 132, P_Cn, 133, P_Lo, 139, P_Cn, 142, P_Lo, 145, P_Cn, 146, P_Lo, 150, P_Cn, 153, P_Lo, 155, P_Cn, 156, P_Lo, 157, P_Cn, 158, P_Lo, 160, P_Cn, 163, P_Lo, 165, P_Cn, 168, P_Lo, 171, P_Cn, 174, P_Lo, 186, P_Cn, 190, P_Mc, 192, P_Mn, 193, P_Mc, 195, P_Cn, 198, P_Mc, 201, P_Cn, 202, P_Mc, 205, P_Mn, 206, P_Cn, 208, P_Lo, 209, P_Cn, 215, P_Mc, 216, P_Cn, 230, P_Nd, 240, P_No, 243, P_So, 249, P_Sc, 250, P_So, 251, P_Cn,
    0, P_Mn, 1, P_Mc, 4, P_Cn, 5, P_Lo, 13, P_Cn, 14, P_Lo, 17, P_Cn, 18, P_Lo, 41, P_Cn, 42, P_Lo, 58, P_Cn, 61, P_Lo, 62, P_Mn, 65, P_Mc, 69, P_Cn, 70, P_Mn, 73, P_Cn, 74, P_Mn, 78, P_Cn, 85, P_Mn, 87, P_Cn, 88, P_Lo, 91, P_Cn, 96, P_Lo, 98, P_Mn, 100, P_Cn, 102, P_Nd, 112, P_Cn, 120, P_No, 127, P_So, 128, P_Lo, 129, P_Mn, 130, P_Mc, 132, P_Cn, 133, P_Lo, 141, P_Cn, 142, P_Lo, 145, P_Cn, 146, P_Lo, 169, P_Cn, 170, P_Lo, 180, P_Cn, 181, P_Lo, 186, P_Cn, 188, P_Mn, 189, P_Lo, 190, P_Mc, 191, P_Mn, 192, P_Mc, 197, P_Cn, 198, P_Mn, 199, P_Mc, 201, P_Cn, 202, P_Mc, 204, P_Mn, 206, P_Cn, 213, P_Mc, 215, P_Cn, 222, P_Lo, 223, P_Cn, 224, P_Lo, 226, P_Mn, 228, P_Cn, 230, P_Nd, 240, P_Cn, 241, P_Lo, 243, P_Cn,
    0, P_Cn, 1, P_Mn, 2, P_Mc, 4, P_Cn, 5, P_Lo, 13, P_Cn, 14, P_Lo, 17, P_Cn, 18, P_Lo, 59, P_Cn, 61, P_Lo, 62, P_Mc, 65, P_Mn, 69, P_Cn, 70, P_Mc, 73, P_Cn, 74, P_Mc, 77, P_Mn, 78, P_Lo, 79, P_So, 80, P_Cn, 84, P_Lo, 87, P_Mc, 88, P_No, 95, P_Lo, 98, P_Mn, 100, P_Cn, 102, P_Nd, 112, P_No, 121, P_So, 122, P_Lo, 128, P_Cn, 130, P_Mc, 132, P_Cn, 133, P_Lo, 151, P_Cn, 154, P_Lo, 178, P_Cn, 179, P_Lo, 188, P_Cn, 189, P_Lo, 190, P_Cn, 192, P_Lo, 199, P_Cn, 202, P_Mn, 203, P_Cn, 207, P_Mc, 210, P_Mn, 213, P_Cn, 214, P_Mn, 215, P_Cn, 216, P_Mc, 224, P_Cn, 230, P_Nd, 240, P_Cn, 242, P_Mc, 244, P_Po, 245, P_Cn,
    0, P_Cn, 1, P_Lo, 49, P_Mn, 50, P_Lo, 52, P_Mn, 59, P_Cn, 63, P_Sc, 64, P_Lo, 70, P_Lm, 71, P_Mn, 79, P_Po, 80, P_Nd, 90, P_Po, 92, P_Cn, 129, P_Lo, 131, P_Cn, 132, P_Lo, 133, P_Cn, 135, P_Lo, 137, P_Cn, 138, P_Lo, 139, P_Cn, 141, P_Lo, 142, P_Cn, 148, P_Lo, 152, P_Cn, 153, P_Lo, 160, P_Cn, 161, P_Lo, 164, P_Cn, 165, P_Lo, 166, P_Cn, 167, P_Lo, 168, P_Cn, 170, P_Lo, 172, P_Cn, 173, P_Lo, 177, P_Mn, 178, P_Lo, 180, P_Mn, 186, P_Cn, 187, P_Mn, 189, P_Lo, 190, P_Cn, 192, P_Lo, 197, P_Cn, 198, P_Lm, 199, P_Cn, 200, P_Mn, 206, P_Cn, 208, P_Nd, 218, P_Cn, 220, P_Lo, 224, P_Cn,
    0, P_Lo, 1, P_So, 4, P_Po, 19, P_So, 20, P_Po, 21, P_So, 24, P_Mn, 26, P_So, 32, P_Nd, 42, P_No, 52, P_So, 53, P_Mn, 54, P_So, 55, P_Mn, 56, P_So, 57, P_Mn, 58, P_Ps, 59, P_Pe, 60, P_Ps, 61, P_Pe, 62, P_Mc, 64, P_Lo, 72, P_Cn, 73, P_Lo, 109, P_Cn, 113, P_Mn, 127, P_Mc, 128, P_Mn, 133, P_Po, 134, P_Mn, 136, P_Lo, 141, P_Mn, 152, P_Cn, 153, P_Mn, 189, P_Cn, 190, P_So, 198, P_Mn, 199, P_So, 205, P_Cn, 206, P_So, 208, P_Po, 213, P_So, 217, P_Po, 219, P_Cn,
    0, P_Lo, 43, P_Mc, 45, P_Mn, 49, P_Mc, 50, P_Mn, 56, P_Mc, 57, P_Mn, 59, P_Mc, 61, P_Mn, 63, P_Lo, 64, P_Nd, 74, P_Po, 80, P_Lo, 86, P_Mc, 88, P_Mn, 90, P_Lo, 94, P_Mn, 97, P_Lo, 98, P_Mc, 101, P_Lo, 103, P_Mc, 110, P_Lo, 113, P_Mn, 117, P_Lo, 130, P_Mn, 131, P_Mc, 133, P_Mn, 135, P_Mc, 141, P_Mn, 142, P_Lo, 143, P_Mc, 144, P_Nd, 154, P_Mc, 157, P_Mn, 158, P_So, 160, P_Lu, 198, P_Cn, 199, P_Lu, 200, P_Cn, 205, P_Lu, 206, P_Cn, 208, P_Lo, 251, P_Po, 252, P_Lm, 253, P_Lo,
    0, P_Lo,
    0, P_Lo, 73, P_Cn, 74, P_Lo, 78, P_Cn, 80, P_Lo, 87, P_Cn, 88, P_Lo, 89, P_Cn, 90, P_Lo, 94, P_Cn, 96, P_Lo, 137, P_Cn, 138, P_Lo, 142, P_Cn, 144, P_Lo, 177, P_Cn, 178, P_Lo, 182, P_Cn, 184, P_Lo, 191, P_Cn, 192, P_Lo, 193, P_Cn, 194, P_Lo, 198, P_Cn, 200, P_Lo, 215, P_Cn, 216, P_Lo,
    0, P_Lo, 17, P_Cn, 18, P_Lo, 22, P_Cn, 24, P_Lo, 91, P_Cn, 93, P_Mn, 96, P_Po, 105, P_No, 125, P_Cn, 128, P_Lo, 144, P_So, 154, P_Cn, 160, P_Lu, 246, P_Cn, 248, P_Ll, 254, P_Cn,
    0, P_Pd, 1, P_Lo,
    0, P_Lo, 109, P_Po, 111, P_Lo, 128, P_Zs, 129, P_Lo, 155, P_Ps, 156, P_Pe, 157, P_Cn, 160, P_Lo, 235, P_Po, 238, P_Nl, 241, P_Lo, 249, P_Cn,
    0, P_Lo, 13, P_Cn, 14, P_Lo, 18, P_Mn, 21, P_Cn, 32, P_Lo, 50, P_Mn, 53, P_Po, 55, P_Cn, 64, P_Lo, 82, P_Mn, 84, P_Cn, 96, P_Lo, 109, P_Cn, 110, P_Lo, 113, P_Cn, 114, P_Mn, 116, P_Cn, 128, P_Lo, 180, P_Mn, 182, P_Mc, 183, P_Mn, 190, P_Mc, 198, P_Mn, 199, P_Mc, 201, P_Mn, 212, P_Po, 215, P_Lm, 216, P_Po, 219, P_Sc, 220, P_Lo, 221, P_Mn, 222, P_Cn, 224, P_Nd, 234, P_Cn, 240, P_No, 250, P_Cn,
    0, P_Po, 6, P_Pd, 7, P_Po, 11, P_Mn, 14, P_Cf, 15, P_Cn, 16, P_Nd, 26, P_Cn, 32, P_Lo, 67, P_Lm, 68, P_Lo, 120, P_Cn, 128, P_Lo, 133, P_Mn, 135, P_Lo, 169, P_Mn, 170, P_Lo, 171, P_Cn, 176, P_Lo, 246, P_Cn,
    0, P_Lo, 31, P_Cn, 32, P_Mn, 35, P_Mc, 39, P_Mn, 41, P_Mc, 44, P_Cn, 48, P_Mc, 50, P_Mn, 51, P_Mc, 57, P_Mn, 60, P_Cn, 64, P_So, 65, P_Cn, 68, P_Po, 70, P_Nd, 80, P_Lo, 110, P_Cn, 112, P_Lo, 117, P_Cn, 128, P_Lo, 172, P_Cn, 176, P_Lo, 202, P_Cn, 208, P_Nd, 218, P_No, 219, P_Cn, 222, P_So,
    0, P_Lo, 23, P_Mn, 25, P_Mc, 27, P_Mn, 28, P_Cn, 30, P_Po, 32, P_Lo, 85, P_Mc, 86, P_Mn, 87, P_Mc, 88, P_Mn, 95, P_Cn, 96, P_Mn, 97, P_Mc, 98, P_Mn, 99, P_Mc, 101, P_Mn, 109, P_Mc, 115, P_Mn, 125, P_Cn, 127, P_Mn, 128, P_Nd, 138, P_Cn, 144, P_Nd, 154, P_Cn, 160, P_Po, 167, P_Lm, 168, P_Po, 174, P_Cn, 176, P_Mn, 190, P_Me, 191, P_Cn,
    0, P_Mn, 4, P_Mc, 5, P_Lo, 52, P_Mn, 53, P_Mc, 54, P_Mn, 59, P_Mc, 60, P_Mn, 61, P_Mc, 66, P_Mn, 67, P_Mc, 69, P_Lo, 76, P_Cn, 80, P_Nd, 90, P_Po, 97, P_So, 107, P_Mn, 116, P_So, 125, P_Cn, 128, P_Mn, 130, P_Mc, 131, P_Lo, 161, P_Mc, 162, P_Mn, 166, P_Mc, 168, P_Mn, 170, P_Mc, 171, P_Mn, 174, P_Lo, 176, P_Nd, 186, P_Lo, 230, P_Mn, 231, P_Mc, 232, P_Mn, 234, P_Mc, 237, P_Mn, 238, P_Mc, 239, P_Mn, 242, P_Mc, 244, P_Cn, 252, P_Po,
    0, P_Lo, 36, P_Mc, 44, P_Mn, 52, P_Mc, 54, P_Mn, 56, P_Cn, 59, P_Po, 64, P_Nd, 74, P_Cn, 77, P_Lo, 80, P_Nd, 90, P_Lo, 120, P_Lm, 126, P_Po, 128, P_Ll, 137, P_Cn, 192, P_Po, 200, P_Cn, 208, P_Mn, 211, P_Po, 212, P_Mn, 225, P_Mc, 226, P_Mn, 233, P_Lo, 237, P_Mn, 238, P_Lo, 242, P_Mc, 244, P_Mn, 245, P_Lo, 247, P_Cn, 248, P_Mn, 250, P_Cn,
    0, P_Ll, 44, P_Lm, 107, P_Ll, 120, P_Lm, 121, P_Ll, 155, P_Lm, 192, P_Mn, 246, P_Cn, 251, P_Mn,
    0, P_Lul, 150, P_Ll, 158, P_Lul,
    0, P_Ll, 8, P_Lu, 16, P_Ll, 22, P_Cn, 24, P_Lu, 30, P_Cn, 32, P_Ll, 40, P_Lu, 48, P_Ll, 56, P_Lu, 64, P_Ll, 70, P_Cn, 72, P_Lu, 78, P_Cn, 80, P_Ll, 88, P_Cn, 89, P_Lu, 90, P_Cn, 91, P_Lu, 92, P_Cn, 93, P_Lu, 94, P_Cn, 95, P_Lul, 97, P_Ll, 104, P_Lu, 112, P_Ll, 126, P_Cn, 128, P_Ll, 136, P_Lt, 144, P_Ll, 152, P_Lt, 160, P_Ll, 168, P_Lt, 176, P_Ll, 181, P_Cn, 182, P_Ll, 184, P_Lu, 188, P_Lt, 189, P_Sk, 190, P_Ll, 191, P_Sk, 194, P_Ll, 197, P_Cn, 198, P_Ll, 200, P_Lu, 204, P_Lt, 205, P_Sk, 208, P_Ll, 212, P_Cn, 214, P_Ll, 216, P_Lu, 220, P_Cn, 221, P_Sk, 224, P_Ll, 232, P_Lu, 237, P_Sk, 240, P_Cn, 242, P_Ll, 245, P_Cn, 246, P_Ll, 248, P_Lu, 252, P_Lt, 253, P_Sk, 255, P_Cn,
    0, P_Zs, 11, P_Cf, 16, P_Pd, 22, P_Po, 24, P_Pi, 25, P_Pf, 26, P_Ps, 27, P_Pi, 29, P_Pf, 30, P_Ps, 31, P_Pi, 32, P_Po, 40, P_Zl, 41, P_Zp, 42, P_Cf, 47, P_Zs, 48, P_Po, 57, P_Pi, 58, P_Pf, 59, P_Po, 63, P_Pc, 65, P_Po, 68, P_Sm, 69, P_Ps, 70, P_Pe, 71, P_Po, 82, P_Sm, 83, P_Po, 84, P_Pc, 85, P_Po, 95, P_Zs, 96, P_Cf, 101, P_Cn, 102, P_Cf, 112, P_No, 113, P_Lm, 114, P_Cn, 116, P_No, 122, P_Sm, 125, P_Ps, 126, P_Pe, 127, P_Lm, 128, P_No, 138, P_Sm, 141, P_Ps, 142, P_Pe, 143, P_Cn, 144, P_Lm, 157, P_Cn, 160, P_Sc, 191, P_Cn, 208, P_Mn, 221, P_Me, 225, P_Mn, 226, P_Me, 229, P_Mn, 241, P_Cn,
    0, P_So, 2, P_Lu, 3, P_So, 7, P_Lu, 8, P_So, 10, P_Ll, 11, P_Lu, 14, P_Ll, 16, P_Lu, 19, P_Ll, 20, P_So, 21, P_Lu, 22, P_So, 24, P_Sm, 25, P_Lu, 30, P_So, 36, P_Lu, 37, P_So, 38, P_Lu, 39, P_So, 40, P_Lu, 41, P_So, 42, P_Lu, 46, P_So, 47, P_Ll, 48, P_Lu, 52, P_Ll, 53, P_Lo, 57, P_Ll, 58, P_So, 60, P_Ll, 62, P_Lu, 64, P_Sm, 69, P_Lul, 71, P_Ll, 74, P_So, 75, P_Sm, 76, P_So, 78, P_Ll, 79, P_So, 80, P_No, 96, P_Nl, 131, P_Lul, 133, P_Nl, 137, P_No, 138, P_So, 140, P_Cn, 144, P_Sm, 149, P_So, 154, P_Sm, 156, P_So, 160, P_Sm, 161, P_So, 163, P_Sm, 164, P_So, 166, P_Sm, 167, P_So, 174, P_Sm, 175, P_So, 206, P_Sm, 208, P_So, 210, P_Sm, 211, P_So, 212, P_Sm, 213, P_So, 244, P_Sm,
    0, P_Sm,
    0, P_So, 8, P_Ps, 9, P_Pe, 10, P_Ps, 11, P_Pe, 12, P_So, 32, P_Sm, 34, P_So, 41, P_Ps, 42, P_Pe, 43, P_So, 124, P_Sm, 125, P_So, 155, P_Sm, 180, P_So, 220, P_Sm, 226, P_So, 255, P_Cn,
    0, P_So, 39, P_Cn, 64, P_So, 75, P_Cn, 96, P_No, 156, P_So, 234, P_No,
    0, P_So, 183, P_Sm, 184, P_So, 193, P_Sm, 194, P_So, 248, P_Sm,
    0, P_So, 111, P_Sm, 112, P_So,
    0, P_So, 104, P_Ps, 105, P_Pe, 106, P_Ps, 107, P_Pe, 108, P_Ps, 109, P_Pe, 110, P_Ps, 111, P_Pe, 112, P_Ps, 113, P_Pe, 114, P_Ps, 115, P_Pe, 116, P_Ps, 117, P_Pe, 118, P_No, 148, P_So, 192, P_Sm, 197, P_Ps, 198, P_Pe, 199, P_Sm, 230, P_Ps, 231, P_Pe, 232, P_Ps, 233, P_Pe, 234, P_Ps, 235, P_Pe, 236, P_Ps, 237, P_Pe, 238, P_Ps, 239, P_Pe, 240, P_Sm,
    0, P_So,
    0, P_Sm, 131, P_Ps, 132, P_Pe, 133, P_Ps, 134, P_Pe, 135, P_Ps, 136, P_Pe, 137, P_Ps, 138, P_Pe, 139, P_Ps, 140, P_Pe, 141, P_Ps, 142, P_Pe, 143, P_Ps, 144, P_Pe, 145, P_Ps, 146, P_Pe, 147, P_Ps, 148, P_Pe, 149, P_Ps, 150, P_Pe, 151, P_Ps, 152, P_Pe, 153, P_Sm, 216, P_Ps, 217, P_Pe, 218, P_Ps, 219, P_Pe, 220, P_Sm, 252, P_Ps, 253, P_Pe, 254, P_Sm,
    0, P_So, 48, P_Sm, 69, P_So, 71, P_Sm, 77, P_So, 116, P_Cn, 118, P_So, 150, P_Cn, 152, P_So, 186, P_Cn, 189, P_So, 201, P_Cn, 202, P_So, 210, P_Cn, 236, P_So, 240, P_Cn,
    0, P_Lu, 47, P_Cn, 48, P_Ll, 95, P_Cn, 96, P_Lul, 98, P_Lu, 101, P_Ll, 103, P_Lul, 109, P_Lu, 113, P_Ll, 114, P_Lul, 116, P_Ll, 117, P_Lul, 119, P_Ll, 124, P_Lm, 126, P_Lu, 129, P_Ll, 130, P_Lul, 228, P_Ll, 229, P_So, 235, P_Lul, 239, P_Mn, 242, P_Lul, 244, P_Cn, 249, P_Po, 253, P_No, 254, P_Po,
    0, P_Ll, 38, P_Cn, 39, P_Ll, 40, P_Cn, 45, P_Ll, 46, P_Cn, 48, P_Lo, 104, P_Cn, 111, P_Lm, 112, P_Po, 113, P_Cn, 127, P_Mn, 128, P_Lo, 151, P_Cn, 160, P_Lo, 167, P_Cn, 168, P_Lo, 175, P_Cn, 176, P_Lo, 183, P_Cn, 184, P_Lo, 191, P_Cn, 192, P_Lo, 199, P_Cn, 200, P_Lo, 207, P_Cn, 208, P_Lo, 215, P_Cn, 216, P_Lo, 223, P_Cn, 224, P_Mn,
    0, P_Po, 2, P_Pi, 3, P_Pf, 4, P_Pi, 5, P_Pf, 6, P_Po, 9, P_Pi, 10, P_Pf, 11, P_Po, 12, P_Pi, 13, P_Pf, 14, P_Po, 23, P_Pd, 24, P_Po, 26, P_Pd, 27, P_Po, 28, P_Pi, 29, P_Pf, 30, P_Po, 32, P_Pi, 33, P_Pf, 34, P_Ps, 35, P_Pe, 36, P_Ps, 37, P_Pe, 38, P_Ps, 39, P_Pe, 40, P_Ps, 41, P_Pe, 42, P_Po, 47, P_Lm, 48, P_Po, 58, P_Pd, 60, P_Po, 64, P_Pd, 65, P_Po, 66, P_Ps, 67, P_Po, 69, P_Cn, 128, P_So, 154, P_Cn, 155, P_So, 244, P_Cn,
    0, P_So, 214, P_Cn, 240, P_So, 252, P_Cn,
    0, P_Zs, 1, P_Po, 4, P_So, 5, P_Lm, 6, P_Lo, 7, P_Nl, 8, P_Ps, 9, P_Pe, 10, P_Ps, 11, P_Pe, 12, P_Ps, 13, P_Pe, 14, P_Ps, 15, P_Pe, 16, P_Ps, 17, P_Pe, 18, P_So, 20, P_Ps, 21, P_Pe, 22, P_Ps, 23, P_Pe, 24, P_Ps, 25, P_Pe, 26, P_Ps, 27, P_Pe, 28, P_Pd, 29, P_Ps, 30, P_Pe, 32, P_So, 33, P_Nl, 42, P_Mn, 46, P_Mc, 48, P_Pd, 49, P_Lm, 54, P_So, 56, P_Nl, 59, P_Lm, 60, P_Lo, 61, P_Po, 62, P_So, 64, P_Cn, 65, P_Lo, 151, P_Cn, 153, P_Mn, 155, P_Sk, 157, P_Lm, 159, P_Lo, 160, P_Pd, 161, P_Lo, 251, P_Po, 252, P_Lm, 255, P_Lo,
    0, P_Cn, 5, P_Lo, 46, P_Cn, 49, P_Lo, 143, P_Cn, 144, P_So, 146, P_No, 150, P_So, 160, P_Lo, 187, P_Cn, 192, P_So, 228, P_Cn, 240, P_Lo,
    0, P_So, 31, P_Cn, 32, P_No, 42, P_So, 72, P_No, 80, P_So, 81, P_No, 96, P_So, 128, P_No, 138, P_So, 177, P_No, 192, P_So, 255, P_Cn,
    0, P_Lo, 1, P_Cn,
    0, P_Cn, 181, P_Lo, 182, P_Cn, 192, P_So,
    0, P_Cn, 213, P_Lo, 214, P_Cn,
    0, P_Lo, 21, P_Lm, 22, P_Lo,
    0, P_Lo, 141, P_Cn, 144, P_So, 199, P_Cn, 208, P_Lo, 248, P_Lm, 254, P_Po,
    0, P_Lo, 12, P_Lm, 13, P_Po, 16, P_Lo, 32, P_Nd, 42, P_Lo, 44, P_Cn, 64, P_Lul, 110, P_Lo, 111, P_Mn, 112, P_Me, 115, P_Po, 116, P_Mn, 126, P_Po, 127, P_Lm, 128, P_Lul, 156, P_Lm, 158, P_Mn, 160, P_Lo, 230, P_Nl, 240, P_Mn, 242, P_Po, 248, P_Cn,
    0, P_Sk, 23, P_Lm, 32, P_Sk, 34, P_Lul, 48, P_Ll, 50, P_Lul, 112, P_Lm, 113, P_Ll, 121, P_Lul, 125, P_Lu, 127, P_Ll, 128, P_Lul, 136, P_Lm, 137, P_Sk, 139, P_Lul, 143, P_Lo, 144, P_Lul, 148, P_Ll, 150, P_Lul, 170, P_Lu, 175, P_Cn, 176, P_Lu, 181, P_Ll, 182, P_Lul, 184, P_Cn, 247, P_Lo, 248, P_Lm, 250, P_Ll, 251, P_Lo,
    0, P_Lo, 2, P_Mn, 3, P_Lo, 6, P_Mn, 7, P_Lo, 11, P_Mn, 12, P_Lo, 35, P_Mc, 37, P_Mn, 39, P_Mc, 40, P_So, 44, P_Cn, 48, P_No, 54, P_So, 56, P_Sc, 57, P_So, 58, P_Cn, 64, P_Lo, 116, P_Po, 120, P_Cn, 128, P_Mc, 130, P_Lo, 180, P_Mc, 196, P_Mn, 198, P_Cn, 206, P_Po, 208, P_Nd, 218, P_Cn, 224, P_Mn, 242, P_Lo, 248, P_Po, 251, P_Lo, 252, P_Po, 253, P_Lo, 254, P_Cn,
    0, P_Nd, 10, P_Lo, 38, P_Mn, 46, P_Po, 48, P_Lo, 71, P_Mn, 82, P_Mc, 84, P_Cn, 95, P_Po, 96, P_Lo, 125, P_Cn, 128, P_Mn, 131, P_Mc, 132, P_Lo, 179, P_Mn, 180, P_Mc, 182, P_Mn, 186, P_Mc, 188, P_Mn, 189, P_Mc, 193, P_Po, 206, P_Cn, 207, P_Lm, 208, P_Nd, 218, P_Cn, 222, P_Po, 224, P_Lo, 229, P_Mn, 230, P_Lm, 231, P_Lo, 240, P_Nd, 250, P_Lo, 255, P_Cn,
    0, P_Lo, 41, P_Mn, 47, P_Mc, 49, P_Mn, 51, P_Mc, 53, P_Mn, 55, P_Cn, 64, P_Lo, 67, P_Mn, 68, P_Lo, 76, P_Mn, 77, P_Mc, 78, P_Cn, 80, P_Nd, 90, P_Cn, 92, P_Po, 96, P_Lo, 112, P_Lm, 113, P_Lo, 119, P_So, 122, P_Lo, 123, P_Mc, 124, P_Mn, 125, P_Mc, 126, P_Lo, 176, P_Mn, 177, P_Lo, 178, P_Mn, 181, P_Lo, 183, P_Mn, 185, P_Lo, 190, P_Mn, 192, P_Lo, 193, P_Mn, 194, P_Lo, 195, P_Cn, 219, P_Lo, 221, P_Lm, 222, P_Po, 224, P_Lo, 235, P_Mc, 236, P_Mn, 238, P_Mc, 240, P_Po, 242, P_Lo, 243, P_Lm, 245, P_Mc, 246, P_Mn, 247, P_Cn,
    0, P_Cn, 1, P_Lo, 7, P_Cn, 9, P_Lo, 15, P_Cn, 17, P_Lo, 23, P_Cn, 32, P_Lo, 39, P_Cn, 40, P_Lo, 47, P_Cn, 48, P_Ll, 91, P_Sk, 92, P_Lm, 96, P_Ll, 102, P_Cn, 112, P_Ll, 192, P_Lo, 227, P_Mc, 229, P_Mn, 230, P_Mc, 232, P_Mn, 233, P_Mc, 235, P_Po, 236, P_Mc, 237, P_Mn, 238, P_Cn, 240, P_Nd, 250, P_Cn,
    0, P_Cn, 163, P_Lo, 164, P_Cn, 176, P_Lo, 199, P_Cn, 203, P_Lo, 252, P_Cn,
    0, P_Cs, 1, P_Cn,
    0, P_Cn, 127, P_Cs, 129, P_Cn, 255, P_Cs,
    0, P_Cn, 255, P_Cs,
    0, P_Co, 1, P_Cn,
    0, P_Cn, 255, P_Co,
    0, P_Lo, 110, P_Cn, 112, P_Lo, 218, P_Cn,
    0, P_Ll, 7, P_Cn, 19, P_Ll, 24, P_Cn, 29, P_Lo, 30, P_Mn, 31, P_Lo, 41, P_Sm, 42, P_Lo, 55, P_Cn, 56, P_Lo, 61, P_Cn, 62, P_Lo, 63, P_Cn, 64, P_Lo, 66, P_Cn, 67, P_Lo, 69, P_Cn, 70, P_Lo, 178, P_Sk, 194, P_Cn, 211, P_Lo,
    0, P_Lo, 62, P_Pe, 63, P_Ps, 64, P_Cn, 80, P_Lo, 144, P_Cn, 146, P_Lo, 200, P_Cn, 240, P_Lo, 252, P_Sc, 253, P_So, 254, P_Cn,
    0, P_Mn, 16, P_Po, 23, P_Ps, 24, P_Pe, 25, P_Po, 26, P_Cn, 32, P_Mn, 48, P_Po, 49, P_Pd, 51, P_Pc, 53, P_Ps, 54, P_Pe, 55, P_Ps, 56, P_Pe, 57, P_Ps, 58, P_Pe, 59, P_Ps, 60, P_Pe, 61, P_Ps, 62, P_Pe, 63, P_Ps, 64, P_Pe, 65, P_Ps, 66, P_Pe, 67, P_Ps, 68, P_Pe, 69, P_Po, 71, P_Ps, 72, P_Pe, 73, P_Po, 77, P_Pc, 80, P_Po, 83, P_Cn, 84, P_Po, 88, P_Pd, 89, P_Ps, 90, P_Pe, 91, P_Ps, 92, P_Pe, 93, P_Ps, 94, P_Pe, 95, P_Po, 98, P_Sm, 99, P_Pd, 100, P_Sm, 103, P_Cn, 104, P_Po, 105, P_Sc, 106, P_Po, 108, P_Cn, 112, P_Lo, 117, P_Cn, 118, P_Lo, 253, P_Cn, 255, P_Cf,
    0, P_Cn, 1, P_Po, 4, P_Sc, 5, P_Po, 8, P_Ps, 9, P_Pe, 10, P_Po, 11, P_Sm, 12, P_Po, 13, P_Pd, 14, P_Po, 16, P_Nd, 26, P_Po, 28, P_Sm, 31, P_Po, 33, P_Lu, 59, P_Ps, 60, P_Po, 61, P_Pe, 62, P_Sk, 63, P_Pc, 64, P_Sk, 65, P_Ll, 91, P_Ps, 92, P_Sm, 93, P_Pe, 94, P_Sm, 95, P_Ps, 96, P_Pe, 97, P_Po, 98, P_Ps, 99, P_Pe, 100, P_Po, 102, P_Lo, 112, P_Lm, 113, P_Lo, 158, P_Lm, 160, P_Lo, 191, P_Cn, 194, P_Lo, 200, P_Cn, 202, P_Lo, 208, P_Cn, 210, P_Lo, 216, P_Cn, 218, P_Lo, 221, P_Cn, 224, P_Sc, 226, P_Sm, 227, P_Sk, 228, P_So, 229, P_Sc, 231, P_Cn, 232, P_So, 233, P_Sm, 237, P_So, 239, P_Cn, 249, P_Cf, 252, P_So, 254, P_Cn,
    0, P_Lo, 12, P_Cn, 13, P_Lo, 39, P_Cn, 40, P_Lo, 59, P_Cn, 60, P_Lo, 62, P_Cn, 63, P_Lo, 78, P_Cn, 80, P_Lo, 94, P_Cn, 128, P_Lo, 251, P_Cn,
    0, P_Po, 3, P_Cn, 7, P_No, 52, P_Cn, 55, P_So, 64, P_Nl, 117, P_No, 121, P_So, 138, P_No, 140, P_So, 143, P_Cn, 144, P_So, 156, P_Cn, 160, P_So, 161, P_Cn, 208, P_So, 253, P_Mn, 254, P_Cn,
    0, P_Cn, 128, P_Lo, 157, P_Cn, 160, P_Lo, 209, P_Cn, 224, P_Mn, 225, P_No, 252, P_Cn,
    0, P_Lo, 32, P_No, 36, P_Cn, 48, P_Lo, 65, P_Nl, 66, P_Lo, 74, P_Nl, 75, P_Cn, 80, P_Lo, 118, P_Mn, 123, P_Cn, 128, P_Lo, 158, P_Cn, 159, P_Po, 160, P_Lo, 196, P_Cn, 200, P_Lo, 208, P_Po, 209, P_Nl, 214, P_Cn,
    0, P_Lu, 40, P_Ll, 80, P_Lo, 158, P_Cn, 160, P_Nd, 170, P_Cn, 176, P_Lu, 212, P_Cn, 216, P_Ll, 252, P_Cn,
    0, P_Lo, 40, P_Cn, 48, P_Lo, 100, P_Cn, 111, P_Po, 112, P_Cn,
    0, P_Lo, 55, P_Cn, 64, P_Lo, 86, P_Cn, 96, P_Lo, 104, P_Cn,
    0, P_Lo, 6, P_Cn, 8, P_Lo, 9, P_Cn, 10, P_Lo, 54, P_Cn, 55, P_Lo, 57, P_Cn, 60, P_Lo, 61, P_Cn, 63, P_Lo, 86, P_Cn, 87, P_Po, 88, P_No, 96, P_Lo, 119, P_So, 121, P_No, 128, P_Lo, 159, P_Cn, 167, P_No, 176, P_Cn, 224, P_Lo, 243, P_Cn, 244, P_Lo, 246, P_Cn, 251, P_No,
    0, P_Lo, 22, P_No, 28, P_Cn, 31, P_Po, 32, P_Lo, 58, P_Cn, 63, P_Po, 64, P_Cn, 128, P_Lo, 184, P_Cn, 188, P_No, 190, P_Lo, 192, P_No, 208, P_Cn, 210, P_No,
    0, P_Lo, 1, P_Mn, 4, P_Cn, 5, P_Mn, 7, P_Cn, 12, P_Mn, 16, P_Lo, 20, P_Cn, 21, P_Lo, 24, P_Cn, 25, P_Lo, 52, P_Cn, 56, P_Mn, 59, P_Cn, 63, P_Mn, 64, P_No, 72, P_Cn, 80, P_Po, 89, P_Cn, 96, P_Lo, 125, P_No, 127, P_Po, 128, P_Lo, 157, P_No, 160, P_Cn, 192, P_Lo, 200, P_So, 201, P_Lo, 229, P_Mn, 231, P_Cn, 235, P_No, 240, P_Po, 247, P_Cn,
    0, P_Lo, 54, P_Cn, 57, P_Po, 64, P_Lo, 86, P_Cn, 88, P_No, 96, P_Lo, 115, P_Cn, 120, P_No, 128, P_Lo, 146, P_Cn, 153, P_Po, 157, P_Cn, 169, P_No, 176, P_Cn,
    0, P_Lo, 73, P_Cn, 128, P_Lu, 179, P_Cn, 192, P_Ll, 243, P_Cn, 250, P_No,
    0, P_Cn, 96, P_No, 127, P_Cn,
    0, P_Mc, 1, P_Mn, 2, P_Mc, 3, P_Lo, 56, P_Mn, 71, P_Po, 78, P_Cn, 82, P_No, 102, P_Nd, 112, P_Cn, 127, P_Mn, 130, P_Mc, 131, P_Lo, 176, P_Mc, 179, P_Mn, 183, P_Mc, 185, P_Mn, 187, P_Po, 189, P_Cf, 190, P_Po, 194, P_Cn, 208, P_Lo, 233, P_Cn, 240, P_Nd, 250, P_Cn,
    0, P_Mn, 3, P_Lo, 39, P_Mn, 44, P_Mc, 45, P_Mn, 53, P_Cn, 54, P_Nd, 64, P_Po, 68, P_Cn, 80, P_Lo, 115, P_Mn, 116, P_Po, 118, P_Lo, 119, P_Cn, 128, P_Mn, 130, P_Mc, 131, P_Lo, 179, P_Mc, 182, P_Mn, 191, P_Mc, 193, P_Lo, 197, P_Po, 202, P_Mn, 205, P_Po, 206, P_Cn, 208, P_Nd, 218, P_Lo, 219, P_Po, 220, P_Lo, 221, P_Po, 224, P_Cn, 225, P_No, 245, P_Cn,
    0, P_Lo, 18, P_Cn, 19, P_Lo, 44, P_Mc, 47, P_Mn, 50, P_Mc, 52, P_Mn, 53, P_Mc, 54, P_Mn, 56, P_Po, 62, P_Mn, 63, P_Cn, 128, P_Lo, 135, P_Cn, 136, P_Lo, 137, P_Cn, 138, P_Lo, 142, P_Cn, 143, P_Lo, 158, P_Cn, 159, P_Lo, 169, P_Po, 170, P_Cn, 176, P_Lo, 223, P_Mn, 224, P_Mc, 227, P_Mn, 235, P_Cn, 240, P_Nd, 250, P_Cn,
    0, P_Mn, 2, P_Mc, 4, P_Cn, 5, P_Lo, 13, P_Cn, 15, P_Lo, 17, P_Cn, 19, P_Lo, 41, P_Cn, 42, P_Lo, 49, P_Cn, 50, P_Lo, 52, P_Cn, 53, P_Lo, 58, P_Cn, 60, P_Mn, 61, P_Lo, 62, P_Mc, 64, P_Mn, 65, P_Mc, 69, P_Cn, 71, P_Mc, 73, P_Cn, 75, P_Mc, 78, P_Cn, 80, P_Lo, 81, P_Cn, 87, P_Mc, 88, P_Cn, 93, P_Lo, 98, P_Mc, 100, P_Cn, 102, P_Mn, 109, P_Cn, 112, P_Mn, 117, P_Cn,
    0, P_Lo, 53, P_Mc, 56, P_Mn, 64, P_Mc, 66, P_Mn, 69, P_Mc, 70, P_Mn, 71, P_Lo, 75, P_Po, 80, P_Nd, 90, P_Cn, 91, P_Po, 92, P_Cn, 93, P_Po, 94, P_Cn, 128, P_Lo, 176, P_Mc, 179, P_Mn, 185, P_Mc, 186, P_Mn, 187, P_Mc, 191, P_Mn, 193, P_Mc, 194, P_Mn, 196, P_Lo, 198, P_Po, 199, P_Lo, 200, P_Cn, 208, P_Nd, 218, P_Cn,
    0, P_Cn, 128, P_Lo, 175, P_Mc, 178, P_Mn, 182, P_Cn, 184, P_Mc, 188, P_Mn, 190, P_Mc, 191, P_Mn, 193, P_Po, 216, P_Lo, 220, P_Mn, 222, P_Cn,
    0, P_Lo, 48, P_Mc, 51, P_Mn, 59, P_Mc, 61, P_Mn, 62, P_Mc, 63, P_Mn, 65, P_Po, 68, P_Lo, 69, P_Cn, 80, P_Nd, 90, P_Cn, 96, P_Po, 109, P_Cn, 128, P_Lo, 171, P_Mn, 172, P_Mc, 173, P_Mn, 174, P_Mc, 176, P_Mn, 182, P_Mc, 183, P_Mn, 184, P_Cn, 192, P_Nd, 202, P_Cn,
    0, P_Lo, 26, P_Cn, 29, P_Mn, 32, P_Mc, 34, P_Mn, 38, P_Mc, 39, P_Mn, 44, P_Cn, 48, P_Nd, 58, P_No, 60, P_Po, 63, P_So, 64, P_Cn,
    0, P_Cn, 160, P_Lu, 192, P_Ll, 224, P_Nd, 234, P_No, 243, P_Cn, 255, P_Lo,
    0, P_Cn, 192, P_Lo, 249, P_Cn,
    0, P_Lo, 9, P_Cn, 10, P_Lo, 47, P_Mc, 48, P_Mn, 55, P_Cn, 56, P_Mn, 62, P_Mc, 63, P_Mn, 64, P_Lo, 65, P_Po, 70, P_Cn, 80, P_Nd, 90, P_No, 109, P_Cn, 112, P_Po, 114, P_Lo, 144, P_Cn, 146, P_Mn, 168, P_Cn, 169, P_Mc, 170, P_Mn, 177, P_Mc, 178, P_Mn, 180, P_Mc, 181, P_Mn, 183, P_Cn,
    0, P_Lo, 154, P_Cn,
    0, P_Nl, 111, P_Cn, 112, P_Po, 117, P_Cn, 128, P_Lo,
    0, P_Lo, 68, P_Cn,
    0, P_Lo, 47, P_Cn,
    0, P_Lo, 71, P_Cn,
    0, P_Lo, 57, P_Cn, 64, P_Lo, 95, P_Cn, 96, P_Nd, 106, P_Cn, 110, P_Po, 112, P_Cn, 208, P_Lo, 238, P_Cn, 240, P_Mn, 245, P_Po, 246, P_Cn,
    0, P_Lo, 48, P_Mn, 55, P_Po, 60, P_So, 64, P_Lm, 68, P_Po, 69, P_So, 70, P_Cn, 80, P_Nd, 90, P_Cn, 91, P_No, 98, P_Cn, 99, P_Lo, 120, P_Cn, 125, P_Lo, 144, P_Cn,
    0, P_Lo, 69, P_Cn, 80, P_Lo, 81, P_Mc, 127, P_Cn, 143, P_Mn, 147, P_Lm, 160, P_Cn, 224, P_Lm, 225, P_Cn,
    0, P_Cn, 236, P_Lo, 237, P_Cn,
    0, P_Lo, 243, P_Cn,
    0, P_Lo, 2, P_Cn,
    0, P_Lo, 107, P_Cn, 112, P_Lo, 125, P_Cn, 128, P_Lo, 137, P_Cn, 144, P_Lo, 154, P_Cn, 156, P_So, 157, P_Mn, 159, P_Po, 160, P_Cf, 164, P_Cn,
    0, P_So, 246, P_Cn,
    0, P_So, 39, P_Cn, 41, P_So, 101, P_Mc, 103, P_Mn, 106, P_So, 109, P_Mc, 115, P_Cf, 123, P_Mn, 131, P_So, 133, P_Mn, 140, P_So, 170, P_Mn, 174, P_So, 233, P_Cn,
    0, P_So, 66, P_Mn, 69, P_So, 70, P_Cn,
    0, P_So, 87, P_Cn, 96, P_No, 114, P_Cn,
    0, P_Lu, 26, P_Ll, 52, P_Lu, 78, P_Ll, 85, P_Cn, 86, P_Ll, 104, P_Lu, 130, P_Ll, 156, P_Lu, 157, P_Cn, 158, P_Lu, 160, P_Cn, 162, P_Lu, 163, P_Cn, 165, P_Lu, 167, P_Cn, 169, P_Lu, 173, P_Cn, 174, P_Lu, 182, P_Ll, 186, P_Cn, 187, P_Ll, 188, P_Cn, 189, P_Ll, 196, P_Cn, 197, P_Ll, 208, P_Lu, 234, P_Ll,
    0, P_Ll, 4, P_Lu, 6, P_Cn, 7, P_Lu, 11, P_Cn, 13, P_Lu, 21, P_Cn, 22, P_Lu, 29, P_Cn, 30, P_Ll, 56, P_Lu, 58, P_Cn, 59, P_Lu, 63, P_Cn, 64, P_Lu, 69, P_Cn, 70, P_Lu, 71, P_Cn, 74, P_Lu, 81, P_Cn, 82, P_Ll, 108, P_Lu, 134, P_Ll, 160, P_Lu, 186, P_Ll, 212, P_Lu, 238, P_Ll,
    0, P_Ll, 8, P_Lu, 34, P_Ll, 60, P_Lu, 86, P_Ll, 112, P_Lu, 138, P_Ll, 166, P_Cn, 168, P_Lu, 193, P_Sm, 194, P_Ll, 219, P_Sm, 220, P_Ll, 226, P_Lu, 251, P_Sm, 252, P_Ll,
    0, P_Ll, 21, P_Sm, 22, P_Ll, 28, P_Lu, 53, P_Sm, 54, P_Ll, 79, P_Sm, 80, P_Ll, 86, P_Lu, 111, P_Sm, 112, P_Ll, 137, P_Sm, 138, P_Ll, 144, P_Lu, 169, P_Sm, 170, P_Ll, 195, P_Sm, 196, P_Ll, 202, P_Lul, 204, P_Cn, 206, P_Nd,
    0, P_Mn, 55, P_So, 59, P_Mn, 109, P_So, 117, P_Mn, 118, P_So, 132, P_Mn, 133, P_So, 135, P_Po, 140, P_Cn, 155, P_Mn, 160, P_Cn, 161, P_Mn, 176, P_Cn,
    0, P_Mn, 7, P_Cn, 8, P_Mn, 25, P_Cn, 27, P_Mn, 34, P_Cn, 35, P_Mn, 37, P_Cn, 38, P_Mn, 43, P_Cn,
    0, P_Lo, 197, P_Cn, 199, P_No, 208, P_Mn, 215, P_Cn,
    0, P_Lu, 34, P_Ll, 68, P_Mn, 75, P_Cn, 80, P_Nd, 90, P_Cn, 94, P_Po, 96, P_Cn,
    0, P_Lo, 4, P_Cn, 5, P_Lo, 32, P_Cn, 33, P_Lo, 35, P_Cn, 36, P_Lo, 37, P_Cn, 39, P_Lo, 40, P_Cn, 41, P_Lo, 51, P_Cn, 52, P_Lo, 56, P_Cn, 57, P_Lo, 58, P_Cn, 59, P_Lo, 60, P_Cn, 66, P_Lo, 67, P_Cn, 71, P_Lo, 72, P_Cn, 73, P_Lo, 74, P_Cn, 75, P_Lo, 76, P_Cn, 77, P_Lo, 80, P_Cn, 81, P_Lo, 83, P_Cn, 84, P_Lo, 85, P_Cn, 87, P_Lo, 88, P_Cn, 89, P_Lo, 90, P_Cn, 91, P_Lo, 92, P_Cn, 93, P_Lo, 94, P_Cn, 95, P_Lo, 96, P_Cn, 97, P_Lo, 99, P_Cn, 100, P_Lo, 101, P_Cn, 103, P_Lo, 107, P_Cn, 108, P_Lo, 115, P_Cn, 116, P_Lo, 120, P_Cn, 121, P_Lo, 125, P_Cn, 126, P_Lo, 127, P_Cn, 128, P_Lo, 138, P_Cn, 139, P_Lo, 156, P_Cn, 161, P_Lo, 164, P_Cn, 165, P_Lo, 170, P_Cn, 171, P_Lo, 188, P_Cn, 240, P_Sm, 242, P_Cn,
    0, P_So, 44, P_Cn, 48, P_So, 148, P_Cn, 160, P_So, 175, P_Cn, 177, P_So, 192, P_Cn, 193, P_So, 208, P_Cn, 209, P_So, 246, P_Cn,
    0, P_No, 13, P_Cn, 16, P_So, 47, P_Cn, 48, P_So, 108, P_Cn, 112, P_So, 173, P_Cn, 230, P_So,
    0, P_So, 3, P_Cn, 16, P_So, 60, P_Cn, 64, P_So, 73, P_Cn, 80, P_So, 82, P_Cn,
    0, P_So, 251, P_Sk,
    0, P_So, 211, P_Cn, 224, P_So, 237, P_Cn, 240, P_So, 247, P_Cn,
    0, P_So, 116, P_Cn, 128, P_So, 213, P_Cn,
    0, P_So, 12, P_Cn, 16, P_So, 72, P_Cn, 80, P_So, 90, P_Cn, 96, P_So, 136, P_Cn, 144, P_So, 174, P_Cn,
    0, P_Cn, 16, P_So, 31, P_Cn, 32, P_So, 40, P_Cn, 48, P_So, 49, P_Cn, 51, P_So, 63, P_Cn, 64, P_So, 76, P_Cn, 80, P_So, 95, P_Cn, 128, P_So, 146, P_Cn, 192, P_So, 193, P_Cn,
    0, P_Cn, 214, P_Lo, 215, P_Cn,
    0, P_Cn, 52, P_Lo, 53, P_Cn, 64, P_Lo, 65, P_Cn,
    0, P_Cn, 29, P_Lo, 30, P_Cn, 32, P_Lo, 33, P_Cn,
    0, P_Cn, 161, P_Lo, 162, P_Cn,
    0, P_Lo, 30, P_Cn,
    0, P_Cn, 1, P_Cf, 2, P_Cn, 32, P_Cf, 128, P_Cn,
    0, P_Mn, 240, P_Cn,
    0, P_Cn, 253, P_Co, 254, P_Cn,
};
const unsigned int UnicodeProperty::property_offsets[] = {
    0x0, 2, 122,
    0x100, 122, 248,
    0x200, 248, 294,
    0x300, 294, 366,
    0x400, 366, 390,
    0x500, 390, 450,
    0x600, 450, 520,
    0x700, 520, 560,
    0x800, 560, 606,
    0x900, 606, 728,
    0xA00, 728, 878,
    0xB00, 878, 1040,
    0xC00, 1040, 1174,
    0xD00, 1174, 1290,
    0xE00, 1290, 1398,
    0xF00, 1398, 1486,
    0x1000, 1486, 1576,
    0x1100, 1576, 1578,
    0x1200, 1578, 1632,
    0x1300, 1632, 1666,
    0x1400, 1666, 1670,
    0x1500, 1576, 1578,
    0x1600, 1670, 1696,
    0x1700, 1696, 1770,
    0x1800, 1770, 1810,
    0x1900, 1810, 1866,
    0x1A00, 1866, 1930,
    0x1B00, 1930, 2012,
    0x1C00, 2012, 2076,
    0x1D00, 2076, 2094,
    0x1E00, 2094, 2100,
    0x1F00, 2100, 2228,
    0x2000, 2228, 2342,
    0x2100, 2342, 2474,
    0x2200, 2474, 2476,
    0x2300, 2476, 2512,
    0x2400, 2512, 2526,
    0x2500, 2526, 2538,
    0x2600, 2538, 2544,
    0x2700, 2544, 2608,
    0x2800, 2608, 2610,
    0x2900, 2610, 2674,
    0x2A00, 2474, 2476,
    0x2B00, 2674, 2706,
    0x2C00, 2706, 2760,
    0x2D00, 2760, 2822,
    0x2E00, 2822, 2908,
    0x2F00, 2908, 2916,
    0x3000, 2916, 3020,
    0x3100, 3020, 3046,
    0x3200, 3046, 3072,
    0x3300, 2608, 2610,
    0x3400, 3072, 3076,
    0x3500, 0, 2,
    0x4D00, 3076, 3084,
    0x4E00, 3072, 3076,
    0x4F00, 0, 2,
    0x9F00, 3084, 3090,
    0xA000, 3090, 3096,
    0xA100, 1576, 1578,
    0xA400, 3096, 3110,
    0xA500, 1576, 1578,
    0xA600, 3110, 3156,
    0xA700, 3156, 3214,
    0xA800, 3214, 3284,
    0xA900, 3284, 3350,
    0xAA00, 3350, 3448,
    0xAB00, 3448, 3506,
    0xAC00, 3072, 3076,
    0xAD00, 0, 2,
    0xD700, 3506, 3520,
    0xD800, 3520, 3524,
    0xD900, 0, 2,
    0xDB00, 3524, 3532,
    0xDC00, 3520, 3524,
    0xDD00, 0, 2,
    0xDF00, 3532, 3536,
    0xE000, 3536, 3540,
    0xE100, 0, 2,
    0xF800, 3540, 3544,
    0xF900, 1576, 1578,
    0xFA00, 3544, 3552,
    0xFB00, 3552, 3596,
    0xFC00, 1576, 1578,
    0xFD00, 3596, 3620,
    0xFE00, 3620, 3730,
    0xFF00, 3730, 3850,
    0x10000, 3850, 3878,
    0x10100, 3878, 3914,
    0x10200, 3914, 3930,
    0x10300, 3930, 3970,
    0x10400, 3970, 3990,
    0x10500, 3990, 4002,
    0x10600, 1576, 1578,
    0x10700, 4002, 4014,
    0x10800, 4014, 4066,
    0x10900, 4066, 4096,
    0x10A00, 4096, 4162,
    0x10B00, 4162, 4192,
    0x10C00, 4192, 4206,
    0x10D00, 0, 2,
    0x10E00, 4206, 4212,
    0x10F00, 0, 2,
    0x11000, 4212, 4262,
    0x11100, 4262, 4328,
    0x11200, 4328, 4388,
    0x11300, 4388, 4460,
    0x11400, 4460, 4520,
    0x11500, 4520, 4546,
    0x11600, 4546, 4596,
    0x11700, 4596, 4622,
    0x11800, 4622, 4636,
    0x11900, 0, 2,
    0x11A00, 4636, 4642,
    0x11B00, 0, 2,
    0x11C00, 4642, 4696,
    0x11D00, 0, 2,
    0x12000, 1576, 1578,
    0x12300, 4696, 4700,
    0x12400, 4700, 4710,
    0x12500, 4710, 4714,
    0x12600, 0, 2,
    0x13000, 1576, 1578,
    0x13400, 4714, 4718,
    0x13500, 0, 2,
    0x14400, 1576, 1578,
    0x14600, 4718, 4722,
    0x14700, 0, 2,
    0x16800, 1576, 1578,
    0x16A00, 4722, 4748,
    0x16B00, 4748, 4780,
    0x16C00, 0, 2,
    0x16F00, 4780, 4800,
    0x17000, 3072, 3076,
    0x17100, 0, 2,
    0x18700, 4800, 4806,
    0x18800, 1576, 1578,
    0x18A00, 4806, 4810,
    0x18B00, 0, 2,
    0x1B000, 4810, 4814,
    0x1B100, 0, 2,
    0x1BC00, 4814, 4840,
    0x1BD00, 0, 2,
    0x1D000, 4840, 4844,
    0x1D100, 4844, 4874,
    0x1D200, 4874, 4882,
    0x1D300, 4882, 4890,
    0x1D400, 4890, 4946,
    0x1D500, 4946, 5000,
    0x1D600, 5000, 5032,
    0x1D700, 5032, 5074,
    0x1D800, 2608, 2610,
    0x1DA00, 5074, 5102,
    0x1DB00, 0, 2,
    0x1E000, 5102, 5122,
    0x1E100, 0, 2,
    0x1E800, 5122, 5132,
    0x1E900, 5132, 5148,
    0x1EA00, 0, 2,
    0x1EE00, 5148, 5284,
    0x1EF00, 0, 2,
    0x1F000, 5284, 5308,
    0x1F100, 5308, 5326,
    0x1F200, 5326, 5342,
    0x1F300, 5342, 5346,
    0x1F400, 2608, 2610,
    0x1F600, 5346, 5358,
    0x1F700, 5358, 5366,
    0x1F800, 5366, 5386,
    0x1F900, 5386, 5420,
    0x1FA00, 0, 2,
    0x20000, 3072, 3076,
    0x20100, 0, 2,
    0x2A600, 5420, 5426,
    0x2A700, 3072, 3076,
    0x2A800, 0, 2,
    0x2B700, 5426, 5436,
    0x2B800, 5436, 5446,
    0x2B900, 0, 2,
    0x2CE00, 5446, 5452,
    0x2CF00, 0, 2,
    0x2F800, 1576, 1578,
    0x2FA00, 5452, 5456,
    0x2FB00, 0, 2,
    0xE0000, 5456, 5466,
    0xE0100, 5466, 5470,
    0xE0200, 0, 2,
    0xF0000, 3536, 3540,
    0xF0100, 0, 2,
    0xFFF00, 5470, 5476,
    0x100000, 3536, 3540,
    0x100100, 0, 2,
    0x10FF00, 5470, 5476,
    0x110000, 0, 2,
};
const int UnicodeProperty::nproperty_offsets = (sizeof(UnicodeProperty::property_offsets) / (3*sizeof(unsigned int)));

inline const unsigned int *
UnicodeProperty::find_offset(uint32_t u)
{
    // Up to U+1A00 each page has its own definition.
    if (u < 0x1A00)
        return &property_offsets[3*(u >> 8)];
    // At or after U+1A00, binary search.
    int l = 0x1A, r = nproperty_offsets - 2;
    while (l <= r) {
        int m = (l + r) / 2;
        const unsigned int *ptr = &property_offsets[3*m];
        if (u < ptr[0])
            r = m - 1;
        else if (u >= ptr[3])
            l = m + 1;
        else
            return ptr;
    }
    // If search fails, return last record, which will be all-unassigned.
    return &property_offsets[3*(nproperty_offsets - 1)];
}

int
UnicodeProperty::property(uint32_t u)
{
    const unsigned int *offsets = find_offset(u);

    // Now we only care about the last byte.
    u &= 255;

    // Binary search within record.
    int l = offsets[1], r = offsets[2] - 4;
    const unsigned char *the_ptr;
    while (l <= r) {
        int m = ((l + r) / 2) & ~1;
        const unsigned char *ptr = &property_pages[m];
        if (u < ptr[0])
            r = m - 2;
        else if (u >= ptr[2])
            l = m + 2;
        else {
            the_ptr = ptr;
            goto found_ptr;
        }
    }
    the_ptr = &property_pages[l];

  found_ptr:
    // Found right block.
    if (the_ptr[1] == P_Lul)
        return ((u - the_ptr[0]) % 2 ? P_Ll : P_Lu);
    else
        return the_ptr[1];
}

static const char property_names[] =
    "Cn\0Co\0Cs\0Cf\0Cc\0\0\0\0\0\0\0\0\0\0"
    "Zs\0Zl\0Zp\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "Mn\0Mc\0Me\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "Lo\0Lu\0Ll\0Lt\0Lm\0\0\0\0\0\0\0\0\0\0"
    "No\0Nd\0Nl\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "Po\0Pc\0Pd\0Ps\0Pe\0Pi\0Pf\0\0\0\0"
    "So\0Sm\0Sc\0Sk";

static const char * const property_long_names[] = {
    "Unassigned", "PrivateUse", "Surrogate", "Format", "Control", 0, 0, 0,
    "SpaceSeparator", "LineSeparator", "ParagraphSeparator", 0, 0, 0, 0, 0,
    "NonspacingMark", "SpacingMark", "EnclosingMark", 0, 0, 0, 0, 0,
    "OtherLetter", "UppercaseLetter", "LowercaseLetter", "TitlecaseLetter", "ModifierLetter", 0, 0, 0,
    "OtherNumber", "DecimalNumber", "LetterNumber", 0, 0, 0, 0, 0,
    "OtherPunctuation", "ConnectorPunctuation", "DashPunctuation", "OpenPunctuation", "ClosePunctuation", "InitialPunctuation", "FinalPunctuation", 0,
    "OtherSymbol", "MathSymbol", "CurrencySymbol", "ModifierSymbol"
};

static const char * const property_category_long_names[] = {
    "Other", "Separator", "Mark", "Letter", "Number", "Punctuation", "Symbol"
};


const char *
UnicodeProperty::property_name(int p)
{
    if (p >= 0 && p <= P_Sk && property_names[p*3])
        return &property_names[p*3];
    else
        return "?";
}

bool
UnicodeProperty::parse_property(const String &s, int &prop, int &prop_mask)
{
    if (s.length() == 0)
        return false;
    else if (s.length() <= 2) {
        for (int i = 0; i <= P_S; i += 010)
            if (property_names[3*i] == s[0]) {
                if (s.length() == 1) {
                    prop = i;
                    prop_mask = P_TMASK;
                    return true;
                }
                for (; property_names[3*i]; i++)
                    if (property_names[3*i+1] == s[1]) {
                        prop = i;
                        prop_mask = 0377;
                        return true;
                    }
                break;
            }
        return false;
    } else {
        const char * const *dict = property_category_long_names;
        for (int i = 0; i <= P_S; i += 010, dict++)
            if (s == *dict) {
                prop = i;
                prop_mask = P_TMASK;
                return true;
            }
        dict = property_long_names;
        for (int i = 0; i <= P_Sk; i++, dict++)
            if (*dict && s == *dict) {
                prop = i;
                prop_mask = 0377;
                return true;
            }
        return false;
    }
}

